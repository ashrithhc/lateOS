#include "../include/syscall.h"
#include <stdio.h>
int clrscr_call()
{
    // _syscall(int, clrscr);
    long retVal;
	__asm__ volatile ("int $0x80;" : "=a"(retVal) : "a"(7) : );
}
void clrscr()
{
    clrscr_call();
    return;
}