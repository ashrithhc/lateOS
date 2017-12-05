#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/freelist.h>

/*This static struct has the starting point of our free list at all times*/
static struct freeList* freeListStart;

uint32_t* loader_stack;
extern char kernmem, physbase;

/*Initialize only once, preferably on boot, sets up the free list*/
void initializeFreelist(uint32_t *modulep, void *physbase, void *physfree){
	uint16_t pageSize = 0x1000; //Our page size will be 4kB
	struct freeList* head = (struct freeList*)physfree;
	struct freeList *tempNode = head, *prev = NULL;
	uint64_t base;

	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;

	while(modulep[0] != 0x9001) modulep += modulep[1]+2;

	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			base = ((smap->base + 0x1000) % 0x1000);
			while (base < (smap->base + smap->length)){
				/*Skipping kernel memory*/
/*				if(base >= (uint64_t)physbase &&  base < (uint64_t)physfree){
					base = (uint64_t)physfree;
					continue;	
				}*/
				/*Skipping video memory*/
/*				if(base >= (uint64_t)0xb8000 && base < (uint64_t)0xb8000+160*25){
					base = (uint64_t)0xb8000+160*25;
					continue;
				}*/
				/*Skipping all addresses before physfree*/
				if(base < (uint64_t)physfree){
					base = (uint64_t)physfree + 0x100000;
					continue;
				}
				/*Creating Free List of all memory that can be used by OS*/
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
	freeListStart = head;
}

/*Returns the pointer to the current "head" of the free list*/
struct freeList* getCurrentFreeListHead(){
	return freeListStart;
}

/*Returns the phyaddr of the current "head" of the free list and moves the "head*/
uint64_t getFreeFrame(){
	uint64_t freeFrame = freeListStart->phyaddr;
	freeListStart = freeListStart->next;
	return freeFrame;
}

void printFreeList(int num){
	int i;
	struct freeList* tempAddr = freeListStart;
	for (i=0; i<num; i++){
		if (tempAddr->phyaddr < 0x200000) kprintf("%p ", tempAddr->phyaddr);
		tempAddr = tempAddr->next;
	}
}
