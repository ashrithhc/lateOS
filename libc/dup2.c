#include "../include/syscall.h"

int dup2(int fd1, int fd2)
{
	// _syscall2(int, dup2, int, fd1, int, fd2);
	long retVal;
    __asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(33), "g"((long)(fd1)), "g"((long)(fd2)) : "rax", "rbx", "rcx");
    return (int)(retVal);
}

