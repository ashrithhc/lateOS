#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/defs.h>

int charRet;

int readCall(){
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(&charRet)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
}

int getchar()
{
    readCall();
    return (int)charRet;
}	

int readcallString(char* s){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(stdin)), "r"((long)(s)), "r"((long)(4096)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
}

char* gets(char *inpString)
{
	char *myStr = inpString;
	readcallString(myStr);
	return inpString;
}

int puts(const char *s)
{
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (1),"r" ((long)(stdout)),"r" ((long)(s)), "r" ((long)(strlen(s))) : "rax","memory","rbx","rcx","rdx");
	return (int)retVal;
}

int strcmp(char *str1, char *str2){
	while(*str1 == *str2)
	{
		if(*str1 == '\0') return 0;
		str1++; str2++;
	}
	return (*str1 - *str2);
}

void strcpy(char *str2, char *str1){
	while(*str1)
	{
		*str2 = *str1;
		str1++; str2++;
	}
	*str2 = '\0';
}

int strtoInt(char* str){
    int num = 0;
    for(int i=0; i<strlen(str); i++) num = (num * 10) + (str[i] - '0');
    return num;
}

int strlen(const char *string)
{
	int length=0;
	while(*string) { length++; string++; }
	return length;
}

char* strcat(char *str1, char *str2)
{
	while(*str1 != '\0') str1++;
	while(*str2 != '\0')
	{
		*str1 = *str2;
		str1++; str2++;
	}
	*str1 = '\0';
	return str1;
}

int fputs(const char *s, int fromFile)
{
	return puts(s);
}

int fputchar(int c,int fd)
{
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(1), "r"((long)(fd)), "r"((long)(&c)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)retVal;
}

char charRetF;
int readFCall(int fd)
{
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(&charRetF)), "r"((long)(1)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
	return 1;
}

char fgetc(int fd)
{
	readFCall(fd);
	return charRetF;
}

int clearScreenSys()
{
    long retVal;
	__asm__ __volatile__ ("int $0x80;" : "=a"(retVal) : "a"(7) : );
	return (int)retVal;
}

void clearScreen()
{
    clearScreenSys();
    return;
}

char* fgets(char* inpStr, int num, int fileDesc)
{
	char *str = inpStr;
	do{
		num = getchar(fileDesc);
		*str++ = num;  
	} while ((num != '\n') && (num != EOF));
	*str = '\n';
	return inpStr;
}