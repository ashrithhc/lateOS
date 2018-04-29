#include <syscall.h>
#include <sys/defs.h>
#include <string.h>

int chdir(const char *path)
{
    char a[40];
    strcpy(a,(char *)path);
    int l = strlen(a);
    if(path[l-1] != '/'){
        strcat(a,"/");
    }
	_syscall1(int,chdir, a);
}
int cwd_call(char* buf,size_t size){
    _syscall2(int,getCurrentDirectory, char*, buf,int,size);
}
char *getCurrentDirectory(char *buf, size_t size){
    cwd_call(buf,size);
    return buf;
}