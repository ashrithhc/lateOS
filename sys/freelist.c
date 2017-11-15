#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/freelist.h>

uint32_t* loader_stack;
extern char kernmem, physbase;

void initializeFreelist(uint32_t *modulep){
	uint16_t pageSize = 4*0x1000;
	uint64_t physfree = 0x217000;
	struct freeList* head = (struct freeList*)physfree;
	struct freeList *tempNode = head, *prev = NULL;
	uint64_t base;
	uint32_t j, numPages;

	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;

	while(modulep[0] != 0x9001) modulep += modulep[1]+2;

	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			numPages = smap->length / pageSize;
			base = smap->base;
			for (j=0; j<numPages; j++){
				tempNode->phyaddr = base;
				if (prev) {
					prev->next = tempNode;
				}
				prev = tempNode;
				tempNode += sizeof(struct freeList);
				base += pageSize;
			}
			kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
		}
	}

	kprintf("I want to print page size = %x\n", pageSize);
}

