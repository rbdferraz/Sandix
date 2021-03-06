/*
 *	Non-Uniform Memory Access (NUMA)
 *
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

#ifndef _SANDIX_NUMA_H_
#define _SANDIX_NUMA_H_

#ifdef CONFIG_NODES_SHIFT
#define NODES_SHIFT	CONFIG_NODES_SHIFT
#else
#define NODES_SHIFT	0
#endif

#define MAX_NR_NODES	(1 << NODES_SHIFT)

#define NUMA_NO_NODE	(-1)

#endif /* _SANDIX_NUMA_H_ */
