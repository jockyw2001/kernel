/*
 *  linux/arch/arm/mm/pgd.c
 *
 *  Copyright (C) 1998-2005 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/slab.h>

#include <asm/cp15.h>
#include <asm/pgalloc.h>
#include <asm/page.h>
#include <asm/tlbflush.h>

#include "mm.h"

#define __pgd_alloc()	(pgd_t *)__get_free_pages(GFP_KERNEL, 2)
#define __pgd_free(pgd)	free_pages((unsigned long)pgd, 2)

DEFINE_SPINLOCK(pgd_lock);
LIST_HEAD(pgd_list);

#ifdef CONFIG_MP_PAGE_GLOBAL_DIRECTORY_SPEEDUP
#define PGD_SIZE    (PTRS_PER_PGD * sizeof(pgd_t))
#define PGD_CACHE_SIZE 512
LIST_HEAD(pgd_cache_list);
DEFINE_SPINLOCK(pgd_cache_list_spin_lock);
atomic_t pgd_cache_list_cnt = ATOMIC_INIT(0);
#endif

static inline void pgd_list_add(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_add(&page->lru, &pgd_list);
}

static inline void pgd_list_del(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_del(&page->lru);
}

/*
 * need to get a 16k page for level 1
 */
pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *new_pgd, *init_pgd;
	pud_t *new_pud, *init_pud;
	pmd_t *new_pmd, *init_pmd;
	pte_t *new_pte, *init_pte;
	unsigned long flags;

#ifdef CONFIG_MP_PAGE_GLOBAL_DIRECTORY_SPEEDUP
	spin_lock_irqsave(&pgd_cache_list_spin_lock, flags);
	if(atomic_add_unless(&pgd_cache_list_cnt, -1, 0)){
		struct list_head *list;
		BUG_ON(list_empty(&pgd_cache_list));
		list = pgd_cache_list.next;
		list_del(list);
		spin_unlock_irqrestore(&pgd_cache_list_spin_lock, flags);
		new_pgd = (pgd_t *)list;
	}
	else{
		spin_unlock_irqrestore(&pgd_cache_list_spin_lock, flags);
		new_pgd = __pgd_alloc();
	}
#else
	new_pgd = __pgd_alloc();
#endif
	if (!new_pgd)
		goto no_pgd;

	memset(new_pgd, 0, USER_PTRS_PER_PGD * sizeof(pgd_t));

	spin_lock_irqsave(&pgd_lock, flags);
	/*
	 * Copy over the kernel and IO PGD entries
	 */
	init_pgd = pgd_offset_k(0);
	memcpy(new_pgd + USER_PTRS_PER_PGD, init_pgd + USER_PTRS_PER_PGD,
		       (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));

#if !defined(CONFIG_CPU_CACHE_V7) || !defined(CONFIG_SMP)
	clean_dcache_area(new_pgd, PTRS_PER_PGD * sizeof(pgd_t));
#endif

#ifdef CONFIG_ARM_LPAE
	/*
	 * Allocate PMD table for modules and pkmap mappings.
	 */
	new_pud = pud_alloc(mm, new_pgd + pgd_index(MODULES_VADDR),
			    MODULES_VADDR);
	if (!new_pud){
		spin_unlock_irqrestore(&pgd_lock, flags);
		goto no_pud;
	}

	new_pmd = pmd_alloc(mm, new_pud, 0);
	if (!new_pmd){
		spin_unlock_irqrestore(&pgd_lock, flags);
		goto no_pmd;
	}
#endif

	pgd_list_add(new_pgd);
	spin_unlock_irqrestore(&pgd_lock, flags);

	if (!vectors_high()) {
		/*
		 * On ARM, first page must always be allocated since it
		 * contains the machine vectors. The vectors are always high
		 * with LPAE.
		 */
		new_pud = pud_alloc(mm, new_pgd, 0);
		if (!new_pud)
			goto no_pud;

		new_pmd = pmd_alloc(mm, new_pud, 0);
		if (!new_pmd)
			goto no_pmd;

		new_pte = pte_alloc_map(mm, NULL, new_pmd, 0);
		if (!new_pte)
			goto no_pte;

		init_pud = pud_offset(init_pgd, 0);
		init_pmd = pmd_offset(init_pud, 0);
		init_pte = pte_offset_map(init_pmd, 0);
		set_pte_ext(new_pte + 0, init_pte[0], 0);
		set_pte_ext(new_pte + 1, init_pte[1], 0);
		pte_unmap(init_pte);
		pte_unmap(new_pte);
	}

	return new_pgd;

no_pte:
	pmd_free(mm, new_pmd);
no_pmd:
	pud_free(mm, new_pud);
no_pud:
	spin_lock_irqsave(&pgd_lock, flags);
	pgd_list_del(new_pgd);
	spin_unlock_irqrestore(&pgd_lock, flags);
	__pgd_free(new_pgd);
no_pgd:
	return NULL;
}

void pgd_free(struct mm_struct *mm, pgd_t *pgd_base)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pgtable_t pte;
	unsigned long flags;

	if (!pgd_base)
		return;

	spin_lock_irqsave(&pgd_lock, flags);
	pgd_list_del(pgd_base);
	spin_unlock_irqrestore(&pgd_lock, flags);

	pgd = pgd_base + pgd_index(0);
	if (pgd_none_or_clear_bad(pgd))
		goto no_pgd;

	pud = pud_offset(pgd, 0);
	if (pud_none_or_clear_bad(pud))
		goto no_pud;

	pmd = pmd_offset(pud, 0);
	if (pmd_none_or_clear_bad(pmd))
		goto no_pmd;

	pte = pmd_pgtable(*pmd);
	pmd_clear(pmd);
	pte_free(mm, pte);
no_pmd:
	pud_clear(pud);
	pmd_free(mm, pmd);
no_pud:
	pgd_clear(pgd);
	pud_free(mm, pud);
no_pgd:
#ifdef CONFIG_ARM_LPAE
	/*
	 * Free modules/pkmap or identity pmd tables.
	 */
	for (pgd = pgd_base; pgd < pgd_base + PTRS_PER_PGD; pgd++) {
		if (pgd_none_or_clear_bad(pgd))
			continue;
		if (pgd_val(*pgd) & L_PGD_SWAPPER)
			continue;
		pud = pud_offset(pgd, 0);
		if (pud_none_or_clear_bad(pud))
			continue;
		pmd = pmd_offset(pud, 0);
		pud_clear(pud);
		pmd_free(mm, pmd);
		pgd_clear(pgd);
		pud_free(mm, pud);
	}
#endif
#ifdef CONFIG_MP_PAGE_GLOBAL_DIRECTORY_SPEEDUP
	spin_lock_irqsave(&pgd_cache_list_spin_lock, flags);
	if(atomic_add_unless(&pgd_cache_list_cnt, 1, PGD_CACHE_SIZE))
	{
	   struct list_head *list = (struct list_head *)pgd_base;
           list_add(list, &pgd_cache_list);
           spin_unlock_irqrestore(&pgd_cache_list_spin_lock, flags);
	   return;
	}
	   spin_unlock_irqrestore(&pgd_cache_list_spin_lock, flags);
#endif
	__pgd_free(pgd_base);
}
