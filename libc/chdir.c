#include <syscall.h>
#include <sys/defs.h>
#include <string.h>

int chdirsyscall(char inp[40])
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (80), "g" ((long)(a)) : "rax", "rbx");
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
	// _syscall1(int,chdir, a);
    return chdirsyscall(a);
}
int cwd_call(char* buf,size_t size){
    _syscall2(int,getCurrentDirectory, char*, buf,int,size);
}
char *getCurrentDirectory(char *buf, size_t size){
    cwd_call(buf,size);
    return buf;
}