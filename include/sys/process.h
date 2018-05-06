#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/file.h>

#define True 1
#define False 0
#define posInfinity 9999

#define MAX 1100
#define STACK_S 0x100FFFFF0000
#define VADDR_MASK 0xFFFFFFFFFFFFF000

int pid[MAX];

typedef struct vmaStruct {
	uint64_t beginAddress;
	uint64_t lastAddress;
	uint64_t offset;
    struct vmaStruct *next;
} vmaStruct;

typedef struct Register{
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, rflags, cr3;
}reg;

typedef struct taskStruct {
	char name[100];
	uint64_t kstack[512];
	uint64_t *ustack;
	int time;
	enum {RUNNING, SLEEP, ZOMBIE, IDLE, READY, WAIT, HANG} state;
	uint64_t pid;
	uint64_t ppid;
	uint64_t *rsp;
	uint64_t pml4e;
	int child_count;
	struct vmaStruct *vm;
	char curr_dir[100];
	struct file_t fd[100];
	int fd_c;
	struct Register regs;
}taskStruct;

taskStruct *currentTask;

void schedule();
int execvpe(char*, char* argv[], char* env[]);
void *memcpy(void *,const void *, int);
int fork();
void createNewTask(char*);
struct taskStruct taskQueue[MAX];
void initFirstTask();
void exit();
void initTask();
int wait();
int waitpid(int);
pid_t getTaskPID(void);
pid_t getTaskPPID(void);
int kill(int);
void getCurrentDirectory(char*, int);
int chdir(char*);
unsigned int sleep(unsigned int);
void ps();
void* malloc(int);

#endif