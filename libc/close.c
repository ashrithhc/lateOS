#include "../include/syscall.h"
#include <stdio.h>

int closecall(int inp)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (3), "g" ((long)(pid)) : "rax", "rbx");
    return (int)(retVal);
}

int close(int fd)
{
	_syscall1(int, close, fd);
}

int fclose(int fd)
{
	return closecall(fd);
}
