#include <sys/pagetable.h>
#include <sys/freelist.h>

extern char kernmem;

void id_paging_kernel(PTE *firstPTE, uint64_t physbase, uint64_t physfree){
        for (; physbase < physfree ; physbase += 4096, firstPTE ++){
                firstPTE->physicalPageBase = (physbase);
		firstPTE->p = 1;
		firstPTE->rw = 1;
		firstPTE->us = 1;
        }
}

void setPageTables(void *physfree){
	pml4etable = (PML4E *)getFreeFrame();

	/*Move kernel memory*/
	PDPE *pdpetable = (PDPE *)getFreeFrame();
	PDE *pdetable = (PDE *)getFreeFrame();
	PTE *ptetable = (PTE *)getFreeFrame();

	(pml4etable + (((uint64_t)&kernmem >> (12+9+9+9)) & 511))->pageDirectoryPointer = ((uint64_t)pdpetable >> 12);
	(pdpetable + (((uint64_t)&kernmem >> (12+9+9)) & 511))->pageDirectoryBase = ((uint64_t)pdetable >> 12);
	(pdetable + (((uint64_t)&kernmem >> (12+9)) & 511))->pageTableBase = ((uint64_t)ptetable >> 12);

	id_paging_kernel(ptetable, (uint64_t)0, (uint64_t)physfree);

	__asm__ __volatile__ ("movq %0, %%cr3": : "a" (pml4etable));
}

