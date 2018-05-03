/*#include "../include/syscall.h"
#include <sys/defs.h>
#include <stdio.h>
#include "getchar.c"
int readcall1(char* s){
    // _syscall3(int, read, int, stdin,char* ,s, int, 4096);
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(s)), "r"((long)(4096)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
}
char* gets(char *string)
{
       char *s=string;
     readcall1(s);
      return string;
}


*/