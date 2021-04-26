#include <unistd.h>
#include "curspriv.h"

void PDC_beep(void)
{
}

void PDC_napms(int ms)
{
    usleep(1000 * ms);
}


const char *PDC_sysname(void)
{
   return( "ANSI");
}

enum PDC_port PDC_port_val = PDC_PORT_ANSI;
