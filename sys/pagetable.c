#include <sys/pagetable.h>
#include <sys/freelist.h>

void setPageTables(){
	pml4etable = (PML4E *)getFreeFrame();
	pdpetable = (PDPE *)getFreeFrame();
	pdetable = (PDE *)getFreeFrame();
	ptetable = (PTE *)getFreeFrame();

	(pml4etable + 511)->pageDirectoryPointer = ((uint64_t)pdpetable >> 12);
	(pdpetable + 510)->pageDirectoryBase = ((uint64_t)pdetable >> 12);
	(pdetable + 1)->pageTableBase = ((uint64_t)ptetable >> 12);
}
