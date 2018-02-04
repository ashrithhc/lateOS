#include <sys/defs.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/kprintf.h>

# define kernbase 0xffffffff80000000
# define validatebits 0xFFFFFFFFFFFFF000

static freelist* head = NULL;
extern char kernmem, physbase;
static uint64_t index =0;
static uint64_t *pml4e, *pdpte,*pde,*pte;
static uint64_t pml4_idx,pdpt_idx,pd_idx,pt_idx;
static uint64_t viraddr;
static uint64_t k_cr3 =0;
uint16_t pageSize = 0x1000;

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
        /*int index = ((uint64_t)add)/pageSize;
        if((&pagelist[index])->address == add && ((&pagelist[index])->ref_count >1)){
            (&pagelist[index])->ref_count--;
            return;
        }
        if((&pagelist[index])->address == add){
            (&pagelist[index])->next = head;
            head = &pagelist[i];
        }*/
}

void switchtokern(){
__asm__ volatile("movq %0,%%cr3;"::"r"((uint64_t)k_cr3));
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
        uint64_t* pml4e = (uint64_t*)(r->pml4e + kernbase);
    	uint64_t* pdpte = (uint64_t*)((*(pml4e + ((address >> (12+9+9+9)) & 511)) & validatebits) + kernbase);
    	uint64_t* pdpe = (uint64_t*)((*(pdpte + ((address >> (12+9+9)) & 511)) & validatebits) + kernbase);
        uint64_t* pte = (uint64_t*)((*(pdpe + ((address >> (12+9)) & 511)) & validatebits) + kernbase);
        return pte;
}

void mapNewFrame(uint64_t virtual, uint64_t physical){
	if(!(pml4e[((virtual >> (12+9+9+9) ) & 511)] & 1)){
		uint64_t* p3 = (uint64_t *)getFreeFrame();
		
		pml4e[((virtual >> (12+9+9+9) ) & 511)] = ((uint64_t)p3 & validatebits) | 3;

		p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);

		int id2 = (virtual >> (12+9+9) ) & 511;
		uint64_t* p2 = (uint64_t *)getFreeFrame();
		p3[id2] = ((uint64_t)p2 & validatebits) | 3;

		p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);

		int id3 = (virtual >> 21 ) & 511;
		uint64_t* p1 = (uint64_t *)getFreeFrame();
		p2[id3] = ((uint64_t)p1 & validatebits) | 3;

		p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);

		int id4 = (virtual >> 12 ) & 511;
		p1[id4] =  ((uint64_t)physical & validatebits) | 3;
		return ;
	}
	else{
		uint64_t* p3 = (uint64_t *)(pml4e[((virtual >> (12+9+9+9) ) & 511)] & validatebits);
		int id2 =  (virtual >> (12+9+9) ) & 511;
		p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);
		if( !(p3[id2] & 1)){	
			uint64_t* p2 =(uint64_t *) getFreeFrame();
			p3[id2] = ((uint64_t)p2 & validatebits) | 3;

			p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);

			int id3 = (virtual >> 21 ) & 511;
			uint64_t* p1 = (uint64_t *)getFreeFrame();
			p2[id3] = ((uint64_t)p1 & validatebits) | 3;

			p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);

			int id4 = (virtual >> 12 ) & 511;
			p1[id4] =  ((uint64_t)physical & validatebits) | 3;
			return;
		}
		else{
			uint64_t* p2 = (uint64_t *)(p3[id2] &validatebits);
			int id3 =  (virtual >> 21) & 511;
			p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);

			if( !(p2[id3] & 1)){
				uint64_t* p1 = (uint64_t *)getFreeFrame();
				p2[id3] = ((uint64_t)p1 & validatebits) | 3;

				p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);

				int id4 = (virtual >> 12 ) & 511;
				p1[id4] =  ((uint64_t)physical & validatebits) | 3;
				return;
			}
			else{	
				uint64_t* p1 = (uint64_t *)(p2[id3] &validatebits);	
				int id4 = (virtual >> 12 ) & 511;

				p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);

				p1[id4] = ((uint64_t)physical & validatebits) | 3;
				return;
			}
		}
	}
}

void init_pages_for_process(uint64_t vaddr_s, uint64_t phy, uint64_t* pml4){
	pml4[511] = (pml4e[511] & validatebits) | 7;
	int id1 = (vaddr_s >> (12+9+9+9) ) & 511;
	if(!(pml4[id1] & 1)){
		uint64_t* p3 = (uint64_t *)getFreeFrame();
		pml4[id1] = ((uint64_t)p3 & validatebits) | 7;
		p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);

        memset(p3,0,pageSize);

        int id2 = (vaddr_s >> (12+9+9) ) & 511;
		uint64_t* p2 = (uint64_t *)getFreeFrame();
		p3[id2] = ((uint64_t)p2 & validatebits) | 7;

		p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);
        memset(p2,0,pageSize);
        int id3 = (vaddr_s >> 21 ) & 511;
		uint64_t* p1 = (uint64_t *)getFreeFrame();
		p2[id3] = ((uint64_t)p1 & validatebits) | 7;

		p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);
        memset(p1,0,pageSize);
        int id4 = (vaddr_s >> 12 ) & 511;
		p1[id4] =  ((uint64_t)phy & validatebits) | 7;
		return ;
	}

	else{
		uint64_t* p3 = (uint64_t *)(pml4[id1] & validatebits);
		int id2 =  (vaddr_s >> (12+9+9) ) & 511;
		p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);
		if( !(p3[id2] & 1)){
			uint64_t* p2 =(uint64_t *) getFreeFrame();
			p3[id2] = ((uint64_t)p2 & validatebits) | 7;

			p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);
            memset(p2,0,pageSize);
            int id3 = (vaddr_s >> 21 ) & 511;
			uint64_t* p1 = (uint64_t *)getFreeFrame();
			p2[id3] = ((uint64_t)p1 & validatebits) | 7;

			p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);
            memset(p1,0,pageSize);

            int id4 = (vaddr_s >> 12 ) & 511;
			p1[id4] =  ((uint64_t)phy & validatebits) | 7;
			return;
		}
		else{
			uint64_t* p2 = (uint64_t *)(p3[id2] &validatebits);
			int id3 =  (vaddr_s >> 21) & 511;
			p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);
			if( !(p2[id3] & 1)){
				uint64_t* p1 = (uint64_t *)getFreeFrame();

				p2[id3] = ((uint64_t)p1 & validatebits) | 7;

				p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);
                memset(p1,0,pageSize);
                int id4 = (vaddr_s >> 12 ) & 511;
				p1[id4] =  ((uint64_t)phy & validatebits) | 7;

				return;
			}
			else{
				uint64_t* p1 = (uint64_t *)(p2[id3] &validatebits);
				int id4 = (vaddr_s >> 12 ) & 511;

				p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);

				p1[id4] = ((uint64_t)phy & validatebits) | 7;
				return;
			}
		}
	}
}

void setupPageTables(uint64_t physbase, uint64_t physfree){

	viraddr = (uint64_t)kernbase;//(uint64_t)&kernmem;

	pml4_idx = (viraddr >> (12+9+9+9) ) & 511;
	pdpt_idx = (viraddr >> (12+9+9) ) & 511;
	pd_idx = (viraddr >> 21 ) & 511;
	pt_idx = (viraddr >> 12 ) & 511;

	pml4e = (uint64_t *)getFreeFrame();
	pdpte = (uint64_t *)getFreeFrame();
	pde = (uint64_t *)getFreeFrame();
	pte = (uint64_t *)getFreeFrame();
	pml4e[pml4_idx] = ((uint64_t)pdpte & validatebits) | 3;
	pdpte[pdpt_idx] = ((uint64_t)pde & validatebits) | 3;
	pde[pd_idx] = ((uint64_t)pte & (validatebits)) | 3;
	for(int j=0;physbase<physfree;j++,viraddr+=pageSize,physbase+=pageSize){
		pml4_idx = (viraddr >> (12+9+9+9) ) & 511;
		pdpt_idx = (viraddr >> (12+9+9) ) & 511;
		pd_idx = (viraddr >> 21 ) & 511;
		pt_idx = (viraddr >> 12 ) & 511;   

		if(pt_idx!=0){
			pte[pt_idx] = (physbase & validatebits) | 3;
		}
		else{
			pte = ((uint64_t*)getFreeFrame());
			pde[pd_idx] = ((uint64_t)pte & (validatebits)) | 3;
			pte[pt_idx] = (physbase & validatebits) | 3;		
		}


		//		map(viraddr,physbase);
	}
	k_cr3 = (uint64_t)pml4e;
	// Temp Fix, we need a better approach

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 511;
	pte[pt_idx] = ( (uint64_t)pml4e & validatebits) | 3;
	pml4e = (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 511;
	pte[pt_idx] = ( (uint64_t)pdpte & validatebits) | 3;
	pdpte = (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 511;
	pte[pt_idx] = ( (uint64_t)pde & validatebits) | 3;
	pde= (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 511;
	pte[pt_idx] = ( (uint64_t)pte & validatebits) | 3;
	pte = (uint64_t*)viraddr;

	__asm__ volatile("movq %0,%%cr3"::"r"(k_cr3));	
//	k_cr3 = (uint64_t)pml4e;
}
void copytables(task_struct* p, task_struct* c){
	uint64_t* p4 = (uint64_t *)(p->pml4e + kernbase);
	uint64_t* c4 =(uint64_t *) (c->pml4e + kernbase);
	c4[511] = p4[511];
	for(int i =0;i<511;i++){
		if(p4[i] & 1){
			uint64_t* c3 = (uint64_t *)getNewPage();
            memset(c3,0,pageSize);
			c4[i] = ((uint64_t)((uint64_t)c3 -((uint64_t)kernbase)) & validatebits) | 7;
			uint64_t* p3 = (uint64_t *)(p4[i] & validatebits);
			p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);
			for(int j=0;j<512;j++){
				if(p3[j] & 1){
					uint64_t* c2 = (uint64_t *)getNewPage();
                    memset(c2,0,pageSize);
                    c3[j] = ((uint64_t)((uint64_t)c2 -((uint64_t)kernbase)) & validatebits) | 7;
					uint64_t* p2 = (uint64_t *)(p3[j] & validatebits);
					p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);
					for(int k=0;k<512;k++){
						if(p2[k] & 1){
							uint64_t* c1 = (uint64_t *)getNewPage();
                            memset(c1,0,pageSize);
                            c2[k] = ((uint64_t)((uint64_t)c1 -((uint64_t)kernbase)) & validatebits) | 7;
							uint64_t* p1 = (uint64_t *)(p2[k] & validatebits);
							p1 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p1);
							for(int l=0;l<512;l++){
								if(p1[l]&1){
									pagelist[(((uint64_t)p1[l] & validatebits))/pageSize].ref_count+=1;
									p1[l] = (p1[l] & 0xFFFFFFFFFFFFFFFD) | (0x0000000000000200);
									c1[l] = p1[l];

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
    uint64_t* p4 = (uint64_t *)(pm4 + kernbase);
    for(int i=0;i<511;i++){
        if(p4[i]&1){
            uint64_t* p3 = (uint64_t *)(p4[i] & validatebits);
            p3 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p3);
            for (int j = 0; j < 512; ++j) {
                if(p3[j] & 1){
                    uint64_t* p2 = (uint64_t *)(p3[j] & validatebits);
                    p2 = (uint64_t *)((uint64_t)kernbase + (uint64_t)p2);
                    for (int k = 0; k < 512 ; ++k) {
                        if(p2[k]&1) {
                            uint64_t *p1 = (uint64_t *)(p2[k] & validatebits);
                            p1 = (uint64_t*)((uint64_t) kernbase + (uint64_t) p1);
                            for (int l = 0; l < 512; ++l) {
                                if (p1[l] & 1) {
                                    //  memset((uint64_t*)((p1[l] & validatebits)+kernbase),0,pageSize);
                                    free(((uint64_t) p1[l] & validatebits));
                                }
                                p1[l] = 0;
                            }
                            free(p2[k] & validatebits);
                        }
                        p2[k]=0;

                    }
                    free(p3[j] & validatebits);
                }
                p3[j]=0;
            }
            free(p4[i] & validatebits);
        }
        p4[i]=0;
    }
  //  free(pm4);
}
