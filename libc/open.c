#include <syscall.h>
#include <sys/defs.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

static DIR dir[100];
static struct dirent d[100];
int open(const char *path,int flags);
//FILE filepointer;
int fopen(char *name, char *mode)
{

    int flag;
    if(*mode == 'r'){
        flag = O_RDONLY;
    }   
    int fd = open(name,flag);
    return fd;   
}
int open(const char *path,int flags)
{
    long retVal;
    __asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(2), "g"((long)(path)), "g"((long)(flags)) : "rax", "rbx", "rcx");
    return (int)(retVal);
	return 0;
}
int close(int fd){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (3), "g" ((long)(fd)) : "rax", "rbx");
    return (int)(retVal);
}
int open_dir(const char *path)
{
    // _syscall1(int, opendir, path);
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (16), "g" ((long)(path)) : "rax", "rbx");
    return (int)(retVal);
}

DIR *opendir(const char *name){
    int l = open_dir(name);
    if(l == -1){
        dir[0].fd = l;
        return &dir[0];
    }
    dir[l].fd = l;
    strcpy(dir[l].d_ent.d_name,(char *)name);
    return &dir[l];
}

int direccall(int fd,char* buff,int size){
    _syscall3(int,getdents,int, fd,char*, buff,int,size);
}
struct dirent *readdir(DIR *dirp){
    int i = direccall(dirp->fd,d[dirp->fd].d_name,4096);
    if(i == 0){
        return NULL;
    }
    return &d[dirp->fd];
}
int closedir(DIR *dirp){
    return close(dirp->fd);
}