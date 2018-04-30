#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

char buf[4096];

int readstring(int fd,char* buf,int size){
	// _syscall3(int,read,int,fd,char*,buf,int,size);
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(buf)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 0;
}

int main(int argc, char* argv[], char* envp[])
{
	int fp ;
    if(argc <2){
        puts("-cat: No file name given\n");
        return -1;
    }
    fp=fopen(argv[1],"r");
    if(fp < 0){
        puts("cat: ");
        puts(argv[1]);
        puts(": No such file\n");
        return -1;
    }
//    puts("\n");
    readstring(fp,buf,4096);
	puts(buf);
	close(fp);
}

