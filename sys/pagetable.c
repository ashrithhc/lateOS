#include <sys/pagetable.h>
#include <sys/freelist.h>
#include <sys/kprintf.h>
extern char kernmem;

void *memset(void *s, int c, size_t n)
{
        uint64_t* p=s;
        while(n--)
                *p++ = (uint64_t)c;
        return s;
}

void id_paging(PTE *firstPTE, uint64_t physbase, uint64_t physfree){
        for (; physbase < physfree ; physbase += 4096, firstPTE ++){
                firstPTE->physicalPageBase = (physbase>>12);
		firstPTE->p = 1;
		firstPTE->rw = 1;
		firstPTE->us = 1;
        }
}

void setPageTables(void *physbase,void *physfree){
	pml4etable = (PML4E *)getFreeFrame();
	
	/*Remap kernel memory*/
	PDPE *pdpetable = (PDPE *)getFreeFrame();
	PDE *pdetable = (PDE *)getFreeFrame();
	PTE *ptetable = (PTE *)getFreeFrame();

	memset(pml4etable, 7, 512);
	memset(pdpetable, 7, 512);
	memset(pdetable, 7, 512);
	memset(ptetable, 7, 512);

	uint64_t memToMap = (uint64_t)&kernmem;

	(pml4etable + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer = ((uint64_t)pdpetable>>12);
	(pdpetable + ((memToMap >> (12+9+9)) & 511))->pageDirectoryBase = ((uint64_t)pdetable>>12);
	(pdetable + ((memToMap >> (12+9)) & 511))->pageTableBase = ((uint64_t)ptetable>>12);

	id_paging(ptetable + ((memToMap >> 12) & 511), (uint64_t)physbase, (uint64_t)(physfree));

	/*Remap video memory*/
	memToMap = (uint64_t)(((uint64_t)&kernmem) + 0xb8000);

//	(pml4etable + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer = ((uint64_t)pdpevideo>>12);
//	(pdpevideo+ ((memToMap >> (12+9+9)) & 511))->pageDirectoryBase = ((uint64_t)pdevideo>>12);
//	(pdetable + ((memToMap >> (12+9)) & 511))->pageTableBase = ((uint64_t)ptevideo>>12);

	id_paging(ptetable + ((memToMap >> 12) & 511), (uint64_t)0xb8000, (uint64_t)0xb8000+160*25);

	__asm__ __volatile__ ("movq %0, %%cr3": : "a" (pml4etable));
}

