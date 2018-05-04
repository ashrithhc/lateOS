#include <sys/defs.h>
#include <unistd.h>
#include <stdio.h>

unsigned int sleep_call(unsigned int seconds)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (35), "g" ((long)(seconds)) : "rax", "rbx");
    return (int)retVal;
}

unsigned int sleep(unsigned int seconds)
{
    return sleep_call(seconds);
}

void ps_call()
{
	long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(299) : );
}

void ps()
{
	ps_call();
}

pid_t getpid_call(){
    long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(39) : );
	return (pid_t)retVal;
}

pid_t getppid_call(){
    long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(110) : );
	return (pid_t)retVal;
}

int kill_call(pid_t pid)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (62), "g" ((long)(pid)) : "rax", "rbx");
    return (int)retVal;
}

void kill(pid_t pid)
{
    kill_call(pid);
}

pid_t forkcall()
{
	long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(57) : );
	return (pid_t)retVal;
}

pid_t fork()
{
	return forkcall();
}

void exitcall(int inp)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (60), "g" ((long)(inp)) : "rax", "rbx");
    return (void)retVal;
}

void exit(int status)
{
	exitcall(status);
}

int writecall(int* c){
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (1),"r" ((long)(stdout)),"r" ((long)(c)), "r" ((long)(1)) : "rax","memory","rbx","rcx","rdx");
	return (int)retVal;
}	

int putchar(int c)
{
	return writecall(&c);
}

int execvp(const char *file, char *const argv[],char *const env[]){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(59), "r"((long)(file)), "r"((long)(argv)), "r"((long)(env)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
}