#include <sys/defs.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

int kill_call(pid_t pid)
{
    _syscall1(int, kill, (int)pid);
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
