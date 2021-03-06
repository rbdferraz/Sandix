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

#ifndef _ASM_X86_PGTABLE_H_
#define _ASM_X86_PGTABLE_H_

/*
 * Since we assume another 3-level paging framework for x86,
 * hence we always have pmd helpers without configuration.
 *
 * Also, please forget about the native prefix, we have no
 * paravirt support. They still here because I want to remind
 * myself that page table operations play a critical role in
 * virtualization, check it later.
 */

#include <sandix/pfn.h>
#include <sandix/const.h>
#include <sandix/types.h>
#include <sandix/bitops.h>
#include <sandix/string.h>

#include <asm/page.h>
#include <asm/barrier.h>
#include <asm/cmpxchg.h>

#include <asm/pgtable_types.h>

struct mm_struct;

#define pgd_val(x)			native_pgd_val(x)
#define __pgd(x)			native_make_pgd(x)

#ifndef __PGTABLE_PUD_FOLDED
#define pud_val(x)			native_pud_val(x)
#define __pud(x)			native_make_pud(x)
#endif

#ifndef __PGTABLE_PMD_FOLDED
#define pmd_val(x)			native_pmd_val(x)
#define __pmd(x)			native_make_pmd(x)
#endif

#define pte_val(x)			native_pte_val(x)
#define __pte(x)			native_make_pte(x)

#ifdef CONFIG_X86_32
#include <asm/pgtable_32.h>
#else
#include <asm/pgtable_64.h>
#endif

/*
 * Batch 1, set/clear entry helpers.
 */
#ifndef __PGTABLE_PUD_FOLDED
#define set_pgd(pgdp, pgd)		native_set_pgd(pgdp, pgd)
#define pgd_clear(pgd)			native_pgd_clear(pgd)
#endif

#ifndef __PGTABLE_PMD_FOLDED
#define set_pud(pudp, pud)		native_set_pud(pudp, pud)
#define pud_clear(pud)			native_pud_clear(pud)
#endif

#define set_pte(ptep, pte)		native_set_pte(ptep, pte)
#define set_pte_at(mm, addr, ptep, pte)	native_set_pte_at(mm, addr, ptep, pte)
#define set_pte_atomic(ptep, pte)	native_set_pte_atomic(ptep, pte)
#define set_pmd(pmdp, pmd)		native_set_pmd(pmdp, pmd)

#define pte_clear(mm, addr, ptep)	native_pte_clear(mm, addr, ptep)
#define pmd_clear(pmd)			native_pmd_clear(pmd)

/*
 * Batch 2, entry flag mask helpers.
 */
static inline pgdval_t pgd_flags(pgd_t pgd)
{
	return pgd_val(pgd) & PTE_FLAGS_MASK;
}

static inline pudval_t pud_pfn_mask(pud_t pud)
{
	if (pud_val(pud) & __PAGE_PSE)
		return PHYSICAL_PUD_PAGE_MASK;
	else
		return PTE_PFN_MASK;
}

static inline pudval_t pud_flags_mask(pud_t pud)
{
	return ~pud_pfn_mask(pud);
}

static inline pudval_t pud_flags(pud_t pud)
{
	return pud_val(pud) & pud_flags_mask(pud);
}

static inline pmdval_t pmd_pfn_mask(pmd_t pmd)
{
	if (pmd_val(pmd) & __PAGE_PSE)
		return PHYSICAL_PMD_PAGE_MASK;
	else
		return PTE_PFN_MASK;
}

static inline pmdval_t pmd_flags_mask(pmd_t pmd)
{
	return ~pmd_pfn_mask(pmd);
}

static inline pmdval_t pmd_flags(pmd_t pmd)
{
	return pmd_val(pmd) & pmd_flags_mask(pmd);
}

static inline pteval_t pte_flags(pte_t pte)
{
	return pte_val(pte) & PTE_FLAGS_MASK;
}

/*
 * Batch 3, entry flag accessors.
 */

static inline int pte_none(pte_t pte)
{
	return !pte.pte;
}

#define __HAVE_ARCH_PTE_SAME
static inline int pte_same(pte_t a, pte_t b)
{
	return a.pte == b.pte;
}

static inline int pte_present(pte_t a)
{
	return pte_flags(a) & __PAGE_PSE;
}

static inline int pte_dirty(pte_t pte)
{
	return pte_flags(pte) & __PAGE_DIRTY;
}

static inline int pte_young(pte_t pte)
{
	return pte_flags(pte) & __PAGE_ACCESSED;
}

static inline int pte_write(pte_t pte)
{
	return pte_flags(pte) & __PAGE_RW;
}

static inline int pte_huge(pte_t pte)
{
	return pte_flags(pte) & __PAGE_PSE;
}

static inline int pte_global(pte_t pte)
{
	return pte_flags(pte) & __PAGE_GLOBAL;
}

static inline int pte_exec(pte_t pte)
{
	return !(pte_flags(pte) & __PAGE_NX);
}

static inline int pmd_none(pmd_t pmd)
{
	/* Only check low word on 32-bit platforms, since it might be
	   out of sync with upper half. */
	return (unsigned long)pmd_val(pmd) == 0;
}

static inline int pmd_present(pmd_t pmd)
{
	return pmd_flags(pmd) & __PAGE_PRESENT;
}

static inline int pmd_dirty(pmd_t pmd)
{
	return pmd_flags(pmd) & __PAGE_DIRTY;
}

static inline int pmd_young(pmd_t pmd)
{
	return pmd_flags(pmd) & __PAGE_ACCESSED;
}

static inline int pmd_accessed(pmd_t pmd)
{
	return pmd_flags(pmd) & __PAGE_ACCESSED;
}

static inline int pmd_large(pmd_t pmd)
{
	return pmd_flags(pmd) & __PAGE_PSE;
}

/* Get Page Frame Number */
static inline unsigned long pte_pfn(pte_t pte)
{
	return (pte_val(pte) & PTE_PFN_MASK) >> PAGE_SHIFT;
}

static inline unsigned long pmd_pfn(pmd_t pmd)
{
	return (pmd_val(pmd) & pmd_pfn_mask(pmd)) >> PAGE_SHIFT;
}

static inline unsigned long pud_pfn(pud_t pud)
{
	return (pud_val(pud) & pud_pfn_mask(pud)) >> PAGE_SHIFT;
}

/*
 * Batch 4, entry flag modification helpers.
 */

static inline pte_t pte_set_flags(pte_t pte, pteval_t set)
{
	pteval_t v = pte_val(pte);

	return __pte(v | set);
}

static inline pte_t pte_clear_flags(pte_t pte, pteval_t clear)
{
	pteval_t v = pte_val(pte);

	return __pte(v & ~clear);
}

static inline pte_t pte_mkclean(pte_t pte)
{
	return pte_clear_flags(pte, __PAGE_DIRTY);
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	return pte_set_flags(pte, __PAGE_DIRTY);
}

static inline pte_t pte_mkexec(pte_t pte)
{
	return pte_clear_flags(pte, __PAGE_NX);
}

static inline pte_t pte_mkold(pte_t pte)
{
	return pte_clear_flags(pte, __PAGE_ACCESSED);
}

static inline pte_t pte_mkyoung(pte_t pte)
{
	return pte_set_flags(pte, __PAGE_ACCESSED);
}

static inline pte_t pte_mkwrite(pte_t pte)
{
	return pte_set_flags(pte, __PAGE_RW);
}

static inline pmd_t pmd_set_flags(pmd_t pmd, pmdval_t set)
{
	pmdval_t v = pmd_val(pmd);

	return __pmd(v | set);
}

static inline pmd_t pmd_clear_flags(pmd_t pmd, pmdval_t clear)
{
	pmdval_t v = pmd_val(pmd);

	return __pmd(v & ~clear);
}

static inline pmd_t pmd_mkdirty(pmd_t pmd)
{
	return pmd_set_flags(pmd, __PAGE_DIRTY);
}

static inline pmd_t pmd_mkold(pmd_t pmd)
{
	return pmd_clear_flags(pmd, __PAGE_ACCESSED);
}

static inline pmd_t pmd_mkyoung(pmd_t pmd)
{
	return pmd_set_flags(pmd, __PAGE_ACCESSED);
}

static inline pmd_t pmd_mkwrite(pmd_t pmd)
{
	return pmd_set_flags(pmd, __PAGE_RW);
}

/*
 * Mask out unsupported bits in a present pgprot. Non-present pgprots
 * can use those bits for other purposes, so leave them be.
 */
extern pteval_t __supported_pte_mask;

static inline pgprotval_t massage_pgprot(pgprot_t pgprot)
{
	pgprotval_t protval = pgprot_val(pgprot);

	if (protval & __PAGE_PRESENT)
		protval &= __supported_pte_mask;

	return protval;
}

static inline pte_t pfn_pte(unsigned long page_nr, pgprot_t pgprot)
{
	return __pte(((phys_addr_t)page_nr << PAGE_SHIFT) |
			massage_pgprot(pgprot));
}

static inline pmd_t pfn_pmd(unsigned long page_nr, pgprot_t pgprot)
{
	return __pmd(((phys_addr_t)page_nr << PAGE_SHIFT) |
			massage_pgprot(pgprot));
}

#define pte_pgprot(x) __pgprot(pte_flags(x))
#define pmd_pgprot(x) __pgprot(pmd_flags(x))
#define pud_pgprot(x) __pgprot(pud_flags(x))

/*
 * Batch 5, helpers to pinpoint entries within tables.
 */

/*
 * The pte page can be thought of an array like this: pte_t[PTRS_PER_PTE]
 *
 * This function returns the index of the entry in the pte page which would
 * control the given virtual address
 */
static inline unsigned long pte_index(unsigned long address)
{
	return (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
}

static inline unsigned long pmd_page_vaddr(pmd_t pmd)
{
	return (unsigned long)__va(pmd_val(pmd) & pmd_pfn_mask(pmd));
}

static inline pte_t *pte_offset_kernel(pmd_t *pmd, unsigned long address)
{
	return (pte_t *)pmd_page_vaddr(*pmd) + pte_index(address);
}

/**
 * pte_offset - Find an entry in the first-level page table
 */
static inline pte_t *pte_offset(pmd_t *pmd, unsigned long address)
{
	return ((pte_t *)pmd_page_vaddr(*pmd)) + pte_index(address);
}

/*
 * The pmd page can be thought of an array like this: pmd_t[PTRS_PER_PMD]
 *
 * This macro returns the index of the entry in the pmd page which would
 * control the given virtual address
 */
static inline unsigned long pmd_index(unsigned long address)
{
	return (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
}

static inline int pmd_bad(pmd_t pmd)
{
	return (pmd_flags(pmd) & ~__PAGE_USER) != __KERNEL_TABLE;
}

#if CONFIG_PGTABLE_LEVELS > 2
static inline int pud_none(pud_t pud)
{
	return pud_val(pud) == 0;
}

static inline int pud_present(pud_t pud)
{
	return pud_flags(pud) & __PAGE_PRESENT;
}

static inline unsigned long pud_page_vaddr(pud_t pud)
{
	return (unsigned long)__va(pud_val(pud) & pud_pfn_mask(pud));
}

/**
 * pmd_offset - Find an entry in the second-level page table
 */
static inline pmd_t *pmd_offset(pud_t *pud, unsigned long address)
{
	return ((pmd_t *)pud_page_vaddr(*pud)) + pmd_index(address);
}

static inline int pud_bad(pud_t pud)
{
	return (pud_flags(pud) & ~__PAGE_USER) != __KERNEL_TABLE
}
#endif /* CONFIG_PGTABLE_LEVELS > 2 */
/*
 * ELSE those functions are defined in <asm-generic/pgtable-nopmd.h>
 */

#if CONFIG_PGTABLE_LEVELS > 3
static inline int pgd_present(pgd_t pgd)
{
	return pgd_flags(pgd) & __PAGE_PRESENT;
}

static inline unsigned long pgd_page_vaddr(pgd_t pgd)
{
	return (unsigned long)__va(pgd_val(pgd) & PTE_PFN_MASK);
}

/*
 * The pud page can be thought of an array like this: pud_t[PTRS_PER_PUD]
 *
 * This macro returns the index of the entry in the pud page which would
 * control the given virtual address
 */
static inline unsigned long pud_index(unsigned long address)
{
	return (address >> PUD_SHIFT) & (PTRS_PER_PUD - 1);
}

/**
 * pud_offset - Find an entry in the third-level page table
 */
static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return ((pud_t *)pgd_page_vaddr(*pgd)) + pud_index(address);
}

static inline int pgd_bad(pgd_t pgd)
{
	return (pgd_flags(pgd) & ~_PAGE_USER) != _KERNPG_TABLE;
}

static inline int pgd_none(pgd_t pgd)
{
	return !pgd_val(pgd);
}
#endif /* CONFIG_PGTABLE_LEVELS > 3 */
/*
 * ELSE those functions are defined in <asm-generic/pgtable-nopud.h>
 */

/*
 * The pgd page can be thought of an array like this: pgd_t[PTRS_PER_PGD]
 *
 * This macro returns the index of the entry in the pgd page which would
 * control the given virtual address
 */
static inline unsigned long pgd_index(unsigned long address)
{
	return (address >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1);
}

/**
 * pgd_offset() returns a (pgd_t *)
 * pgd_index() is used get the offset into the pgd page's array of pgd_t's;
 */
#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index((address)))

/*
 * a shortcut which implies the use of the kernel's pgd, instead
 * of a process's
 */
#define pgd_offset_k(addr)	pgd_offset(&init_mm, (address))

#define KERNEL_PGD_BASE		pgd_index(PAGE_OFFSET)
#define KERNEL_PGD_PTRS		(PTRS_PER_PGD - KERNEL_PGD_BASE)

/* local pte updates need not use xchg for locking */
static inline pte_t native_local_ptep_get_and_clear(pte_t *ptep)
{
	pte_t res = *ptep;

	/* Pure native function needs no input for mm, addr */
	native_pte_clear(NULL, 0, ptep);
	return res;
}

static inline pmd_t native_local_pmdp_get_and_clear(pmd_t *pmdp)
{
	pmd_t res = *pmdp;

	native_pmd_clear(pmdp);
	return res;
}

static inline void native_set_pte_at(struct mm_struct *mm, unsigned long addr,
				     pte_t *ptep , pte_t pte)
{
	native_set_pte(ptep, pte);
}

#ifndef CONFIG_PARAVIRT
/*
 * Rules for using pte_update - it must be called after any PTE update which
 * has not been done using the set_pte / clear_pte interfaces.  It is used by
 * shadow mode hypervisors to resynchronize the shadow page tables.  Kernel PTE
 * updates should either be sets, clears, or set_pte_atomic for P->P
 * transitions, which means this hook should only be called for user PTEs.
 * This hook implies a P->P protection or access change has taken place, which
 * requires a subsequent TLB flush.
 */
#define pte_update(mm, addr, ptep)		do { } while (0)
#endif

/*
 * We only update the dirty/accessed state if we set
 * the dirty bit by hand in the kernel, since the hardware
 * will do the accessed bit for us, and we don't want to
 * race with other CPU's that might be updating the dirty
 * bit at the same time.
 */
struct vm_area_struct;

#define  __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
extern int ptep_set_access_flags(struct vm_area_struct *vma,
				 unsigned long address, pte_t *ptep,
				 pte_t entry, int dirty);

#define __HAVE_ARCH_PTEP_TEST_AND_CLEAR_YOUNG
extern int ptep_test_and_clear_young(struct vm_area_struct *vma,
				     unsigned long addr, pte_t *ptep);

#define __HAVE_ARCH_PTEP_CLEAR_YOUNG_FLUSH
extern int ptep_clear_flush_young(struct vm_area_struct *vma,
				  unsigned long address, pte_t *ptep);

#define __HAVE_ARCH_PTEP_GET_AND_CLEAR
static inline pte_t ptep_get_and_clear(struct mm_struct *mm, unsigned long addr,
				       pte_t *ptep)
{
	pte_t pte = native_ptep_get_and_clear(ptep);
	pte_update(mm, addr, ptep);
	return pte;
}

#define __HAVE_ARCH_PTEP_SET_WRPROTECT
static inline void ptep_set_wrprotect(struct mm_struct *mm,
				      unsigned long addr, pte_t *ptep)
{
	clear_bit(__PAGE_BIT_RW, (unsigned long *)&ptep->pte);
	pte_update(mm, addr, ptep);
}


/*
 * clone_pgd_range(pgd_t *dst, pgd_t *src, int count);
 *
 *  dst - pointer to pgd range anwhere on a pgd page
 *  src - ""
 *  count - the number of pgds to copy.
 *
 * dst and src can be on the same page, but the range must not overlap,
 * and must not cross a page boundary.
 */
static inline void clone_pgd_range(pgd_t *dst, pgd_t *src, int count)
{
       memcpy(dst, src, count * sizeof(pgd_t));
}

static inline unsigned long pages_to_mb(unsigned long npg)
{
	return npg >> (20 - PAGE_SHIFT);
}

/* Well... */
void __init init_mem_mapping(void);
void __init init_mem_init(void);
void __init early_alloc_pgt_buf(void);

void set_pte_vaddr(unsigned long vaddr, pte_t pteval);

void __init early_ioremap_page_table_range_init(void);

void __init paging_init(void);

#ifdef CONFIG_X86_32
void __init native_pagetable_init(void);
#else
#define native_pagetable_init        paging_init
#endif

#include <asm-generic/pgtable.h>

#endif /* _ASM_X86_PGTABLE_H_ */
