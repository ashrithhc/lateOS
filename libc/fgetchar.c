#include <syscall.h>
#include <stdio.h>

int readcall(int fd);
char c;
char fgetc(int fd)
{
	readcall(fd);
	//putchar(*`c);
	return c;
}
int readcall(int fd)
{
	//putchar(f->fd);
	// _syscall3(int, read, int, fd, char*, &c, int, 1);
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(&c)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 1;
}

