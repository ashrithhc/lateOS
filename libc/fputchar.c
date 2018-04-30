#include <syscall.h>
#include <stdio.h>

int fputchar(int c,int fd)
{
        int size = 1;
        long retVal;
	    __asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(1), "r"((long)(fd)), "r"((long)(&c)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
	    return (int)(retVal);
	return 0;
}
