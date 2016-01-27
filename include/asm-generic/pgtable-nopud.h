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

#ifndef _ASM_GENERIC_PGTABLE_NOPUD_H_
#define _ASM_GENERIC_PGTABLE_NOPUD_H_

#ifndef __ASSEMBLY__

#define PGTABLE_PUD_FOLDED

/*
 * Having the pud type consist of a pgd gets the size right, and allows
 * us to conceptually access the pgd entry that this pud is folded into
 * without casting.
 */
typedef struct {
	pgd_t pgd;
} pud_t;

#define PUD_SHIFT	PGDIR_SHIFT
#define PTRS_PER_PUD	1
#define PUD_SIZE	(1UL << PUD_SHIFT)
#define PUD_MASK	(~(PUD_SIZE - 1))

/*
 * The "pgd_xxx()" functions here are trivial for a folded two-level
 * setup: the pud is never bad, and a pud always exists (as it's folded
 * into the pgd entry)
 */
static inline int  pgd_none(pgd_t pgd)		{ return 0; }
static inline int  pgd_bad(pgd_t pgd)		{ return 0; }
static inline int  pgd_present(pgd_t pgd)	{ return 1; }
static inline void pgd_clear(pgd_t *pgd)	{ }

#define pud_ERROR(pud)				(pgd_ERROR((pud).pgd))

#define pgd_populate(mm, pgd, pud)		do { } while (0)

/*
 * puds are folded into pgds so this doesn't get actually called,
 * but the define is needed for a generic inline function.
 */
#define set_pgd(pgdptr, pgdval) \
	set_pud((pud_t *)(pgdptr), (pud_t) { pgdval })

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return (pud_t *)pgd;
}

#define pud_val(x)				pgd_val((x).pgd)
#define __pud(x)				((pud_t) { __pgd(x) })

/*
 * allocating and freeing a pud is trivial: the 1-entry pud is
 * inside the pgd, so has no extra memory associated with it.
 */
#define pud_alloc_one(mm, address)		NULL
#define pud_free(mm, x)				do { } while (0)
#define __pud_free_tlb(tlb, x, a)		do { } while (0)

#undef  pud_addr_end
#define pud_addr_end(addr, end)			(end)

#endif /* __ASSEMBLY__ */
#endif /* _ASM_GENERIC_PGTABLE_NOPUD_H_ */
