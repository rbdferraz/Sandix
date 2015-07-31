/*
 *	init/main.c Sandix Kernel Init
 *
 *	Copyright (C) 2015 Yizhou Shan
 */

#include <sandix/compiler.h>
#include <sandix/const.h>
#include <sandix/console.h>
#include <sandix/mm.h>
#include <sandix/string.h>
#include <sandix/screen_info.h>
#include <sandix/types.h>

#include <asm/bootparam.h>
#include <asm/segment.h>
#include <asm/descriptor.h>

struct boot_params boot_params;
struct screen_info screen_info;

#define __ALIGN8	__attribute__((aligned(8)))

/* NOTE! accessed type useless, We do not use! */
struct desc_struct gdt_table[GDT_ENTRIES] __ALIGN8 =
{
	/* Present, DPL=0, Execute/Read */
	/* Present, DPL=0, Read/Write */
	/* Present, DPL=3, Execute/Read */
	/* Present, DPL=3, Read/Write */
	[GDT_ENTRY_KERNEL_CS]		= GDT_ENTRY_INIT(0xc09a, 0, 0xfffff),
	[GDT_ENTRY_KERNEL_DS]		= GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
	
	[GDT_ENTRY_USER_CS]		= GDT_ENTRY_INIT(0xc0fa, 0, 0xfffff),
	[GDT_ENTRY_USER_DS]		= GDT_ENTRY_INIT(0xc0f2, 0, 0xfffff),

	[GDT_ENTRY_KERNEL_TSS]		= GDT_ENTRY_INIT(0xc092, 0, 0),
	[GDT_ENTRY_KERNEL_PERCPU]	= GDT_ENTRY_INIT(0xc092, 0, 0xfffff),
};

struct desc_struct idt_table[IDT_ENTRIES] __ALIGN8;

void handle_int(void)
{
	asm volatile (
		"incb 0xb8000\n\t"
		"incb 0xb8001\n\t"
		"iret"
	);
}

void __init kernel_init(void)
{
	screen_info = boot_params.screen_info;
	con_init();

	ACTIVE_CON->con_set_color(ACTIVE_VC, 0, 1, 5);
	ACTIVE_CON->con_putcs(ACTIVE_VC, "ABCD", 4,ACTIVE_VC->vc_y,ACTIVE_VC->vc_x);
	ACTIVE_CON->con_scroll(ACTIVE_VC, 2, 4);
	ACTIVE_CON->con_scroll(ACTIVE_VC, 1, 10);
}
