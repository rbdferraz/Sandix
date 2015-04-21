/*
 *	2015/04/20 Created by Shan Yizhou.
 *
 *	a20.c: Enable A20 gate.
 *	
 *	Copyright 2009-2014 Intel Corporation; author H. Peter Anvin
 *
 * 	Note: Segment register %fs and %gs are available under real
 *	mode x86 cpu. In addition, for sake of simplicity, this code
 *	enable a20 only in fast way, leave the slow way away.
 */

#include "boot.h"
#include "string.h"

/*
 *	INT 15 - SYSTEM - later PS/2s - ENABLE A20 GATE
 *			AX = 2401h
 *	Return: CF clear if successful
 *			AH = 00h
 */
static int enable_a20_bios()
{
	struct biosregs ireg, oreg;
	
	memset(&ireg, 0, sizeof ireg);
	memset(&oreg, 0, sizeof oreg);
	
	initregs(&ireg);
	ireg.ax = 0x2401;
	intcall(0x15, &ireg, &oreg);
	
	if (!(oreg.eflags & 1) && !oreg.ah)
		return 1;/* BIOS works */
	return 0;
}

/*
 *	Bitfields for PS/2 system control port A:
 *	7-6	any bit set to 1 turns activity light on
 *	5	unused
 *	4	watchdog timout occurred
 *	3	=0 RTC/CMOS security lock (on password area) unlocked
 *		=1 CMOS locked (done by POST)
 *	2	unused
 *	1	A20 is active
 *	0	=0 system reset or write
 *		=1 pulse alternate reset pin (high-speed alternate CPU reset)
 */
static int enable_a20_fast(void)
{
	u8 port_a;

	port_a = inb(0x92);	/* System control port A */
	port_a |=  0x02;	/* Enable A20 */
	port_a &= ~0x01;	/* Do not reset machine */
	outb(port_a, 0x92);
}

#define A20_TEST_ADDR	(4*0x80)
#define A20_TEST_LOOPS	32

/*
 *	Returns nonzero if the A20 line is enabled.
 *	The memory address used is the int $0x80 vector,
 *	which should be safe coz BIOS don't use this interrupt.
 *	0x0000:0200 --> 0x000200
 *	0xffff:0210 --> 0x100200
 */
static int a20_test(void)
{
	int ok = 0, loops = A20_TEST_LOOPS;
	int saved, tmp;

	set_fs(0x0000);
	set_gs(0xffff);

	saved = tmp = rdfs32(A20_TEST_ADDR);
	while (loops--) {
		wrfs32(++tmp, A20_TEST_ADDR);
		io_delay(); /* Serialize and make delay constant */
		ok = rdgs32(A20_TEST_ADDR + 0x10) ^ tmp;
		if (ok)
			break;
	}

	wrfs(saved, A20_TEST_ADDR);
	return ok;
}


int enable_a20(void)
{
	int loops = A20_TEST_LOOPS;

	while (loops--) {
		if (a20_test())
			return 0;
		if (enable_a20_bios)
			return 0;
		enable_a20_fast();
	}

	return -1;
}


