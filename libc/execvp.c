#include "../include/syscall.h"
#include "putchar.c"
/*int execvp(const char *file, char *const argv[],char *const env[]){
//	  putchar(file[0]);
    // _syscall3(int,execve,char*, file,char*,argv,char*, env);
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(59), "r"((long)(file)), "r"((long)(argv)), "r"((long)(env)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 0;
}*/
