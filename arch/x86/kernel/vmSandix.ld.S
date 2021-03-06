/*
 *	Copyright (C) 2015-2016 Yizhou Shan <shan13@purdue.edu>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <asm/page.h>

#include <sandix/cache.h>
#include <sandix/compiler.h>

#define LOAD_OFFSET		__PAGE_OFFSET
#define	LOAD_PHYS_OFFSET	0x100000

#undef i386
OUTPUT_ARCH(i386)
ENTRY(startup_32)

SECTIONS
{
	. = LOAD_OFFSET + LOAD_PHYS_OFFSET;
	phys_startup_32 = startup_32 - LOAD_OFFSET;

	__kstart = .;

	.text : {
		__text_start = .;
		*(.text*)
		__text_end = .;
	}

	. = ALIGN(PAGE_SIZE);
	.rodata : {
		__rodata_start = .;
		*(.rodata*)
		__rodata_end = .;
	}

	. = ALIGN(PAGE_SIZE);
	.data : {
		__data_start = .;

		/*
		 * the init task, which needs to be aligned
		 * with THREAD_SIZE (8KB in 32bit, 32KB in 64bit)
		 */
		. = ALIGN(THREAD_SIZE);
		*(.data..init_task)

		/* __read_mostly */
		. = ALIGN(L1_CACHE_BYTES);
		*(.data..read_mostly)
		. = ALIGN(L1_CACHE_BYTES);

		. = ALIGN(L1_CACHE_BYTES);
		*(.data)
		. = ALIGN(L1_CACHE_BYTES);

		/* WARN_ONCE etc.*/
		. = ALIGN(L1_CACHE_BYTES);
		*(.data..unlikely)
		. = ALIGN(L1_CACHE_BYTES);

		__data_end = .;
	}

	/*
	 * APIC Drivers, check <asm/apic.h> for details.
	 */
	. = ALIGN(8);
	.apicdrivers : {
		__apicdrivers = .;
		*(.apicdrivers);
		__apicdrivers_end = .;
	}

	/*
	 * __init __initdata section
	 */
	. = ALIGN(PAGE_SIZE);
	.init : {
		__init_start = .;
		*(.init.text)
		*(.init.data)
		__init_end = .;
	}

	/*
	 * System Image Signature
	 * Checked by head_$(BITS).S to see if the entire image was loaded.
	 */
	. = ALIGN(4);
	.signature : {
		kernel_sig = .;
		LONG(0x5a5aaa55)
	}

	. = ALIGN(PAGE_SIZE);
	.bss : {
		__bss_start = .;
		*(.bss)
		__bss_end = .;
	}

	. = ALIGN(PAGE_SIZE);
	.brk : {
		__brk_start = .;
		. += 64 * 1024;
		__brk_limit = .;
	}

	__kend = .;

	/DISCARD/ : { *(.note*) }
	/DISCARD/ : { *(.eh_frame) }
	/DISCARD/ : { *(.comment) }

	/*
	 * Because .brk section is used for page tables during
	 * boot, which means now we can have 16 page tables at most.
	 * 16 page tables can mapping 64M kernel range.
	 *
	 * Therefore, assert if kernel virtual address __kend
	 * exceed 64M upper limit, then we should change .brk size
	 */

	/* If exp is zero, then exit the linker and print message. */
	. = ASSERT((__kend - __kstart <= 64*1024*1024),
		"Kernel image bigger than 64M. Please changing .brk size,
		also check arch/x86/kernel/head.S");
}
