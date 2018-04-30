#include <sys/defs.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

int kill_call(pid_t pid)
{
    // _syscall1(int, kill, (int)pid);
    long __res;
    __asm__ volatile (  "movq %1, %%rax ; movq %2, %%rbx; int $0x80; movq %%rax, %0;"
                  : "=m" (__res)
                  : "g" (62),"g" ((long)(pid))
                  : "rax","rbx" );
    return (int) (__res);
}

int strtoInt(char* num){
    int dec = 0, i, len;
    len = strlen(num);
    for(i=0; i<len; i++){
        dec = dec * 10 + ( num[i] - '0' );
    }
    return dec;
}

int main(int argc, char* argv[], char* envp[])
{
   if(argc<3){
       puts("Must use : kill -9 PID\n");
   }
    int pid = strtoInt(argv[2]);
    if(pid < 0){
        puts("Incorrect PID\n");
    }
    kill_call((pid_t)pid);
}
