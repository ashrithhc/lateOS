#include <sys/defs.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/kprintf.h>

# define kernbase 0xffffffff80000000
# define validatebits 0xFFFFFFFFFFFFF000

static freelist* head = NULL;
extern char kernmem, physbase;
static uint64_t index =0;
static uint64_t *pml4e, *pdpte, *pde, *pte;
static uint64_t k_cr3 =0;
uint16_t pageSize = 0x1000;

void *memset(void *s, int c, size_t n)
{
	unsigned char* p=s;
	while(n--)
		*p++ = (unsigned char)c;
	return s;
}

uint64_t *mapCurrentPageTable(uint64_t virtual, int num, uint64_t* physical){
	virtual += num * pageSize;
	*(pte + ((virtual >> 12 ) & 511)) = ((uint64_t)physical & validatebits) | 3;
	return (uint64_t*) virtual;
}

void setupPageTables(uint64_t physbase, uint64_t physfree){
	pml4e = (uint64_t*) getFreeFrame();
	pdpte = (uint64_t*) getFreeFrame();
	*(pml4e + ((kernbase >> (12+9+9+9)) & 511)) = ((uint64_t)pdpte & validatebits) | 3;
	pde = (uint64_t*) getFreeFrame();
	*(pdpte + ((kernbase >> (12+9+9)) & 511)) = ((uint64_t)pde & validatebits) | 3;
	pte = (uint64_t*) getFreeFrame();
	*(pde + ((kernbase >> (12+9)) & 511)) = ((uint64_t)pte & validatebits) | 3;

	uint64_t virtual = (uint64_t)kernbase;
	while (physbase < physfree){
		if(((virtual >> 12) & 511) != 0){
			*(pte + ((virtual >> 12 ) & 511)) = (physbase & validatebits) | 3;
		}
		else{
			pte = ((uint64_t*)getFreeFrame());
			*(pde + ((virtual >> (12+9) ) & 511)) = ((uint64_t)pte & validatebits) | 3;
			*(pte + ((virtual >> 12 ) & 511)) = (physbase & validatebits) | 3;		
		}
		virtual += pageSize;
		physbase += pageSize;
	}

	k_cr3 = (uint64_t) pml4e;

	pml4e = mapCurrentPageTable(virtual, 1, pml4e);
	pdpte = mapCurrentPageTable(virtual, 2, pdpte);
	pde = mapCurrentPageTable(virtual, 3, pde);
	pte = mapCurrentPageTable(virtual, 4, pte);

	__asm__ __volatile__("movq %0,%%cr3"::"r"(k_cr3));	
}

void initializeFreelist(uint32_t *modulep, void *physbase, void *physfree){
	struct smap_t* smap;
	uint64_t lastFreeFrame = 0, base;
	freelist* last = NULL;

	while(modulep[0] != 0x9001) modulep += modulep[1]+2;

	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1  && smap->length != 0) {
			base = smap->base;
			while(base < (smap->base + smap->length)){
		        index = ((uint64_t)base)/pageSize;

				if((base > ((uint64_t)physfree + 0x1000)) && (base + pageSize < (smap->base + smap->length))){
					if(head == NULL){
						(&pagelist[index])->address = base;
						(&pagelist[index])->next = head;
		                (&pagelist[index])->ref_count = 0;
		                head = &pagelist[index];
						last = head;	
					}
					else{
						(&pagelist[index])->address = base;
						(&pagelist[index])->next = NULL;
		                (&pagelist[index])->ref_count = 0;
		                last->next = &pagelist[index];
						last = &pagelist[index];
					}	
				index++;
				}
				base += pageSize;
			}	
			kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
			lastFreeFrame = smap->base + smap->length; 
	    }
	}

	kprintf("Kernmem = %x\n", (uint64_t)&kernmem);
	kprintf("Physbase = %x\n", (uint64_t)&physbase);
	kprintf("Kernbase = %x\n", (uint64_t)&kernmem - (uint64_t)&physbase);
	setupPageTables((uint64_t)0, lastFreeFrame);
}

uint64_t getFreeFrame(){
	freelist* temp = head;
    temp->ref_count = 1;
	head = head->next;
	return temp->address;
}

uint64_t getNewPage(){
	uint64_t newframe = (uint64_t)getFreeFrame();	
	mapNewFrame(kernbase + newframe, newframe);
	return (kernbase + newframe);
}

void free(uint64_t add){
	return;
}

void switchtokern(){
__asm__ __volatile__("movq %0,%%cr3;"::"r"((uint64_t)k_cr3));
}

uint64_t kmalloc(int size){
	int no_pages = (size/pageSize)+1;
	uint64_t add = getNewPage();
	no_pages--;
	if(no_pages>0){
		for(int i=0;i<no_pages;i++){
			getNewPage();
		}
	}
	return add;
}

uint64_t* getPTE(uint64_t address){
        uint64_t* pml4e = (uint64_t*)(currentTask->pml4e + kernbase);
    	uint64_t* pdpte = (uint64_t*)((*(pml4e + ((address >> (12+9+9+9)) & 511)) & validatebits) + kernbase);
    	uint64_t* pdpe = (uint64_t*)((*(pdpte + ((address >> (12+9+9)) & 511)) & validatebits) + kernbase);
        uint64_t* pte = (uint64_t*)((*(pdpe + ((address >> (12+9)) & 511)) & validatebits) + kernbase);
        return pte;
}

void mapNewFrameNew(uint64_t virtual, uint64_t physical){
	uint64_t* pdpte = (uint64_t *) getFreeFrame();
	*(pml4e + ((virtual >> (12+9+9+9) ) & 511)) = ((uint64_t)pdpte & validatebits) | 3;
	pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);

	uint64_t* pdpe = (uint64_t *)getFreeFrame();
	pdpte[((virtual >> (12+9+9) ) & 511)] = ((uint64_t)pdpe & validatebits) | 3;
	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);

	uint64_t* pte = (uint64_t *)getFreeFrame();
	pdpe[((virtual >> (12+9) ) & 511)] = ((uint64_t)pte & validatebits) | 3;
	pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);

	pte[((virtual >> 12 ) & 511)] =  ((uint64_t)physical & validatebits) | 3;
}

void mapFreeFrameExistYes(uint64_t virtual, uint64_t physical){
	uint64_t* pdpe =(uint64_t *) getFreeFrame();
	pdpte[((virtual >> (12+9+9) ) & 511)] = ((uint64_t)pdpe & validatebits) | 3;
	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);

	uint64_t* pte = (uint64_t *)getFreeFrame();
	pdpe[((virtual >> (12+9) ) & 511)] = ((uint64_t)pte & validatebits) | 3;
	pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
	pte[((virtual >> 12 ) & 511)] =  ((uint64_t)physical & validatebits) | 3;
}

void mapFreeFrameExistNo(uint64_t virtual, uint64_t physical){
	uint64_t* pdpe = (uint64_t *)(pdpte[((virtual >> (12+9+9) ) & 511)] &validatebits);
	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);

	if( !(pdpe[((virtual >> (12+9) ) & 511)] & 1)){
		uint64_t* pte = (uint64_t *)getFreeFrame();
		pdpe[((virtual >> (12+9) ) & 511)] = ((uint64_t)pte & validatebits) | 3;

		pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
		pte[((virtual >> 12 ) & 511)] =  ((uint64_t)physical & validatebits) | 3;
	}
	else{	
		uint64_t* pte = (uint64_t *)(pdpe[((virtual >> (12+9) ) & 511)] &validatebits);	
		pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
		pte[((virtual >> 12 ) & 511)] = ((uint64_t)physical & validatebits) | 3;
	}
}

void mapFreeFrameExist(uint64_t virtual, uint64_t physical){
	uint64_t* pdpte = (uint64_t *)(*(pml4e + ((virtual >> (12+9+9+9) ) & 511)) & validatebits);
	pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);
	if( !(pdpte[((virtual >> (12+9+9) ) & 511)] & 1)) mapFreeFrameExistYes(virtual, physical);
	else mapFreeFrameExistNo(virtual, physical);
}

void mapNewFrame(uint64_t virtual, uint64_t physical){
	if(!(*(pml4e + ((virtual >> (12+9+9+9) ) & 511)) & 1)) mapNewFrameNew( virtual, physical);
	else mapFreeFrameExist( virtual, physical);
}

void setNewPagePDPTE(uint64_t virtual, uint64_t physical, uint64_t* pml4){
	uint64_t* pdpte = (uint64_t *)getFreeFrame();
	*(pml4 + ((virtual >> (12+9+9+9) ) & 511)) = ((uint64_t)pdpte & validatebits) | 7;
	pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);
    memset(pdpte,0,pageSize);

	uint64_t* pdpe = (uint64_t *)getFreeFrame();
	*(pdpte + ((virtual >> (12+9+9) ) & 511)) = ((uint64_t)pdpe & validatebits) | 7;
	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);
    memset(pdpe,0,pageSize);

	uint64_t* pte = (uint64_t *)getFreeFrame();
	*(pdpe + ((virtual >> (12+9) ) & 511)) = ((uint64_t)pte & validatebits) | 7;
	pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
    memset(pte,0,pageSize);

	*(pte + ((virtual >> 12 ) & 511)) =  ((uint64_t)physical & validatebits) | 7;
}

void setNewPagePDPE(uint64_t virtual, uint64_t physical, uint64_t* pml4, uint64_t* pdpte){
	uint64_t* pdpe =(uint64_t *) getFreeFrame();
	*(pdpte + ((virtual >> (12+9+9) ) & 511)) = ((uint64_t)pdpe & validatebits) | 7;

	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);
    memset(pdpe,0,pageSize);
	uint64_t* pte = (uint64_t *)getFreeFrame();
	*(pdpe + ((virtual >> (12+9) ) & 511)) = ((uint64_t)pte & validatebits) | 7;

	pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
    memset(pte,0,pageSize);
	*(pte + ((virtual >> 12 ) & 511)) =  ((uint64_t)physical & validatebits) | 7;
}

void setExistingPagePDPE(uint64_t virtual, uint64_t physical, uint64_t* pml4, uint64_t* pdpte){
	uint64_t* pdpe = (uint64_t *)(*(pdpte + ((virtual >> (12+9+9) ) & 511)) &validatebits);
	pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);
	if( !(*(pdpe + ((virtual >> (12+9) ) & 511)) & 1)){
		uint64_t* pte = (uint64_t *)getFreeFrame();
		*(pdpe + ((virtual >> (12+9) ) & 511)) = ((uint64_t)pte & validatebits) | 7;

		pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
        memset(pte,0,pageSize);
		*(pte + ((virtual >> 12 ) & 511)) =  ((uint64_t)physical & validatebits) | 7;
	}
	else{
		uint64_t* pte = (uint64_t *)(*(pdpe + ((virtual >> (12+9) ) & 511)) &validatebits);
		pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
		*(pte + ((virtual >> 12 ) & 511)) = ((uint64_t)physical & validatebits) | 7;
	}
}

void setExistingPagePDPTE(uint64_t virtual, uint64_t physical, uint64_t* pml4){
	uint64_t* pdpte = (uint64_t *)(*(pml4 + ((virtual >> (12+9+9+9) ) & 511)) & validatebits);
	pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);
	if( !(*(pdpte + ((virtual >> (12+9+9) ) & 511)) & 1)) setNewPagePDPE(virtual, physical, pml4, pdpte);
	else setExistingPagePDPE(virtual, physical, pml4, pdpte);
}

void init_pages_for_process(uint64_t virtual, uint64_t physical, uint64_t* pml4){
	*(pml4 + 511) = (*(pml4e + 511) & validatebits) | 7;
	if(!(*(pml4 + ((virtual >> (12+9+9+9) ) & 511)) & 1)) setNewPagePDPTE(virtual, physical, pml4);
	else setExistingPagePDPTE(virtual, physical, pml4);
}

void copytables(taskStruct *p, taskStruct *c){
	uint64_t* pml4eParent = (uint64_t *)(p->pml4e + kernbase);
	uint64_t* pml4eChild =(uint64_t *) (c->pml4e + kernbase);
	pml4eChild[511] = pml4eParent[511];
	for(int i=0; i<511; i++){
		if(pml4eParent[i] & 1){
			uint64_t* pdpteChild = (uint64_t *)getNewPage();
            memset(pdpteChild,0,pageSize);
			pml4eChild[i] = ((uint64_t)((uint64_t)pdpteChild -((uint64_t)kernbase)) & validatebits) | 7;
			uint64_t* pdpte = (uint64_t *)(pml4eParent[i] & validatebits);
			pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);
			for(int j=0;j<512;j++){
				if(pdpte[j] & 1){
					uint64_t* pdpeChild = (uint64_t *)getNewPage();
                    memset(pdpeChild,0,pageSize);
                    pdpteChild[j] = ((uint64_t)((uint64_t)pdpeChild -((uint64_t)kernbase)) & validatebits) | 7;
					uint64_t* pdpe = (uint64_t *)(pdpte[j] & validatebits);
					pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);
					for(int k=0;k<512;k++){
						if(pdpe[k] & 1){
							uint64_t* pteChild = (uint64_t *)getNewPage();
                            memset(pteChild,0,pageSize);
                            pdpeChild[k] = ((uint64_t)((uint64_t)pteChild -((uint64_t)kernbase)) & validatebits) | 7;
							uint64_t* pte = (uint64_t *)(pdpe[k] & validatebits);
							pte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pte);
							for(int l=0;l<512;l++){
								if(pte[l]&1){
									pagelist[(((uint64_t)pte[l] & validatebits))/pageSize].ref_count+=1;
									pte[l] = (pte[l] & 0xFFFFFFFFFFFFFFFD) | (0x0000000000000200);
									pteChild[l] = pte[l];
								}
							}
						}	
					}
				}
			}
		}	
	}
}

void dealloc_pml4(uint64_t pm4){
    uint64_t* pml4eParent = (uint64_t *)(pm4 + kernbase);
    for(int i=0;i<511;i++){
        if(pml4eParent[i]&1){
            uint64_t* pdpte = (uint64_t *)(pml4eParent[i] & validatebits);
            pdpte = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpte);
            for (int j = 0; j < 512; ++j) {
                if(pdpte[j] & 1){
                    uint64_t* pdpe = (uint64_t *)(pdpte[j] & validatebits);
                    pdpe = (uint64_t *)((uint64_t)kernbase + (uint64_t)pdpe);
                    for (int k = 0; k < 512 ; ++k) {
                        if(pdpe[k]&1) {
                            uint64_t *pte = (uint64_t *)(pdpe[k] & validatebits);
                            pte = (uint64_t*)((uint64_t) kernbase + (uint64_t) pte);
                            for (int l = 0; l < 512; ++l) {
                                if (pte[l] & 1) {
                                    free(((uint64_t) pte[l] & validatebits));
                                }
                                pte[l] = 0;
                            }
                            free(pdpe[k] & validatebits);
                        }
                        pdpe[k]=0;

                    }
                    free(pdpte[j] & validatebits);
                }
                pdpte[j]=0;
            }
            free(pml4eParent[i] & validatebits);
        }
        pml4eParent[i]=0;
    }
}
