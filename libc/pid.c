#include <stdio.h>
#include <sys/defs.h>
#include <syscall.h>

pid_t getpid_call(){
    // _syscall(pid_t, getTaskPID);
    long retVal;
	__asm__ volatile ("int $0x80;" : "=a"(retVal) : "a"(39) : );
	return (pid_t)retVal;
}

pid_t getppid_call(){
    // _syscall(pid_t, getTaskPPID);
    long retVal;
	__asm__ volatile ("int $0x80;" : "=a"(retVal) : "a"(110) : );
	return (pid_t)retVal;
}
