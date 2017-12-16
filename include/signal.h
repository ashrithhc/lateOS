#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/defs.h>

int kill(pid_t pid, int sig);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

#define T_SYSCALL   128
#define SYS_exit       60
#define SYS_brk        12
#define SYS_fork       57
#define SYS_getpid     39
#define SYS_getppid   110
#define SYS_execve     59
#define SYS_getdents   78
#define SYS_opendir    4
#define SYS_readdir    5
#define SYS_closedir   6
#define SYS_wait4      61
#define SYS_nanosleep  35
#define SYS_alarm      37
#define SYS_getcwd     79
#define SYS_chdir      80
#define SYS_open        2
#define SYS_read        0
#define SYS_write       1
#define SYS_lseek       8
#define SYS_close       3
#define SYS_pipe       22
#define SYS_dup        32
#define SYS_dup2       33
#define SYS_listprocess 7
#define SYS_killprocess 13
#define SYS_clearscreen 14
#define SYS_shutdown    15
#define SYS_listfiles   16
#define SYS_catfiles    17
#define SYS_echofiles   18
#define SYS_yield       19


static __inline uint64_t syscall_0(uint64_t n) {

	uint64_t a = -1;  
    __asm volatile("int $0x80" : "=a" (a) : "0" (n));  
    return a;
}

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t val;                                                                                                                          
    __asm volatile("movq %1,%%rax;"
                   "movq %2,%%rbx;"
                   "int $0x80;"
                   "movq %%rax,%0;"
                   :"=r"(val)
                   :"r"(n),"r"(a1)
                   :"rbp","rcx","rdx","rsi","rdi","r12","r11","r10","r9","r8","r13","r14","r15"
                  );
   return val;
}

static __inline uint64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) {
	uint64_t res;
    __asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0"(n) ,"b"((uint64_t)(a1)),"c"((uint64_t)(a2))\
                    :"cc","memory");
    return res;
}

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t a; 
    __asm__ volatile("int $0x80" : "=a" (a) : "0" (n), "D" ((uint64_t)a1), "S" ((uint64_t)a2), "b"((uint64_t)a3)); 
    return a;


}

#endif
