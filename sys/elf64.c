#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/task.h>
#include <sys/pagetable.h>
#include <sys/kprintf.h>
#include <sys/freelist.h>
#include <sys/defs.h>
#include <sys/string.h>

void idMap(uint64_t virtual, uint64_t physical){
	PML4E *pml4base = (PML4E *)currentCR3();
	PDPE *pdpebase = (PDPE *)((uint64_t)((pml4base + ((virtual >> (12+9+9+9)) & 511))->pageDirectoryPointer)<<12);
	PDE *pdebase = (PDE *)((uint64_t)((pdpebase + ((virtual >> (12+9+9)) & 511))->pageDirectoryBase)<<12);
	PTE *ptebase = (PTE *)((uint64_t)((pdebase + ((virtual >> (12+9)) & 511))->pageTableBase)<<12);

	(ptebase + ((virtual >> 12) & 511))->physicalPageBase = physical;
}

void *readELF(char *filename){
	tarfsHeader *start = (tarfsHeader *)&_binary_tarfs_start;
	if (filename == NULL) return NULL;
	
	//char* tempbuf = &_binary_tarfs_start;
	uint64_t *end = (uint64_t *)start;
	while(end <= (uint64_t *)(&_binary_tarfs_end)){
//	while (*end || *(end+1) || *(end+2)){
		if (strcmp(start->name, filename) != 0){
			kprintf(start->name);
			int j, k=0;
			uint64_t temp = 0;
			for (j=10; j>=0; j--){
				temp += ((start->size[j] - 48) * power(8, k++));
			}
				if (temp%512){
					temp /= 512;
					temp *= 512;
					temp += 512;
				}
			
			start = (tarfsHeader *)((uint64_t)start + temp + sizeof(tarfsHeader));
			end = (uint64_t *)start ;
		}
		else return (void *)start;
	}
	return NULL;
}

int loadEXE(taskStruct *task, void *binary){
	mmStruct *mm = task->mm;
	Elf64_Ehdr *eHeader = (Elf64_Ehdr *)binary;
	Elf64_Phdr *pHeader = (Elf64_Phdr *)((uint64_t)eHeader + eHeader->e_phoff);

	task->rip = eHeader->e_entry;
	task->mm->mmap = NULL;

	int i;
	for (i=0; i<eHeader->e_phnum; i++){
		if (pHeader->p_type == 1){
			vmaStruct *vma = (vmaStruct *)getFreeFrame();

			if (mm->mmap == NULL) mm->mmap = vma;
			else mm->current->next = vma;
			mm->current = vma;
			vma->mm = mm;
			vma->start = pHeader->p_vaddr;
			vma->end = pHeader->p_vaddr + pHeader->p_memsz;

			uint64_t size = (((vma->end/0x1000)*0x1000 + 0x1000) - ((vma->start/0x1000)*0x1000 + 0x1000));
			uint64_t numPages = size/0x1000;
			uint64_t start = (pHeader->p_vaddr/0x1000)*0x1000;

			while (numPages){
				uint64_t pAddr = (uint64_t)getFreeFrame();
				idMap(start, pAddr); start+=0x1000;
				numPages--;
			}
			vma->flags = pHeader->p_flags;
			vma->file = NULL; vma->next = NULL; vma->type = NONE;

			if ((pHeader->p_flags == (0x4|0x2)) || (pHeader->p_flags == (0x4|0x2))){
				task->mm->csStart = pHeader->p_vaddr;
				task->mm->csEnd = pHeader->p_vaddr + pHeader->p_memsz;
				vma->file->start = (uint64_t)eHeader;
				vma->file->offset = pHeader->p_offset;
				vma->file->size = pHeader->p_filesz;

				memcpy((void *)vma->start, (void *)((uint64_t)eHeader + pHeader->p_offset), pHeader->p_filesz);
				if (pHeader->p_flags == (0x4|0x2)){
					vma->file->bss = 0;
					vma->type = TEXT;
				}
				else {
					vma->file->bss = pHeader->p_memsz - pHeader->p_filesz;
					vma->type = DATA;
				}
			}
		}
		pHeader++;
	}

	vmaStruct *vmaHeap = (vmaStruct *)getFreeFrame();
	if (mm->mmap == NULL) mm->mmap = vmaHeap;
	else mm->current->next = vmaHeap;

	mm->current = vmaHeap; vmaHeap->mm = mm;
	vmaHeap->start = HEAPTOP;
	mm->brk = HEAPTOP;
	vmaHeap->end = HEAPTOP + 0x1000;
	vmaHeap->flags = (0x4|0x2); vmaHeap->type = HEAP; vmaHeap->file = NULL; vmaHeap->next = NULL;
	uint64_t pageForHeap = getFreeFrame();
	idMap(HEAPTOP, pageForHeap);

	vmaStruct *vmaStack = (vmaStruct *)getFreeFrame();
	if (mm->mmap == NULL) mm->mmap = vmaStack;
	else mm->current->next = vmaStack;

	mm->current = vmaStack;
	uint64_t pageForStack = getFreeFrame();
	idMap(STACKTOP, pageForStack);
	uint64_t *stack = (uint64_t *)STACKTOP;
	vmaStack->start = (uint64_t)stack + 0x1000;
	vmaStack->end = (uint64_t)stack;
	vmaStack->flags = (0x4|0x2);
	vmaStack->type = STACK; vmaStack->file = NULL; vmaStack->next = NULL;

	task->rsp = (uint64_t)((uint64_t)stack + 0x1000 - 16);
	return 0;
}
