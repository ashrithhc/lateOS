#include <syscall.h>
#include <sys/defs.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

static DIR dir[100];
static struct dirent d[100];
int open(const char *path,int flags);

int chdirsyscall(char inp[40])
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (80), "g" ((long)(inp)) : "rax", "rbx");
    return (int)retVal;
}

int chdir(char *path)
{
    char filePath[100];
    strcpy(filePath, path);
    if(path[strlen(filePath) - 1] != '/') strcat(filePath,"/");
    return chdirsyscall(filePath);
}

int currentDirSys(char* buf,size_t size){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(79), "g"((long)(buf)), "g"((long)(size)) : "rax", "rbx", "rcx");
    return (int)retVal;
}

char *getCurrentDirectory(char *dirPath, size_t size){
    currentDirSys(dirPath, size);
    return dirPath;
}

int closecall(int inp)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (3), "g" ((long)(inp)) : "rax", "rbx");
    return (int)retVal;
}

int fclose(int fd)
{
    return closecall(fd);
}

int dup2(int fd1, int fd2)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(33), "g"((long)(fd1)), "g"((long)(fd2)) : "rax", "rbx", "rcx");
    return (int)retVal;
}

int fopen(char *name, char *perm)
{
    int file = open(name, O_RDONLY);
    return file;   
}

off_t lseek(int fd, off_t offset, int p)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(8), "r"((long)(fd)), "r"((long)(offset)), "r"((long)(p)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (off_t)retVal;
}

int open(const char *path,int flags)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(2), "g"((long)(path)), "g"((long)(flags)) : "rax", "rbx", "rcx");
    return (int)retVal;
}

int close(int fd){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (3), "g" ((long)(fd)) : "rax", "rbx");
    return (int)retVal;
}

int open_dir(const char *path)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (16), "g" ((long)(path)) : "rax", "rbx");
    return (int)retVal;
}

DIR *opendir(const char *name){
    int glDirVar = open_dir(name);
    int index;
    if (glDirVar == -1) index = 0;
    else index = glDirVar;
    (&(dir[index]))->fd = glDirVar;
    strcpy((&((&(dir[index]))->d_ent))->d_name, (char *)name);
    return &dir[index];
}

int direccall(int fd,char* buff,int size){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(78), "r"((long)(fd)), "r"((long)(buff)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
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