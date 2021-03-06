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

#ifndef _ASM_X86_SETUP_H_
#define _ASM_X86_SETUP_H_

#include <asm/bootparam.h>
#include <sandix/compiler.h>

extern struct boot_params boot_params;

/* Prepare screen first */
void __init early_arch_setup(void);

/* The real architecture setup */
void __init arch_setup(void);

/* For simplicity, reserve legacy 1MB */
void __init probe_roms(void);

/* Allocate space in brk area */
void * __init extend_brk(unsigned long size, unsigned long align);

#endif /* _ASM_X86_SETUP_H_ */
