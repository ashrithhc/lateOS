#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
typedef struct rusage r;
pid_t wait(int* status){
	_syscall4(pid_t,wait4,pid_t,-1,int*,status,int,0,rusage,NULL);
	long retVal;
	__asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; movq %5, %%rsi; int $0x80; movq %%rax, %0" : "=m" (retVal) : "g" (61),"g" ((long)(-1)),"g" ((long)(status)), "g" ((long)(0)), "g" ((long)(NULL)) : "rax", "rbx", "rcx", "rdx", "rsi");
	return (pid_t)(retVal);
}
pid_t waitpid(pid_t pid,pid_t *status){
	// _syscall2(pid_t,waitid,int,pid,int*,status);
	long retVal;
	__asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(247), "g"((long)(pid)), "g"((long)(status)) : "rax", "rbx", "rcx");
	return (pid_t)(retVal);
}
