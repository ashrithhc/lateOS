#include<syscall.h>
#include<sys/defs.h>
off_t lseek(int fd, off_t offset, int p)
{
	// _syscall3(off_t,lseek,int,fd,off_t,offset,int,p);
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(8), "r"((long)(fd)), "r"((long)(offset)), "r"((long)(p)) : "rax", "memory", "rbx", "rcx", "rdx");
	return (off_t)(retVal);
	return 0;
}
