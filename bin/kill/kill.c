#include <sys/defs.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <string.h>

int kill_syscall(pid_t pid)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (62), "g" ((long)(pid)) : "rax", "rbx");
    return (int)(retVal);
}

int validateARGS(int argc, char *pName){
    if (argc < 3){
       puts("Must use : kill -9 PID\n");
       return -1;
    }
    int pid = strtoInt(pName);
    if(pid < 0){
        puts("Incorrect PID\n");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[], char* envp[])
{
    if (validateARGS(argc, argv[2]) == -1) return -1;
    kill_syscall((pid_t) strtoInt(argv[2]));
    return 0;
}
