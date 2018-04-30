#include "../include/syscall.h"

int pipe(int fd[2])
{
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (22), "g" ((long)(&fd[0])) : "rax", "rbx");
    return (int)(retVal);
}
