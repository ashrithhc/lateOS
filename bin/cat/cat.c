#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

char buf[4096];

int validateARGS(int argc, char *filenmae){
    if(argc < 2){
        puts("Usage : cat <file nama>\n");
        return -1;
    }
    int file = fopen(filename, "r");
    if(file < 0){
        puts("File not found\n");
        return -1;
    }
    close(file);
    return 1;
}


int readStr_sys(int fd, char* buf, int size){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(buf)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
	return 0;
}

int main(int argc, char* argv[], char* envp[])
{
	int file;
    if (validateARGS(argc, argv[1]) == -1) return -1;
    
    file = fopen(argv[1], "r");
    readStr_sys(file, buf, 4096);
	puts(buf);
	close(file);
}

