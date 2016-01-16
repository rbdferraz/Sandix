/*
 *	Copyright (C) 2015-2016 Yizhou Shan <shanyizhou@ict.ac.cn>
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

#ifndef _SANDIX_MUTEX_H_
#define _SANDIX_MUTEX_H_

#include <sandix/compiler.h>

/* TODO now used by tty part */

struct mutex {
	int x;
};

#define DEFINE_MUTEX(NAME) \
	struct mutex NAME

static __always_inline void mutex_lock(struct mutex *m)
{
}

static __always_inline void mutex_unlock(struct mutex *m)
{
}

#endif /* _SANDIX_MUTEX_H_ */
