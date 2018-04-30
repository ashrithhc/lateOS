#include <syscall.h>

void exitcall(int inp)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (60), "g" ((long)(inp)) : "rax", "rbx");
    return (void)(retVal);
}

void exit(int status)
{
	// _syscall1(void, exit, status);
	exitcall(status);
}
