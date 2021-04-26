#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../curspriv.h"
#include "pdcansi.h"

#ifndef __U_BOOT__
#include <sys/select.h>
#endif

#ifdef __U_BOOT__
#include <linux/delay.h>
#endif

/* Modified from the accepted answer at

https://stackoverflow.com/questions/33025599/move-the-cursor-in-a-c-program

_kbhit( ) returns -1 if no key has been hit,  and the keycode if one
has been hit.  You can just loop on it until the return value is >= 0.
Hitting a function or arrow or similar key will cause 27 (escape) to
be returned,  followed by cryptic codes that depend on what terminal
emulation is in place.

   Further info on VT100/ANSI control sequences is at

https://www.gnu.org/software/screen/manual/html_node/Control-Sequences.html
*/

extern bool PDC_resize_occurred;

static bool check_key( int *c)
{
#ifdef __U_BOOT__
    PDC_napms(0);
    if (tstc()) {
        if (c) {
            *c = getchar();
        }
        return TRUE;
    }

    return FALSE;
#else
    bool rval;
    const int STDIN = 0;
    struct timeval timeout;
    fd_set rdset;

    if( PDC_resize_occurred)
       return( TRUE);
    FD_ZERO( &rdset);
    FD_SET( STDIN, &rdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if( select( STDIN + 1, &rdset, NULL, NULL, &timeout) > 0)
       {
       rval = TRUE;
       if( c)
          *c = getchar( );
       }
    else
       rval = FALSE;
    return( rval);
#endif
}

bool PDC_check_key( void)
{
   return( check_key( NULL));
}

void PDC_flushinp( void)
{
   int thrown_away_char;

   while( check_key( &thrown_away_char))
      ;
}

#define MAX_COUNT 15

typedef struct

{
   int key_code;
   const char *xlation;
} xlate_t;

static int xlate_vt_codes( const int *c, const int count)
{
   static const xlate_t xlates[] =  {
             { KEY_UP,     "[A"   },
             { KEY_DOWN,   "[B"   },
             { KEY_LEFT,   "[D"   },
             { KEY_RIGHT,  "[C"   },
             { KEY_HOME,   "OH"   },
             { KEY_HOME,   "[H"   },
             { KEY_END,    "OF"   },
             { KEY_END,    "[F"   },
             { KEY_END,    "[8~"  },         /* rxvt */
             { KEY_B2,     "[E"   },

             { KEY_BTAB,   "[Z"   },           /* Shift-Tab */
             { KEY_IC,     "[2~"  },
             { KEY_DC,     "[3~"  },
             { KEY_PPAGE,  "[5~"  },
             { KEY_NPAGE,  "[6~"  },

             { CTL_LEFT,   "[1;5D"  },
             { CTL_RIGHT,  "[1;5C"  },
             { CTL_UP,     "[1;5A"  },
             { CTL_DOWN,   "[1;5B"  },

             { ALT_PGUP,   "[5;3~"  },
             { ALT_PGDN,   "[6;3~"  },

             { KEY_F(1),    "[[A"   },      /* Linux console */
             { KEY_F(2),    "[[B"   },
             { KEY_F(3),    "[[C"   },
             { KEY_F(4),    "[[D"   },
             { KEY_F(5),    "[[E"   },
             { KEY_END,     "[4~"   },
             { KEY_HOME,    "[1~"   },
             { KEY_HOME,    "[7~"   },    /* rxvt */

             { KEY_F(1),    "OP"      },
             { KEY_F(1),    "[11~"    },
             { KEY_F(2),    "OQ"      },
             { KEY_F(2),    "[12~"    },
             { KEY_F(3),    "OR"      },
             { KEY_F(3),    "[13~"    },
             { KEY_F(4),    "OS"      },
             { KEY_F(4),    "[14~"    },
             { KEY_F(5),    "[15~"    },
             { KEY_F(6),    "[17~"    },
             { KEY_F(7),    "[18~"    },
             { KEY_F(8),    "[19~"    },
             { KEY_F(9),    "[20~"    },
             { KEY_F(10),   "[21~"   },
             { KEY_F(11),   "[23~"   },
             { KEY_F(12),   "[24~"   },
             { KEY_F(13),   "[1;2P"  },      /* shift-f1 */
             { KEY_F(14),   "[1;2Q"  },
             { KEY_F(15),   "[1;2R"  },
             { KEY_F(16),   "[1;2S"  },
             { KEY_F(17),   "[15;2~"   },  /* shift-f5 */
             { KEY_F(18),   "[17;2~"   },
             { KEY_F(19),   "[18;2~"   },
             { KEY_F(20),   "[19;2~"   },
             { KEY_F(21),   "[20;2~"   },
             { KEY_F(22),   "[21;2~"   },
             { KEY_F(23),   "[23;2~"   },  /* shift-f11 */
             { KEY_F(24),   "[24;2~"   },

             { KEY_F(15),   "[25~"   },    /* shift-f3 on rxvt */
             { KEY_F(16),   "[26~"   },    /* shift-f4 on rxvt */
             { KEY_F(17),   "[28~"   },    /* shift-f5 on rxvt */
             { KEY_F(18),   "[29~"   },    /* shift-f6 on rxvt */
             { KEY_F(19),   "[31~"   },    /* shift-f7 on rxvt */
             { KEY_F(20),   "[32~"   },    /* shift-f8 on rxvt */
             { KEY_F(21),   "[33~"   },    /* shift-f9 on rxvt */
             { KEY_F(22),   "[34~"   },    /* shift-f10 on rxvt */
             { KEY_F(23),   "[23$"   },    /* shift-f11 on rxvt */
             { KEY_F(24),   "[24$"   },    /* shift-f12 on rxvt */

             { ALT_UP,      "[1;3A"   },
             { ALT_RIGHT,   "[1;3C"   },
             { ALT_DOWN,    "[1;3B"   },
             { ALT_LEFT,    "[1;3D"   },
             { CTL_UP,      "[1;5A"   },
             { CTL_RIGHT,   "[1;5C"   },
             { CTL_DOWN,    "[1;5B"   },
             { CTL_LEFT,    "[1;5D"   },
             { KEY_SUP,     "[1;2A"   },
             { KEY_SRIGHT,  "[1;2C"   },
             { KEY_SDOWN,   "[1;2B"   },
             { KEY_SLEFT,   "[1;2D"   }  };
   const size_t n_keycodes = sizeof( xlates) / sizeof( xlates[0]);
   size_t i;
   int rval = -1;

   if( count == 1)
      {
      if( c[0] >= 'a' && c[0] <= 'z')
         rval = ALT_A + c[0] - 'a';
      else if( c[0] >= '0' && c[0] <= '9')
         rval = ALT_0 + c[0] - '0';
      else
         {
         const char *text = "',./[];`\x1b\\=-\x0a\x7f";
         const char *tptr = strchr( text, c[0]);
         const int codes[] = { ALT_FQUOTE, ALT_COMMA, ALT_STOP, ALT_FSLASH,
                     ALT_LBRACKET, ALT_RBRACKET,
                     ALT_SEMICOLON, ALT_BQUOTE, ALT_ESC,
                     ALT_BSLASH, ALT_EQUAL, ALT_MINUS, ALT_ENTER, ALT_BKSP };

         if( tptr)
             rval = codes[tptr - text];
         }
      }
   if( count >= 2)
      for( i = 0; rval == -1 && i < n_keycodes; i++)
         {
         int j = 0;

         while( j < count && xlates[i].xlation[j]
                               && xlates[i].xlation[j] == c[j])
            j++;
         if( j == count && !xlates[i].xlation[j])
            rval = xlates[i].key_code;
         }
   return( rval);
}

int PDC_get_key( void)
{
   int rval = -1;

   if( PDC_resize_occurred)
      {
      PDC_resize_occurred = FALSE;
      return( KEY_RESIZE);
      }
   if( check_key( &rval))
      {
      int c[MAX_COUNT];

      SP->key_code = (rval == 27);
      if( rval == 27)
         {
         int count = 0;

         rval = -1;
         while( rval == -1 && count < MAX_COUNT && check_key( &c[count]))
            {
            count++;
            rval = xlate_vt_codes( c, count);
            if( rval == ALT_LBRACKET && check_key( NULL))
               rval = -1;
            }
         if( !count)             /* Escape hit */
            {
            SP->key_code = 0;
            rval = 27;
            }
         count--;
         }
      else if( (rval & 0xc0) == 0xc0)      /* start of UTF-8 */
         {
         check_key( &c[0]);
         assert( (c[0] & 0xc0) == 0x80);
         c[0] &= 0x3f;
         if( !(rval & 0x20))      /* two-byte : U+0080 to U+07ff */
            rval = c[0] | ((rval & 0x1f) << 6);
         else
            {
            check_key( &c[1]);
            assert( (c[1] & 0xc0) == 0x80);
            c[1] &= 0x3f;
            if( !(rval & 0x10))   /* three-byte : U+0800 - U+ffff */
               rval = (c[1] | (c[0] << 6) | ((rval & 0xf) << 12));
            else              /* four-byte : U+FFFF - U+10FFFF : SMP */
               {              /* (Supplemental Multilingual Plane) */
               check_key( &c[2]);
               assert( (c[2] & 0xc0) == 0x80);
               c[2] &= 0x3f;
               rval = (c[2] | (c[1] << 6) | (c[0] << 12) | ((rval & 0xf) << 18));
               }
            }
         }
      else if( rval == 127)
         rval = 8;
      }
   return( rval);
}

int PDC_modifiers_set( void)
{
   return( OK);
}

bool PDC_has_mouse( void)
{
    return FALSE;
}

int PDC_mouse_set( void)
{
   return(  OK);
}

void PDC_set_keyboard_binary( bool on)
{
   INTENTIONALLY_UNUSED_PARAMETER( on);
   return;
}
