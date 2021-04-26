// #define USE_TERMIOS

#ifdef USE_TERMIOS
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig_term;
#endif

#include <assert.h>
#include "curspriv.h"
#include "pdcansi.h"
#include "../common/pdccolor.h"
#include "../common/pdccolor.c"

#ifdef USING_COMBINING_CHARACTER_SCHEME
int PDC_expand_combined_characters( const cchar_t c, cchar_t *added);
#endif

int PDC_rows = -1, PDC_cols = -1;
bool PDC_resize_occurred = FALSE;
const int STDIN = 0;
chtype PDC_capabilities = 0;
static mmask_t _stored_trap_mbe;

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

    PDC_LOG(("PDC_scr_open called\n"));
    COLORS = 16;
    assert( SP);
    if (!SP || PDC_init_palette( ))
        return ERR;

    setbuf( stdin, NULL);
#ifdef USE_TERMIOS
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigwinchHandler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        fprintf( stderr, "Sigaction failed\n");
        return( -1);
    }
    sigwinchHandler( 0);

    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf( stderr, "Sigaction (INT) failed\n");
        return( -1);
    }
#else
    {
        const char *env = getenv("PDC_LINES");

        PDC_rows = (env ? atoi( env) : 25);
        env = getenv( "PDC_COLS");
        PDC_cols = (env ? atoi( env) : 80);
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
        fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
                MAX_LINES, SP->lines);
        fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
                MAX_COLUMNS, SP->cols);

        return ERR;
    }

    PDC_reset_prog_mode();
    PDC_LOG(("PDC_scr_open exit\n"));
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
