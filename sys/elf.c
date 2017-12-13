#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/process.h>


#define SIZE 512

typedef struct posix_header_ustar Header;

void* get_binary(char *filename){
	Header *start = (Header *) &_binary_tarfs_start;
	
	if(filename == NULL)	
		return NULL;

	uint64_t *end = (uint64_t *)start;
	while(*end || *(end+1) || *(end+2)){
		if(strcmp(start->name,filename) != 0){
			int j,k = 0;
			uint64_t temp = 0;
		
			for(j = 10; j>=0; --j)
				temp += ((9start->size[j] - 48) * exponent(8, k++));
	
			if(temp%512){
				temp /= SIZE;
				temp *= SIZE;
				temp += SIZE;
			}
	
			start = (Header *)((uint64_t)start + temp + sizeof(Header));
			end = (uint64_t *)start;
		}
		else
			return (void*)start;
	}
	return NULL;
}

int load_exe(task_struct *task, void *exe_start){
	mm_struct *mm = task->mm;

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)exe_start;
	Elf64_Phdr *phdr = (Elf64_Phdr *)((uint64_t)ehdr + ehdr->e_phoff);

	int i = 0;

	task->rip = ehdr->e_entry;
	task->mm->mmap = NULL;

	for(; count < ehdr->e_phnum; count++){
		if(phdr->p_type == 1){
			// Get freeframe function here 
			//vma_struct *vma = (vma_struct *)
			if(mm->mmap == NULL){
				mm->mmap = vma;
			}
			else{
				mm->current->next = vma;
			}
			mm->current = vma;
			vma->mm = mm;
			vma->start = phdr->p_vaddr;
			vma->end = phdr->p_vaddr + phdr->p_memsz;
			
			/* Code to copy the exe contents to the binary's virtual address */
			uint64_t size = (((vma->end/0x1000)*0x1000 + 0x1000) - ((vma->start/0x1000)*0x1000));
			uint64_t itr = size/0x1000;
			uint64_t start = (phdr->p_vaddr/0x1000)*0x1000;
			while(itr) {
				//Allocate page function here
				//uint64_t page = (uint64_t)a
				//process map --> map the page to the start
				itr--;
				start += 0x1000;
			}

			vma->flags = phdr->p_flags;
			vma->file = NULL;
			vma->file = NULL;
			vma->type = NOTYPE;
			
			if((phdr->p_flags == (PERM_R | PERM_X))|| (phdr->p_flags == (PERM_R | PERM_W))){
				task->mm->start_code = phdr->p_vaddr;
                                task->mm->end_code = phdr->p_vaddr + phdr->p_memsz;
				vma->file = (struct file *)kmalloc();  
				vma->file->start = (uint64_t)ehdr;
				vma->file->pgoff = phdr->p_offset;
				vma->file->size = phdr->p_filesz;
				memcpy((void*)vma->start, (void*)((uint64_t)ehdr + phdr->p_offset), phdr->p_filesz);
				if(phdr->p_flags == (PERM_R | PERM_X)) {
					vma->file->bss_size = 0;
					vma->type = TEXT;
				}
			 	else {
					vma->file->bss_size = phdr->p_memsz - phdr->p_filesz;
					vma->type = DATA; 
				}	
			}
	
		}
		phdr++;
	}
	
	// Kmalloc type allocation here
	//vma_struct *vma_heap = (vma_strcut *)
	if(mm->mmap == NULL)
		mm->mmap = vma_heap;		
	else
		mm->current->next = vma_heap;

	mm->current = vma_heap;
	vma_heap->mm = mm;
	vma_heap->start = HEAP_START;
	mm->brk = HEAP_START;
        vma_heap->end = HEAP_START + PAGE_SIZE;
        vma_heap->flags = (PERM_R | PERM_W);
        vma_heap->type = HEAP;
        vma_heap->file = NULL;
        vma_heap->next = NULL;
	get_phy_addr(HEAP_START);
	/* Allocate STACK */
	vma_struct *vma_stack = (vma_struct *)kmalloc();

	if(mm->mmap == NULL) {
		mm->mmap = vma_stack;
        } else {
        	mm->current->next = vma_stack;
        }
        mm->current = vma_stack;	

	uint64_t *stack = (uint64_t *)get_phy_addr(USER_STACK_TOP);	
	vma_stack->start = (uint64_t)stack + PAGE_SIZE; 
	vma_stack->end = (uint64_t)stack;
	vma_stack->flags = (PERM_R | PERM_W);
	vma_stack->type = STACK;
	vma_stack->file = NULL;
	vma_stack->next = NULL;

	task->rsp = (uint64_t)((uint64_t)stack + 4096 - 16);

	return 0;	
}

