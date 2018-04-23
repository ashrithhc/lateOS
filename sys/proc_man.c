#include<sys/process.h>
#include<sys/gdt.h>
#include<sys/mem.h>
#include<sys/kprintf.h>
int np = 0;
void init_proc(){
	for(int i=0;i<MAX;i++){
		taskQueue[i].state = READY;
		taskQueue[i].fd_c = 0;
		taskQueue[i].time = 0;
	}
}
void schedule(){
		taskStruct *last = r;
		int i = (last->pid+1)%MAX;
			/*
			Code to check for the next running process to schedule from the process array in a circular manner
		*/
		while(i!= last->pid){         
			if(taskQueue[i].state == RUNNING){
				break;
			}
			i = (i+1)%MAX;
		}
        r = (taskStruct *)&taskQueue[i];
//		kprintf("switch enter");
		set_tss_rsp((uint64_t*)&(currentTask->kstack[511]));
//		__asm__ volatile("pushq %%rax ;pushq %%rcx ;pushq %%rdx ;pushq %%rsi ;pushq %%rdi ;pushq %%r8 ;pushq %%r9 ;pushq %%r10;pushq %%r11;":::);
		__asm__ volatile("movq %%rsp, %0;":"=g"((last->regs.rsp))::"memory");
		__asm__ volatile("movq %0,%%rsp;"::"r"(currentTask->regs.rsp):"memory");
		__asm__ volatile("movq %0,%%cr3;"::"r"(currentTask->pml4e):);
//		kprintf("\nSwitching to task %d, at %p \n",currentTask->pid,currentTask->regs.rip);
//		__asm__ volatile("popq %%r11; popq %%r10;popq %%r9 ;popq %%r8 ;popq %%rdi ;popq %%rsi ;popq %%rdx ;popq %%rcx ;popq %%rax":::);	
//		__asm__ volatile("movq %%rsp,%0"::"r"(currentTask->regs.rsp):"memory");
//		__asm__ volatile("pushq %0;"::"r"(currentTask->regs.rip):"memory");
//		__asm__ volatile("retq");
		__asm__ __volatile__ ("movq $1f, %0":"=g"(last->regs.rip)::);
		__asm__ __volatile__ ("pushq %0;"::"r"(currentTask->regs.rip):);
		__asm__ volatile("retq");
		__asm__ __volatile__ ("1:\t");	

}



