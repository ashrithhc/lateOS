#ifndef _PHY_H
#define _PHY_H
#include <sys/process.h>

typedef struct freelist{
	uint64_t address;
    int ref_count;
	struct freelist* next; 
}freelist;

struct freelist pagelist[75000]; 

typedef struct smap_t {
    uint64_t base, length;
    uint32_t type;
}__attribute__((packed)) smap_t;

uint64_t getFreeFrame();
uint64_t getNewPage();
void mem_map(smap_t* sm, uint64_t physbase, uint64_t physfree);
void setupPageTables();
void init_pages_for_process(uint64_t vaddr_s,uint64_t vaddr_e,uint64_t* pml4);
uint64_t kmalloc(int size);
void copytables(task_struct* a,task_struct* b);
uint64_t* getPTE(uint64_t v);
void switchtokern();
//int getrefcount(uint64_t add);
void free(uint64_t add);
void dealloc_pml4(uint64_t pm4);
void initializeFreelist(uint32_t *modulep, void *physbase, void *physfree);
void mapNewFrame(uint64_t vaddr_s, uint64_t phy);
#endif
