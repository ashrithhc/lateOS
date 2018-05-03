#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/defs.h>

int c;

int readCall(){

	// _syscall3(int, read, int, stdin,int*,&c, int, 1);
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(&c)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 0;
}

int getchar()
{
    readCall();
    return (int)c;
}	

int readcall1(char* s){
    // _syscall3(int, read, int, stdin,char* ,s, int, 4096);
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(s)), "r"((long)(4096)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
}
char* gets(char *string)
{
       char *s=string;
     readcall1(s);
      return string;
}

int puts(const char *s)
{
	int len=strlen(s);
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (1),"r" ((long)(stdout)),"r" ((long)(s)), "r" ((long)(len)) : "rax","memory","rbx","rcx","rdx");
	return (int)(retVal);
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

int fputs(const char *s,int fd )
{
	return 	puts(s);
/*	for(;*s;++s) if(fputchar(*s,fd)!=*s) return EOF;
	return fputchar('\n',fd)=='\n' ? 0 : EOF;	
*/
}

int fputchar(int c,int fd)
{
        int size = 1;
        long retVal;
	    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(1), "r"((long)(fd)), "r"((long)(&c)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
	    return (int)(retVal);
	return 0;
}

char c1;

int readcall(int fd)
{
	//putchar(f->fd);
	// _syscall3(int, read, int, fd, char*, &c, int, 1);
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(&c1)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 1;
}

char fgetc(int fd)
{
	readcall(fd);
	//putchar(*`c);
	return c1;
}

int clrscr_call()
{
    // _syscall(int, clearScreen);
    long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(7) : );
	return (int)retVal;
}
void clearScreen()
{
    clrscr_call();
    return;
}

char* fgets(char* string,int n,int f)
{
	
	char *s=string;
//	putchar(65 + f->fd);
	do{
	n = getchar(f);
	   *s++ = n;  
	}while((n!='\n')&&(n!=EOF));
	*s='\n';
	return string;
}