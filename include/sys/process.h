#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/file.h>

#define MAX 1100
#define STACK_S 0x100FFFFF0000

#define True 1
#define False 0

/**

VMA & Task struct References:
	 http://venkateshabbarapu.blogspot.com/2012/09/process-segments-and-vma.html
	    http://duartes.org/gustavo/blog/post/how-the-kernel-manages-your-memory/
**/

int pid[MAX];
typedef struct vmaStruct {
	uint64_t beginAddress;
	uint64_t lastAddress;
	uint64_t offset;
    struct vmaStruct *next;
} vma;

typedef struct Register{
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, rflags, cr3;
}reg;

typedef struct taskStruct {
	char name[50];
	uint64_t kstack[512];
	uint64_t *ustack;
	int time;
	enum {
		RUNNING,
		SLEEP,
		ZOMBIE,
		IDLE,
		READY,
		WAIT,
        HANG
	} state;
	uint64_t pid;
	uint64_t ppid;
	uint64_t *rsp;
	uint64_t pml4e;
	int child_count;
	struct vmaStruct *vm;
	char curr_dir[50];
	struct file_t fd[25];
	int fd_c;
	struct Register regs;
}taskStruct;

void switch_task(reg*, reg*);
void init_task();
void create_task(taskStruct*,uint64_t main, uint64_t flags, uint64_t pagedir);
void schedule();
void switchtor3();
int execvpe(char* file, char *argv[],char* env[]);
void *memcpy(void *dst,const void *src, int count);
int get_pid();
int get_ppid();
int fork();
void create_process(char* filename);
void addToQ(taskStruct *q);
int get_fd(struct taskStruct*);
taskStruct *r;
struct taskStruct taskQueue[MAX];
void init_proc();
void exit();
void init_p();
int wait();
int waitpid(int pid);
pid_t getpid(void);
pid_t getppid(void);
int kill(int pid);
void getcwd(char *buf, int size);
int chdir(char* path);
unsigned int sleep(unsigned int seconds);
void ps();
void* malloc(int no_of_bytes);
#endif
