#include "../include/syscall.h"
#include "../include/sys/defs.h"
int readCall();
int c;
int getchar()
{
    readCall();
    return (int)c;
}	
int readCall(){

	// _syscall3(int, read, int, stdin,int*,&c, int, 1);
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(&c)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 0;
}
