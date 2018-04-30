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

#endif
