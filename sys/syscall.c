#include <sys/syscall.h>

void syscall1_int_int(int constant, int arg){
	#define _syscall1(type,name,arg1)
	long __res;
	__asm__ volatile (  "movq %1, %%rax ; movq %2, %%rbx; int $0x80; movq %%rax, %0;"
	                  : "=m" (__res)
	                  : "g" (constant),"g" ((long)(arg)) \
			  : "rax","rbx" );\
	 return (int) (__res);
}