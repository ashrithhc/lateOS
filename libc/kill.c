//
// Created by Naga Srinikhil Reddy on 12/4/17.
//

#include <sys/defs.h>
#include <syscall.h>
#include <unistd.h>

int kill_call(pid_t pid)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (62), "g" ((long)(pid)) : "rax", "rbx");
    return (int)(retVal);
}

void kill(pid_t pid)
{
    kill_call(pid);
}