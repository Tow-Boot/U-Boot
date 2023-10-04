// SPDX-License-Identifier: GPL-2.0+
/*
 * Includes code from `lib/efi_loader/efi_console.c`
 * Which is Copyright (c) 2016 Alexander Graf
 */

#ifdef USE_TERMIOS
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig_term;
#endif

#include <assert.h>
#include "../curspriv.h"
#include "pdcansi.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifdef __U_BOOT__
#include <common.h>
#include <dm/device.h>
#include <env.h>
#include <video_console.h>
#include <linux/delay.h>
#endif

#ifdef USING_COMBINING_CHARACTER_SCHEME
int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);
#endif

int PDC_rows = -1, PDC_cols = -1;
bool PDC_resize_occurred = FALSE;
const int STDIN = 0;
chtype PDC_capabilities = 0;
static mmask_t _stored_trap_mbe;

// Whole #ifdef comes from lib/efi_loader/efi_console.c
#ifdef __U_BOOT__

#define cESC '\x1b'
#define ESC "\x1b"

static int term_get_char(s32 *c)
{
	u64 timeout;

	/* Wait up to 100 ms for a character */
	timeout = timer_get_us() + 100000;

	while (!tstc())
		if (timer_get_us() > timeout)
			return 1;

	*c = getchar();
	return 0;
}

/**
 * Receive and parse a reply from the terminal.
 *
 * @n:		array of return values
 * @num:	number of return values expected
 * @end_char:	character indicating end of terminal message
 * Return:	non-zero indicates error
 */
static int term_read_reply(int *n, int num, char end_char)
{
	s32 c;
	int i = 0;

	if (term_get_char(&c) || c != cESC)
		return -1;

	if (term_get_char(&c) || c != '[')
		return -1;

	n[0] = 0;
	while (1) {
		if (!term_get_char(&c)) {
			if (c == ';') {
				i++;
				if (i >= num)
					return -1;
				n[i] = 0;
				continue;
			} else if (c == end_char) {
				break;
			} else if (c > '9' || c < '0') {
				return -1;
			}

			/* Read one more decimal position */
			n[i] *= 10;
			n[i] += c - '0';
		} else {
			return -1;
		}
	}
	if (i != num - 1)
		return -1;

	return 0;
}

/**
 * query_console_serial() - query serial console size
 *
 * When using a serial console or the net console we can only devise the
 * terminal size by querying the terminal using ECMA-48 control sequences.
 *
 * @rows:	pointer to return number of rows
 * @cols:	pointer to return number of columns
 * Returns:	0 on success
 */
static int query_console_serial(int *rows, int *cols)
{
	int ret = 0;
	int n[2];

	/* Empty input buffer */
	while (tstc())
		getchar();

	/*
	 * Not all terminals understand CSI [18t for querying the console size.
	 * We should adhere to escape sequences documented in the console_codes
	 * man page and the ECMA-48 standard.
	 *
	 * So here we follow a different approach. We position the cursor to the
	 * bottom right and query its position. Before leaving the function we
	 * restore the original cursor position.
	 */
	printf(ESC "7"		/* Save cursor position */
	       ESC "[r"		/* Set scrolling region to full window */
	       ESC "[999;999H"	/* Move to bottom right corner */
	       ESC "[6n");	/* Query cursor position */

	/* Read {rows,cols} */
	if (term_read_reply(n, 2, 'R')) {
		ret = 1;
		goto out;
	}

	*cols = n[1];
	*rows = n[0];
out:
	printf(ESC "8");	/* Restore cursor position */
	return ret;
}

/**
 * query_vidconsole() - query video console size
 *
 *
 * @rows:	pointer to return number of rows
 * @cols:	pointer to return number of columns
 * Returns:	0 on success
 */
static int __maybe_unused query_vidconsole(int *rows, int *cols)
{
	const char *stdout_name = env_get("stdout");
	struct stdio_dev *stdout_dev;
	struct udevice *dev;
	struct vidconsole_priv *priv;

	if (!stdout_name || strncmp(stdout_name, "vidconsole", 10) == 0)
		return -ENODEV;
	stdout_dev = stdio_get_by_name("vidconsole");
	if (!stdout_dev)
		return -ENODEV;
	dev = stdout_dev->priv;
	if (!dev)
		return -ENODEV;
	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -ENODEV;
	*rows = priv->rows;
	*cols = priv->cols;
	return 0;
}

/**
 * query_console_size() - update the mode table.
 *
 * By default the only mode available is 80x25. If the console has at least 50
 * lines, enable mode 80x50. If we can query the console size and it is neither
 * 80x25 nor 80x50, set it as an additional mode.
 */
static void query_console_size(void)
{
	int rows = 25, cols = 80;
	int srows = -1, scols = -1;
	int vrows = -1, vcols = -1;

	if (IS_ENABLED(CONFIG_VIDEO)) {
		(void)query_vidconsole(&vrows, &vcols);
	}

	(void)query_console_serial(&srows, &scols);

	// No data queried
	if (srows == -1 && vrows == -1) {
		goto out;
	}

#if CONFIG_PDCURSES_PREFER_VIDCONSOLE
	if (vrows > 0 && vcols > 0) {
		rows = vrows;
		cols = vcols;
	}
	else if (srows > 0 && scols > 0) {
		rows = srows;
		cols = scols;
	}
#else
	// Take the smaller value, only if not -1
	if (vrows > 0) {
		if (srows > 0 && vrows > srows) { rows = srows; }
		else                            { rows = vrows; }
	}
	if (vcols > 0) {
		if (scols > 0 && vcols > scols) { cols = scols; }
		else                            { cols = vcols; }
	}
#endif

out:
	PDC_cols = cols;
	PDC_rows = rows;
}

#endif

/* COLOR_PAIR to attribute encoding table. */

void PDC_reset_prog_mode( void)
{
#ifdef USE_TERMIOS
    struct termios term;

    tcgetattr( STDIN, &orig_term);
    memcpy( &term, &orig_term, sizeof( term));
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN, TCSANOW, &term);
#endif
    PDC_puts_to_stdout( "\033[?47h");      /* Save screen */
    PDC_puts_to_stdout( "\033" "7");         /* save cursor & attribs (VT100) */

    SP->_trap_mbe = _stored_trap_mbe;
    PDC_mouse_set( );          /* clear any mouse event captures */
    PDC_resize_occurred = FALSE;
}

void PDC_reset_shell_mode( void)
{
}

static int initial_PDC_rows, initial_PDC_cols;

int PDC_resize_screen(int nlines, int ncols)
{
   if( PDC_rows == -1)     /* initscr( ) hasn't been called;  we're just */
      {                    /* setting desired size at startup */
      initial_PDC_rows = nlines;
      initial_PDC_cols = ncols;
      }
   return( 0);
}

void PDC_restore_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_save_screen_mode(int i)
{
    INTENTIONALLY_UNUSED_PARAMETER( i);
}

void PDC_scr_close( void)
{
   PDC_puts_to_stdout( "\033" "8");         /* restore cursor & attribs (VT100) */
   PDC_puts_to_stdout( "\033[m");         /* set default screen attributes */
   PDC_puts_to_stdout( "\033[?47l");      /* restore screen */
   PDC_curs_set( 2);          /* blinking block cursor */
   PDC_gotoyx( PDC_cols - 1, 0);
   _stored_trap_mbe = SP->_trap_mbe;
   SP->_trap_mbe = 0;
   PDC_mouse_set( );          /* clear any mouse event captures */
#ifdef USE_TERMIOS
   tcsetattr( STDIN, TCSANOW, &orig_term);
#endif
   PDC_doupdate( );
   PDC_puts_to_stdout( NULL);      /* free internal cache */
   return;
}

void PDC_scr_free( void)
{
    PDC_free_palette( );
#ifdef USING_COMBINING_CHARACTER_SCHEME
    PDC_expand_combined_characters( 0, NULL);
#endif
}

#ifdef USE_TERMIOS
static void sigwinchHandler( int sig)
{
   struct winsize ws;

   INTENTIONALLY_UNUSED_PARAMETER( sig);
   if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1)
      if( PDC_rows != ws.ws_row || PDC_cols != ws.ws_col)
         {
         PDC_rows = ws.ws_row;
         PDC_cols = ws.ws_col;
         PDC_resize_occurred = TRUE;
         if (SP)
            SP->resized = TRUE;
         }
}

static void sigintHandler( int sig)
{
    INTENTIONALLY_UNUSED_PARAMETER( sig);
    if( !SP->raw_inp)
    {
        PDC_scr_close( );
        PDC_scr_free( );
        exit( 0);
    }
}
#endif

#define MAX_LINES 1000
#define MAX_COLUMNS 1000

int PDC_scr_open(void)
{
#ifdef USE_TERMIOS
    struct sigaction sa;
#endif

    PDC_LOG("PDC_scr_open called\n");
    COLORS = 16;
    assert( SP);
    if (!SP || PDC_init_palette( ))
        return ERR;

#ifndef __U_BOOT__
    setbuf( stdin, NULL);
#endif
#ifdef USE_TERMIOS
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigwinchHandler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        PDC_perror("Sigaction failed\n");
        return( -1);
    }
    sigwinchHandler( 0);

    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        PDC_perror("Sigaction (INT) failed\n");
        return( -1);
    }
#else
    {
#ifdef __U_BOOT__
		query_console_size();
#else
        const char *env = getenv("PDC_LINES");

        PDC_rows = (env ? atoi( env) : 25);
        env = getenv( "PDC_COLS");
        PDC_cols = (env ? atoi( env) : 80);
#endif
    }
#endif
    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->visibility = 0;                /* no cursor,  by default */
    SP->curscol = SP->cursrow = 0;
    SP->audible = TRUE;
    SP->mono = FALSE;
    SP->orig_attr = TRUE;
    SP->orig_fore = SP->orig_back = -1;
    SP->termattrs = PDC_capabilities & ~A_BLINK;

    while( PDC_get_rows( ) < 1 && PDC_get_columns( ) < 1)
      ;     /* wait for screen to be drawn and size determined */
    if( initial_PDC_rows > 1 && initial_PDC_cols > 1)
    {
        PDC_resize_screen( initial_PDC_rows, initial_PDC_cols);
        while( PDC_get_rows( ) != initial_PDC_rows
            && PDC_get_columns( ) != initial_PDC_rows)
           ;
    }

    SP->lines = PDC_get_rows();
    SP->cols = PDC_get_columns();

    if (SP->lines < 2 || SP->lines > MAX_LINES
       || SP->cols < 2 || SP->cols > MAX_COLUMNS)
    {
        PDC_perror("LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        PDC_perror("COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }

    PDC_reset_prog_mode();
    PDC_LOG("PDC_scr_open exit\n");
    return( 0);
}

int PDC_set_function_key( const unsigned function, const int new_key)
{
   INTENTIONALLY_UNUSED_PARAMETER( function);
   INTENTIONALLY_UNUSED_PARAMETER( new_key);
   return( 0);
}

void PDC_set_resize_limits( const int new_min_lines,
                            const int new_max_lines,
                            const int new_min_cols,
                            const int new_max_cols)
{
   INTENTIONALLY_UNUSED_PARAMETER( new_min_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_lines);
   INTENTIONALLY_UNUSED_PARAMETER( new_min_cols);
   INTENTIONALLY_UNUSED_PARAMETER( new_max_cols);
   return;
}


bool PDC_can_change_color(void)
{
    return TRUE;
}

int PDC_color_content( int color, int *red, int *green, int *blue)
{
    const PACKED_RGB col = PDC_get_palette_entry( color);

    *red = DIVROUND( Get_RValue(col) * 1000, 255);
    *green = DIVROUND( Get_GValue(col) * 1000, 255);
    *blue = DIVROUND( Get_BValue(col) * 1000, 255);

    return OK;
}

int PDC_init_color( int color, int red, int green, int blue)
{
    const PACKED_RGB new_rgb = PACK_RGB(DIVROUND(red * 255, 1000),
                                 DIVROUND(green * 255, 1000),
                                 DIVROUND(blue * 255, 1000));

    if( !PDC_set_palette_entry( color, new_rgb))
        curscr->_clear = TRUE;
    return OK;
}
