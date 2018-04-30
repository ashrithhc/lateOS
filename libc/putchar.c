#include <syscall.h>
#include <sys/defs.h>
int writecall(int* c){
	// _syscall3(int,write,int,stdout,int*,c,int,1);
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (1),"r" ((long)(stdout)),"r" ((long)(c)), "r" ((long)(1)) : "rax","memory","rbx","rcx","rdx");
	return (int)(retVal);
	return 0;
}	
int putchar(int c)
{
return	writecall(&c);
}
