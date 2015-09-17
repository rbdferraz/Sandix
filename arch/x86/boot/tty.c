/*
 *	2015/04/14 Created by Shan Yizhou.
 * 
 *	tty.c: Simple screen control functions.
 *
 *	Under real-mode, we use BIOS to communicate with the world.
 *	INT 0x10 control screen output, INT 0x16 control keyboard.
 *	We also get time from CMOS via IO port. Large amount code is
 *	derived from linux-3.18 kernel.
 */

#include <sandix/types.h>
#include <stdarg.h>
#include "boot.h"

static void bios_putchar(int ch)
{	
	struct biosregs ireg;
	
	initregs(&ireg);
	ireg.bx = 0x0007;
	ireg.cx = 0x0001;
	ireg.ah = 0x0e;
	ireg.al = ch;
	intcall(0x10, &ireg, NULL);
}

void putchar(int ch)
{
	if (ch == '\n')
		putchar('\r'); /* \n --> \n\r */
	bios_putchar(ch);
}

void puts(const char *str)
{
	while(*str != '\0')
		putchar(*str++);
}
