#include "../curspriv.h"

#ifdef __U_BOOT__
#include <watchdog.h>
#include <linux/delay.h>
#endif

void PDC_beep(void)
{
}

void PDC_napms(int ms)
{
    // Ensures U-Boot stuff continue happening.
    // (Yes, this reduces the precision of napms)
    WATCHDOG_RESET();

    mdelay(ms);
}


const char *PDC_sysname(void)
{
   return( "ANSI");
}

enum PDC_port PDC_port_val = PDC_PORT_ANSI;
