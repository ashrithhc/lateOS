#include <syscall.h>
#include <sys/defs.h>
#include <string.h>

int chdirsyscall(char inp[40])
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (80), "g" ((long)(inp)) : "rax", "rbx");
    return (int)(retVal);
}

int chdir(const char *path)
{
    char a[40];
    strcpy(a,(char *)path);
    int l = strlen(a);
    if(path[l-1] != '/'){
        strcat(a,"/");
    }
    return chdirsyscall(a);
}
int cwd_call(char* buf,size_t size){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(79), "g"((long)(buf)), "g"((long)(size)) : "rax", "rbx", "rcx");
    return (int)(retVal);
}
char *getCurrentDirectory(char *buf, size_t size){
    cwd_call(buf,size);
    return buf;
}

int strcmp(char *s,char *t){
	while(*s==*t)
	{
		if(*s=='\0')
			return 0;
		s++;
		t++;
	}
	return *s-*t;
}

void strcpy(char *string2, char *string1){

	while(*string1)
	{
		*string2=*string1;
		string1++;
		string2++;
	}
	*string2='\0';
}

int strlen(const char *string)
{
	int length=0;
	while(*string)
	{
		length++;
		string++;
	}
	return length;
}

char* strcat(char *string1, char *string2)
{
	while(*string1!='\0')
	{
		string1++;
	}
	while(*string2!='\0')
	{
		*string1=*string2;
		string1++;
		string2++;
	}
	*string1='\0';
	return string1;
}
