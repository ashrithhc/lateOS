#include "../include/syscall.h"

void ps_call()
{
	// _syscall(void, ps);
	long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(299) : );
	// return (type) (__res);
}

void ps()
{
	ps_call();
}
