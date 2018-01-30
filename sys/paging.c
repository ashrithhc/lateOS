#include <sys/defs.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/kprintf.h>

static freelist* head = NULL;
extern char kernmem;
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
	uint64_t physfreeInt = (uint64_t)physfree + sizeof(pagelist);

	while(modulep[0] != 0x9001) modulep += modulep[1]+2;

	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1  && smap->length != 0) {
			base = smap->base;
			while(base < (smap->base + smap->length)){
		        index = ((uint64_t)base)/pageSize;
				if(base>physfreeInt && base+pageSize<(smap->base+smap->length)){
					if(head == NULL){
						pagelist[index].address = base;
						pagelist[index].next = head;
						pagelist[index].free = 1;
		                pagelist[index].ref_count = 0;
		                head = &pagelist[count];
						last = head;	
						index++;   				
					}
					else{
						pagelist[index].address = base;
						pagelist[index].next = NULL;
						pagelist[index].free = 1;
		                pagelist[index].ref_count = 0;
		                last->next = &pagelist[index];
						last = &pagelist[index];
						index++;
					}	
				}
				else{
					pagelist[index].address = base;
					pagelist[index].next = NULL;
		            pagelist[index].ref_count = 1;
		            pagelist[index].free = 0;
					index++;
				}
				base+=pageSize;
			}	
			kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
			lastFreeFrame = smap->base+smap->length; 
	    }
	}
	setupPageTables((uint64_t)0, lastFreeFrame);
}

void free(uint64_t add){
        int i = ((uint64_t)add)/pageSize;
        if(pagelist[i].address == add && (pagelist[i].ref_count >1)){
            pagelist[i].ref_count--;
            return;
        }
        if(pagelist[i].address == add){
            pagelist[i].free = 1;
            pagelist[i].next = head;
            head = &pagelist[i];
        }

}
int getrefcount(uint64_t add){
    return pagelist[add/pageSize].ref_count;
}
void increfcount(uint64_t add){
    pagelist[add/pageSize].ref_count+=1;
}
void switchtokern(){
__asm__ volatile("movq %0,%%cr3;"::"r"((uint64_t)k_cr3));// - 0xffffffff80000000):);
}
uint64_t allocate_page(){
	freelist* temp = head;
	if(temp == NULL){
		kprintf("Trouble Land - Out of memory\n");
        while(1);
	}
	temp->free=0;
    temp->ref_count = 1;
	head = head->next;
	return temp->address;
}
uint64_t kmalloc(int size){
	int no_pages = (size/pageSize)+1;
	uint64_t add = allocate_page_for_process();
	no_pages--;
	if(no_pages>0){
		for(int i=0;i<no_pages;i++){
			allocate_page_for_process();
		}
	}
	return add;
}

uint64_t* getPTE(uint64_t v){
        uint64_t* p4 = (uint64_t*)(r->pml4e + 0xffffffff80000000);
        int id4 = (v >> 39 ) & 0x1FF;
    	uint64_t* p3 = (uint64_t*)((p4[id4]&0xFFFFFFFFFFFFF000) + 0xffffffff80000000);
    	int id3 = (v >> 30 ) & 0x1FF;
    	uint64_t* p2 = (uint64_t*)((p3[id3]&0xFFFFFFFFFFFFF000) + 0xffffffff80000000);
    	int id2 = (v >> 21 ) & 0x1FF;
        uint64_t* p1 = (uint64_t*)((p2[id2]&0xFFFFFFFFFFFFF000) + 0xffffffff80000000);
        return p1;
}

void map(uint64_t vaddr_s, uint64_t phy){
	int id1 = (vaddr_s >> 39 ) & 0x1FF;
	if(!(pml4e[id1] & 1)){
		uint64_t* p3 = (uint64_t *)allocate_page();
		
		pml4e[id1] = ((uint64_t)p3 & 0xFFFFFFFFFFFFF000) | 3;

		p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);

		int id2 = (vaddr_s >> 30 ) & 0x1FF;
		uint64_t* p2 = (uint64_t *)allocate_page();
		p3[id2] = ((uint64_t)p2 & 0xFFFFFFFFFFFFF000) | 3;

		p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);

		int id3 = (vaddr_s >> 21 ) & 0x1FF;
		uint64_t* p1 = (uint64_t *)allocate_page();
		p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 3;

		p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);

		int id4 = (vaddr_s >> 12 ) & 0x1FF;
		p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 3;
		return ;
	}
	else{
		uint64_t* p3 = (uint64_t *)(pml4e[id1] & 0xFFFFFFFFFFFFF000);
		int id2 =  (vaddr_s >> 30 ) & 0x1FF;
		p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);
		if( !(p3[id2] & 1)){	
			uint64_t* p2 =(uint64_t *) allocate_page();
			p3[id2] = ((uint64_t)p2 & 0xFFFFFFFFFFFFF000) | 3;

			p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);

			int id3 = (vaddr_s >> 21 ) & 0x1FF;
			uint64_t* p1 = (uint64_t *)allocate_page();
			p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 3;

			p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);

			int id4 = (vaddr_s >> 12 ) & 0x1FF;
			p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 3;
			return;
		}
		else{
			uint64_t* p2 = (uint64_t *)(p3[id2] &0xFFFFFFFFFFFFF000);
			int id3 =  (vaddr_s >> 21) & 0x1FF;
			p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);

			if( !(p2[id3] & 1)){
				uint64_t* p1 = (uint64_t *)allocate_page();
				p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 3;

				p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);

				int id4 = (vaddr_s >> 12 ) & 0x1FF;
				p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 3;
				return;
			}
			else{	
				uint64_t* p1 = (uint64_t *)(p2[id3] &0xFFFFFFFFFFFFF000);	
				int id4 = (vaddr_s >> 12 ) & 0x1FF;

				p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);

				p1[id4] = ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 3;
				return;
			}
		}
	}
}

uint64_t allocate_page_for_process(){
	uint64_t ax = (uint64_t )allocate_page();	
	viraddr = 0xffffffff80000000 + ax;
	map(viraddr,ax);
	return viraddr;
}
uint64_t getPhysical(uint64_t vr){
	return (uint64_t) *((uint64_t *)(*(uint64_t *)( *((uint64_t *)(pml4e[(vr>>39) & 0x1FF])+ ((vr>>30)&0x1FF)) + ((vr>>21)&0x1FF)) + ((vr>>12)&0x1FF)));
}

void init_pages_for_process(uint64_t vaddr_s, uint64_t phy, uint64_t* pml4){
	pml4[511] = (pml4e[511] & 0xFFFFFFFFFFFFF000) | 7;
	int id1 = (vaddr_s >> 39 ) & 0x1FF;
	if(!(pml4[id1] & 1)){
		uint64_t* p3 = (uint64_t *)allocate_page();
		pml4[id1] = ((uint64_t)p3 & 0xFFFFFFFFFFFFF000) | 7;
		p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);

        memset(p3,0,pageSize);

        int id2 = (vaddr_s >> 30 ) & 0x1FF;
		uint64_t* p2 = (uint64_t *)allocate_page();
		p3[id2] = ((uint64_t)p2 & 0xFFFFFFFFFFFFF000) | 7;

		p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);
        memset(p2,0,pageSize);
        int id3 = (vaddr_s >> 21 ) & 0x1FF;
		uint64_t* p1 = (uint64_t *)allocate_page();
		p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 7;

		p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);
        memset(p1,0,pageSize);
        int id4 = (vaddr_s >> 12 ) & 0x1FF;
		p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 7;
		return ;
	}

	else{
		uint64_t* p3 = (uint64_t *)(pml4[id1] & 0xFFFFFFFFFFFFF000);
		int id2 =  (vaddr_s >> 30 ) & 0x1FF;
		p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);
		if( !(p3[id2] & 1)){
			uint64_t* p2 =(uint64_t *) allocate_page();
			p3[id2] = ((uint64_t)p2 & 0xFFFFFFFFFFFFF000) | 7;

			p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);
            memset(p2,0,pageSize);
            int id3 = (vaddr_s >> 21 ) & 0x1FF;
			uint64_t* p1 = (uint64_t *)allocate_page();
			p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 7;

			p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);
            memset(p1,0,pageSize);

            int id4 = (vaddr_s >> 12 ) & 0x1FF;
			p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 7;
			return;
		}
		else{
			uint64_t* p2 = (uint64_t *)(p3[id2] &0xFFFFFFFFFFFFF000);
			int id3 =  (vaddr_s >> 21) & 0x1FF;
			p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);
			if( !(p2[id3] & 1)){
				uint64_t* p1 = (uint64_t *)allocate_page();

				p2[id3] = ((uint64_t)p1 & 0xFFFFFFFFFFFFF000) | 7;

				p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);
                memset(p1,0,pageSize);
                int id4 = (vaddr_s >> 12 ) & 0x1FF;
				p1[id4] =  ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 7;

				return;
			}
			else{
				uint64_t* p1 = (uint64_t *)(p2[id3] &0xFFFFFFFFFFFFF000);
				int id4 = (vaddr_s >> 12 ) & 0x1FF;

				p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);

				p1[id4] = ((uint64_t)phy & 0xFFFFFFFFFFFFF000) | 7;
				return;
			}
		}
	}
}

void setupPageTables(uint64_t physbase, uint64_t physfree){

	viraddr = (uint64_t)0xffffffff80000000;//(uint64_t)&kernmem;

	pml4_idx = (viraddr >> 39 ) & 0x1FF;
	pdpt_idx = (viraddr >> 30 ) & 0x1FF;
	pd_idx = (viraddr >> 21 ) & 0x1FF;
	pt_idx = (viraddr >> 12 ) & 0x1FF;

	pml4e = (uint64_t *)allocate_page();
	pdpte = (uint64_t *)allocate_page();
	pde = (uint64_t *)allocate_page();
	pte = (uint64_t *)allocate_page();
	pml4e[pml4_idx] = ((uint64_t)pdpte & 0xFFFFFFFFFFFFF000) | 3;
	pdpte[pdpt_idx] = ((uint64_t)pde & 0xFFFFFFFFFFFFF000) | 3;
	pde[pd_idx] = ((uint64_t)pte & (0xFFFFFFFFFFFFF000)) | 3;
	for(int j=0;physbase<physfree;j++,viraddr+=pageSize,physbase+=pageSize){
		pml4_idx = (viraddr >> 39 ) & 0x1FF;
		pdpt_idx = (viraddr >> 30 ) & 0x1FF;
		pd_idx = (viraddr >> 21 ) & 0x1FF;
		pt_idx = (viraddr >> 12 ) & 0x1FF;   

		if(pt_idx!=0){
			pte[pt_idx] = (physbase & 0xFFFFFFFFFFFFF000) | 3;
		}
		else{
			pte = ((uint64_t*)allocate_page());
			pde[pd_idx] = ((uint64_t)pte & (0xFFFFFFFFFFFFF000)) | 3;
			pte[pt_idx] = (physbase & 0xFFFFFFFFFFFFF000) | 3;		
		}


		//		map(viraddr,physbase);
	}
	k_cr3 = (uint64_t)pml4e;
	// Temp Fix, we need a better approach

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 0x1FF;
	pte[pt_idx] = ( (uint64_t)pml4e & 0xFFFFFFFFFFFFF000) | 3;
	pml4e = (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 0x1FF;
	pte[pt_idx] = ( (uint64_t)pdpte & 0xFFFFFFFFFFFFF000) | 3;
	pdpte = (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 0x1FF;
	pte[pt_idx] = ( (uint64_t)pde & 0xFFFFFFFFFFFFF000) | 3;
	pde= (uint64_t*)viraddr;

	viraddr+=pageSize;
	pt_idx = (viraddr >> 12 ) & 0x1FF;
	pte[pt_idx] = ( (uint64_t)pte & 0xFFFFFFFFFFFFF000) | 3;
	pte = (uint64_t*)viraddr;

	__asm__ volatile("movq %0,%%cr3"::"r"(k_cr3));	
//	k_cr3 = (uint64_t)pml4e;
}
void copytables(task_struct* p, task_struct* c){
	uint64_t* p4 = (uint64_t *)(p->pml4e + 0xffffffff80000000);
	uint64_t* c4 =(uint64_t *) (c->pml4e + 0xffffffff80000000);
	c4[511] = p4[511];
	for(int i =0;i<511;i++){
		if(p4[i] & 1){
			uint64_t* c3 = (uint64_t *)allocate_page_for_process();
            memset(c3,0,pageSize);
			c4[i] = ((uint64_t)((uint64_t)c3 -((uint64_t)0xffffffff80000000)) & 0xFFFFFFFFFFFFF000) | 7;
			uint64_t* p3 = (uint64_t *)(p4[i] & 0xFFFFFFFFFFFFF000);
			p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);
			for(int j=0;j<512;j++){
				if(p3[j] & 1){
					uint64_t* c2 = (uint64_t *)allocate_page_for_process();
                    memset(c2,0,pageSize);
                    c3[j] = ((uint64_t)((uint64_t)c2 -((uint64_t)0xffffffff80000000)) & 0xFFFFFFFFFFFFF000) | 7;
					uint64_t* p2 = (uint64_t *)(p3[j] & 0xFFFFFFFFFFFFF000);
					p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);
					for(int k=0;k<512;k++){
						if(p2[k] & 1){
							uint64_t* c1 = (uint64_t *)allocate_page_for_process();
                            memset(c1,0,pageSize);
                            c2[k] = ((uint64_t)((uint64_t)c1 -((uint64_t)0xffffffff80000000)) & 0xFFFFFFFFFFFFF000) | 7;
							uint64_t* p1 = (uint64_t *)(p2[k] & 0xFFFFFFFFFFFFF000);
							p1 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p1);
							for(int l=0;l<512;l++){
								if(p1[l]&1){
                                    increfcount(((uint64_t)p1[l] & 0xFFFFFFFFFFFFF000));
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
    uint64_t* p4 = (uint64_t *)(pm4 + 0xffffffff80000000);
    for(int i=0;i<511;i++){
        if(p4[i]&1){
            uint64_t* p3 = (uint64_t *)(p4[i] & 0xFFFFFFFFFFFFF000);
            p3 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p3);
            for (int j = 0; j < 512; ++j) {
                if(p3[j] & 1){
                    uint64_t* p2 = (uint64_t *)(p3[j] & 0xFFFFFFFFFFFFF000);
                    p2 = (uint64_t *)((uint64_t)0xffffffff80000000 + (uint64_t)p2);
                    for (int k = 0; k < 512 ; ++k) {
                        if(p2[k]&1) {
                            uint64_t *p1 = (uint64_t *)(p2[k] & 0xFFFFFFFFFFFFF000);
                            p1 = (uint64_t*)((uint64_t) 0xffffffff80000000 + (uint64_t) p1);
                            for (int l = 0; l < 512; ++l) {
                                if (p1[l] & 1) {
                                    //  memset((uint64_t*)((p1[l] & 0xFFFFFFFFFFFFF000)+0xffffffff80000000),0,pageSize);
                                    free(((uint64_t) p1[l] & 0xFFFFFFFFFFFFF000));
                                }
                                p1[l] = 0;
                            }
                            free(p2[k] & 0xFFFFFFFFFFFFF000);
                        }
                        p2[k]=0;

                    }
                    free(p3[j] & 0xFFFFFFFFFFFFF000);
                }
                p3[j]=0;
            }
            free(p4[i] & 0xFFFFFFFFFFFFF000);
        }
        p4[i]=0;
    }
  //  free(pm4);
}
