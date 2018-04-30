#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
int puts(const char *s)
{
	int len=strlen(s);
	// _syscall3(int,write,int,stdout,char*,s,int,len);
	long retVal;
	__asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (1),"r" ((long)(stdout)),"r" ((long)(s)), "r" ((long)(len)) : "rax","memory","rbx","rcx","rdx");
	return (int)(retVal);
	// return 0;
}
