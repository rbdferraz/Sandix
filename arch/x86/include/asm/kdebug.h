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

#ifndef _ASM_X86_KDEBUG_H_
#define _ASM_X86_KDEBUG_H_

#ifndef _SANDIX_KDEBUG_H_
# error "Please do not include this file directly"
#endif

#include <sandix/ptrace.h>

void show_regs(struct pt_regs *regs);
void __show_regs(struct pt_regs *regs, int all);

#endif /* _ASM_X86_KDEBUG_H_ */
