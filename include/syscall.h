#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "syscall_const.h"

#define _sys_return(type,res) \
do{ 				\
	return (type)(res); \
}while(0) 

#define _syscall(type,name) \
do{ \
long __res; \
__asm__ volatile (  "int $0x80;" \
                  : "=a" (__res) \
                  : "a"(__NR_##name) \
                  :); \
 return (type) (__res); \
}while(0)

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4)\
do{ \
long __res; \
__asm__ volatile (  "movq %1, %%rax ; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; movq %5, %%rsi; int $0x80; movq %%rax,%0" \
                  : "=m" (__res) \
                  : "g" (__NR_##name),"g" ((long)(arg1)),"g" ((long)(arg2)), "g" ((long)(arg3)), "g" ((long)(arg4)) \
                  : "rax","rbx","rcx","rdx","rsi"); \
 return (type) (__res); \
}while(0)

#endif
