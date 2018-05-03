#include <syscall.h>
#include <sys/defs.h>
#include <string.h>
#include <stdio.h>

int chdirsyscall(char inp[40])
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (80), "g" ((long)(inp)) : "rax", "rbx");
    return (int)(retVal);
}

int chdir(const char *path)
{
    char a[40];
    strcpy(a,(char *)path);
    int l = strlen(a);
    if(path[l-1] != '/'){
        strcat(a,"/");
    }
    return chdirsyscall(a);
}
int cwd_call(char* buf,size_t size){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(79), "g"((long)(buf)), "g"((long)(size)) : "rax", "rbx", "rcx");
    return (int)(retVal);
}
char *getCurrentDirectory(char *buf, size_t size){
    cwd_call(buf,size);
    return buf;
}

int closecall(int inp)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (3), "g" ((long)(inp)) : "rax", "rbx");
    return (int)(retVal);
}

int fclose(int fd)
{
    return closecall(fd);
}
