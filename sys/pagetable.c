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

void *memcpy(void *dest, const void *src, int n){
	unsigned char *retDest = (unsigned char *)dest;
	const unsigned char *retSrc = (unsigned char *)src;

	if (retSrc < retDest) for(retDest+=n, retSrc+=n; n--;) *(--retDest) = *(--retSrc);
	else while(n--) *retDest++ = *retSrc++;
	return dest;
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

	/*ID-map initial memory*/
	memToMap = (uint64_t)0;

	PDPE *pdpetable1 = (PDPE *)getFreeFrame();
	PDE *pdetable1 = (PDE *)getFreeFrame();
	PTE *ptetable1 = (PTE *)getFreeFrame();

        memset(pdpetable1, 7, 512);
        memset(pdetable1, 7, 512);
        memset(ptetable1, 7, 512);

	(pml4etable + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer = ((uint64_t)pdpetable1>>12);
	(pdpetable1 + ((memToMap >> (12+9+9)) & 511))->pageDirectoryBase = ((uint64_t)pdetable1>>12);
	(pdetable1 + ((memToMap >> (12+9)) & 511))->pageTableBase = ((uint64_t)ptetable1>>12);

	id_paging(ptetable1 + ((memToMap >> 12) & 511), (uint64_t)0, (uint64_t)0x200000);

	memToMap = (uint64_t)0x200000;

        PDPE *pdpetable2 = (PDPE *)getFreeFrame();
        PDE *pdetable2 = (PDE *)getFreeFrame();
        PTE *ptetable2 = (PTE *)getFreeFrame();

        memset(pdpetable2, 7, 512);
        memset(pdetable2, 7, 512);
        memset(ptetable2, 7, 512);

//        (pml4etable + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer = ((uint64_t)pdpetable2>>12);
//        (pdpetable2 + ((memToMap >> (12+9+9)) & 511))->pageDirectoryBase = ((uint64_t)pdetable2>>12);
        (pdetable1 + ((memToMap >> (12+9)) & 511))->pageTableBase = ((uint64_t)ptetable2>>12);

        id_paging(ptetable2 + ((memToMap >> 12) & 511), (uint64_t)0x200000, (uint64_t)0x400000);

	__asm__ __volatile__ ("movq %0, %%cr3": : "a" (pml4etable));
}

uint64_t currentCR3(){
	uint64_t pml4eCurrent;
	__asm__ volatile ("movq %0, %%cr3": : "b" (pml4eCurrent));
	return pml4eCurrent;
}

void setCR3(PML4E *pml4eNew){
	__asm__ volatile ("mov %0, %%cr3": : "b"((uint64_t)pml4eNew));
}

void *userAddressSpace(){
	PML4E *pml4eNew = (PML4E *)getFreeFrame();
	PML4E *pml4eCurrent = (PML4E *)currentCR3();

	(pml4eNew + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer = (pml4eCurrent + ((memToMap >> (12+9+9+9)) & 511))->pageDirectoryPointer;

	return (void *)pml4eNew;
}

void setChildPagetables(uint64_t pml4eChild){
	PML4E *childPML4E = (PML4E *)pml4eChild;
	PML4E *parentPML4E = (PML4E *)currentCR3();

	int pml4entry;
	for (pml4entry = 0; pml4entry < 510; pml4entry++){
		PDPE *parentPDPE = (PDPE *)((parentPML4E + pml4entry)->pageDirectoryPointer<<12);
		PDPE *childPDPE = (PDPE *)getFreeFrame();
		memset(childPDPE, 7, 512);
		(childPML4E + pml4entry)->pageDirectoryPointer = ((uint64_t)childPDPE>>12);

		int pdpeentry;
		for (pdpeentry = 0; pdpeentry < 512; pdpeentry++){
			PDE *parentPDE = (PDE *)((parentPDPE + pdpeentry)->pageDirectoryBase<<12);
			PDE *childPDE = (PDE *)getFreeFrame();
			memset(childPDE, 7, 512);
			(childPDPE + pdpeentry)->pageDirectoryBase = ((uint64_t)childPDE>>12);

			int pdeentry;
			for (pdeentry = 0; pdeentry < 512; pdeentry++){
				PTE *parentPTE = (PTE *)((parentPDE + pdeentry)->pageTableBase<<12);
				PTE *childPTE = (PTE *)getFreeFrame();
				memset(childPTE, 7, 512);
				(childPDE + pdeentry)->pageTableBase = ((uint64_t)childPTE>>12);

				int pteentry;
				for (pteentry = 0; pteentry < 512; pteentry++){
					uint64_t entry = (uint64_t)((parentPTE + pteentry)->physicalPageBase);
					(childPTE + pteentry)->physicalPageBase = entry;
				}
			}
		}
	}
}

void freeThisFrame(uint64_t vframe){
	PML4E *pml4ebase = (PML4E *)currentCR3();
	PDPE *pdpebase = (PDPE *)((pml4ebase + ((vframe >> (12+9+9+9)) & 511))->pageDirectoryPointer<<12);
	PDE *pdebase = (PDE *)((pdpebase + ((vframe >> (12+9+9)) & 511))->pageDirectoryBase<<12);
	PTE *ptebase = (PTE *)((pdebase + ((vframe >> (12+9)) & 511))->pageTableBase<<12);
	uint64_t pframe = (unt64_t)((ptebase + ((vframe >> 12) & 511))->physicalPageBase<<12);
	memset(pframe, 0, 512);
	addFrameToFreeList(pframe);
}

void freePageTable(uint64_t pml4e){
	PML4E *pml4ebase = (PML4E *)pml4e;
	int pml4entry;
	for (pml4entry = 0; pml4entry < 510; pml4entry++){
		PDPE *pdpebase = (PDPE *)((pml4ebase + pml4entry)->pageDirectoryPointer<<12);
		int pdpeentry;
		for (pdpeentry = 0; pdpeentry < 512; pdpeentry++){
			PDE *pdebase = (PDE *)((pdpebase + pdpeentry)->pageDirectoryBase<<12);
			int pdeentry;
			for (pdeentry = 0; pdeentry < 512; pdeentry++){
				PTE *ptebase = (PTE *)((pdebase + pdeentry)->pageTableBase<<12);
				int pteentry;
				for (pteentry = 0; pteentry < 512; pteentry++){
					(ptebase + pteentry)->physicalPageBase = (uint64_t)0x0;
				}
			}
		}
	}
}
