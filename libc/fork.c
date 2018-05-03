#include <stdio.h>
#include <syscall.h>
#include <sys/defs.h>
/*pid_t forkcall()
{
	// _syscall(pid_t, fork);
	long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(57) : );
	return (pid_t)retVal;
}

pid_t fork()
{
	pid_t l  = forkcall();
//	putchar((int)l);
	return l;
}*/
