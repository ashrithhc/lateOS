#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/file.h>

#define MAX 1100
#define STACK_S 0x100FFFFF0000
#define VADDR_MASK 0xFFFFFFFFFFFFF000

#define True 1
#define False 0
#define posInfinity 9999

/**

vmaStruct & Task struct References:
	 http://venkateshabbarapu.blogspot.com/2012/09/process-segments-and-vmaStruct.html
	    http://duartes.org/gustavo/blog/post/how-the-kernel-manages-your-memory/
**/

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

void switch_task(reg*, reg*);
void init_task();
void create_task(taskStruct*,uint64_t main, uint64_t flags, uint64_t pagedir);
void schedule();
void switchtor3();
int execvpe(char* file, char *argv[],char* env[]);
void *memcpy(void *dst,const void *src, int count);
int fork();
void createNewTask(char* filename);
void addToQ(taskStruct *q);
int get_fd(struct taskStruct*);
struct taskStruct taskQueue[MAX];
void init_proc();
void exit();
void initTask();
int wait();
int waitpid(int pid);
pid_t getTaskPID(void);
pid_t getTaskPPID(void);
int kill(int pid);
void getcwd(char *buf, int size);
int chdir(char* path);
unsigned int sleep(unsigned int seconds);
void ps();
void* malloc(int no_of_bytes);
#endif
