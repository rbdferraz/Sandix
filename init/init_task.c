/*
 *	Copyright (C) 2015-2016 Yizhou Shan <shan13@purdue.edu>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

/* The first task of Sandix */

#include <sandix/sched.h>

#define INIT_MM(mm) {							\
	.mm_users	= ATOMIC_INIT(2),				\
	.mm_count	= ATOMIC_INIT(1),				\
	.page_table_lock= __SPIN_LOCK_UNLOCKED(mm.page_table_lock)	\
}

#define INIT_TASK(task)							\
{									\
	.state		= 0,						\
	.comm		= "swapper",					\
	.stack		= &init_thread_info,				\
	.usage		= ATOMIC_INIT(2),				\
	.flags		= PF_KTHREAD,					\
	.prio		= MAX_PRIO-20,					\
	.static_prio	= MAX_PRIO-20,					\
	.normal_prio	= MAX_PRIO-20,					\
	.policy		= SCHED_NORMAL,					\
	.nr_cpus_allowed= NR_CPUS,					\
	.tasks		= LIST_HEAD_INIT(task.tasks),			\
	.mm		= NULL,						\
	.active_mm	= &init_mm,					\
	.pid		= 0,						\
	.alloc_lock	= __SPIN_LOCK_UNLOCKED(task.alloc_lock),	\
	.real_parent	= &task,					\
	.parent		= &task,					\
	.children	= LIST_HEAD_INIT(task.children),		\
	.sibling	= LIST_HEAD_INIT(task.sibling),			\
	.group_leader	= &task,					\
	.thread		= INIT_THREAD,					\
}

struct mm_struct   init_mm		= INIT_MM(init_mm);
struct task_struct init_task		= INIT_TASK(init_task);

/*
 * Initial task kernel stack.
 * The alignment is handled specially by linker script.
 */
union thread_union init_thread_union __init_task_data = {
	INIT_THREAD_INFO(init_task)
};
