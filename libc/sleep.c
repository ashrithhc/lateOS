/*//
// Created by Naga Srinikhil Reddy on 12/4/17.
//

#include <sys/defs.h>
#include <syscall.h>
#include <unistd.h>
unsigned int sleep_call(unsigned int seconds)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (35), "g" ((long)(seconds)) : "rax", "rbx");
    return (int)(retVal);
}
unsigned int sleep(unsigned int seconds)
{
    return sleep_call(seconds);
}*/