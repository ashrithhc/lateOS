#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/defs.h>

static char env_var[1000][1000];
static int env_length=0;
static char envBuffer[100];

char* getvar(char* name){
	int i;
	for (i=0; *name != '='; i++) envBuffer[i] = *name++;
	envBuffer[i] = '\0';
	return &envBuffer[0];
}

char* getallenv(int i){
	return &env_var[i][0];
}

char *getenv(const char *name){
	for(int i=0; i<env_length; i++)
	{
		getvar(&env_var[i][0]);
		return &env_var[i][strlen(&envBuffer[0])];
	}
	return NULL;	
}

int getenvlength()
{
	return env_length;
}

int setenv(char *name, char *value, int overwrite){
	for(int i=0; i<env_length; i++){
		getvar(&env_var[i][0]);
		if(strcmp(name, &envBuffer[0]) == 0){
			strcpy(&env_var[i][strlen(&envBuffer[0])+1], value);
			return 1;
		}
	}
	return 0;
}

void pushenvs(char* envp[]){
	int i;
	for (i=0; envp[i] != NULL; i++) strcpy(&env_var[i][0], envp[i]);
	env_length=i;  
}

int pipe(int fd[2])
{
	long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (22), "g" ((long)(&fd[0])) : "rax", "rbx");
    return (int)(retVal);
}

pid_t wait(int* status){
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; movq %5, %%rsi; int $0x80; movq %%rax, %0" : "=m" (retVal) : "g" (61),"g" ((long)(-1)),"g" ((long)(status)), "g" ((long)(0)), "g" ((long)(NULL)) : "rax", "rbx", "rcx", "rdx", "rsi");
	return (pid_t)retVal;
}

pid_t waitpid(pid_t pid,pid_t *status){
	long retVal;
	__asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(247), "g"((long)(pid)), "g"((long)(status)) : "rax", "rbx", "rcx");
	return (pid_t)retVal;
}

/* 
REFERENCED AS IS FROM 
https://github.com/germanoa/compiladores/blob/master/doc/ebook/The%20C%20Programming%20Language%20-%202nd%20Edition%20-%20Ritchie%20Kernighan.pdf
https://stackoverflow.com/questions/13159564/explain-this-implementation-of-malloc-from-the-kr-book
*/

typedef long Align;

union header {
    struct {
        union header *ptr;
        unsigned size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;

#define NALLOC 1024

void free(void *ap)
{
    Header *bp, *p;

    bp = (Header *)ap - 1;

    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break;

    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }

    freep = p;
}

void* sbrk_call(int increment){
    long retVal;
    __asm__ __volatile__ ("movq %1, %%rax; movq %2, %%rbx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g" (9), "g" ((long)(increment)) : "rax", "rbx");
    return (void *)retVal;
}

void* sbrk(int increment)
{
    return sbrk_call(increment);
}


static Header *morecore(int nu)
{
    char *cp;
    Header *up;

    if (nu < NALLOC)
        nu = NALLOC;

    cp = (char*)sbrk((int)(nu * sizeof(Header)));
    if (cp == (char *) -1)
        return NULL;

    up = (Header *) cp;
    up->s.size = nu;
    free(((void*)(up + 1)));

    return freep;
}

void *malloc(size_t nbytes)
{
    Header *p, *prevp;
    unsigned nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    if ((prevp = freep) == NULL) {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {

        if (p->s.size >= nunits) {
            if (p->s.size == nunits) {
                prevp->s.ptr = p->s.ptr;
            } else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p+1);
        }

        if (p == freep)
            if ((p = morecore(nunits)) == NULL)
                return NULL;
    }
}
