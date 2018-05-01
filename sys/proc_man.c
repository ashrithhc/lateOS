#include <sys/process.h>
#include <sys/gdt.h>
#include <sys/mem.h>
#include <sys/kprintf.h>
int np = 0;
void init_proc(){
	for(int i=0;i<MAX;i++){
		taskQueue[i].state = READY;
		taskQueue[i].fd_c = 0;
		taskQueue[i].time = 0;
	}
}

void loadPML4E(uint64_t newPML4){
	__asm__ __volatile__("movq %0, %%cr3;" : : "r"(newPML4) : );
}

void switchRSP(taskStruct *finalTask){
	__asm__ __volatile__("movq %%rsp, %0;" : "=g"((&(finalTask->regs))->rsp) : : "memory");
	__asm__ __volatile__("movq %0, %%rsp;" : : "r"((&(currentTask->regs))->rsp) : "memory");
}

void switchRIP(taskStruct *finalTask){
	__asm__ __volatile__ ("movq $1f, %0" : "=g"((&(finalTask->regs))->rip) : : );
	__asm__ __volatile__ ("pushq %0;" : : "r"((&(currentTask->regs))->rip) : );
}

void schedule(){
	taskStruct *finalTask = currentTask;
	int nextPID = (finalTask->pid+1) % MAX;
	while(nextPID != finalTask->pid){         
		if(taskQueue[nextPID].state == RUNNING){
			break;
		}
		nextPID = (nextPID+1)%MAX;
	}
    currentTask = (taskStruct *)&taskQueue[nextPID];
	set_tss_rsp((uint64_t*)&(currentTask->kstack[511]));
	// __asm__ __volatile__("movq %%rsp, %0;" : "=g"((&(finalTask->regs))->rsp) : : "memory");
	// __asm__ __volatile__("movq %0, %%rsp;" : : "r"((&(currentTask->regs))->rsp) : "memory");
	switchRSP(finalTask);
	loadPML4E(currentTask->pml4e);
	__asm__ __volatile__ ("movq $1f, %0" : "=g"((&(finalTask->regs))->rip) : : );
	__asm__ __volatile__ ("pushq %0;" : : "r"((&(currentTask->regs))->rip) : );
	__asm__ __volatile__("retq");
	__asm__ __volatile__ ("1:\t");
}



