#include <sys/dirent.h>

#define KERNEL_STACK_SIZE 512 

typedef struct taskStruct taskStruct;
typedef struct taskList taskList;
typedef struct vmaStruct vmaStruct;
typedef struct mmStruct mmStruct;
typedef struct file file;

#define HEAPTOP 0x8000000
#define HEAPBASE 0x10000000
#define STACKTOP 0xF0000000
#define STACKBASE 0x2000

int pid[100];

struct taskList{
	taskStruct* taskDetails;
	taskList* next;
};

struct file{
	uint64_t start;
	uint64_t offset;
	uint64_t size;
	uint64_t bss;
};

struct vmaStruct{
	mmStruct *mm;
	uint64_t start;
	uint64_t end;
	vmaStruct *next;
	uint64_t flags;
	uint64_t type;
	file *file;
};

struct mmStruct{
	vmaStruct *mmap, *current;
	uint64_t csStart, csnd;
	uint64_t dsStart, dsEnd;
	uint64_t brkStart, brk, stackStart;
	uint64_t argStart, argEnd;
	uint64_t envStart, envEnd;
	uint64_t rss, totalVM, lockedVM;
};

struct taskStruct{
	int pid;
	int ppid;
	uint64_t kernelStack;
	uint64_t rsp;
	uint64_t rip;
	enum {RUNNING, SLEEPING, ZOMBIE, READY} state;
	mmStruct *mm;
	uint64_t cr3;
	uint64_t initKern;
	uint64_t sleep;
	char pname[15];
	struct fd* fd[100];
	int lastChildPID;
	uint32_t numChild;
	uint64_t alarm;
	char curDir[80];
	file_t *curNode;
	taskStruct *next;
};

void setPID();

int getPID();

void initSchedule();

void createInitTask();

taskStruct *createUserTask(char *);

void *copyTaskStruct(taskStruct *);

void killTask(int, int);

void exitTask(taskStruct *);

void createKernelProcess(uint64_t);

void schedule();

void initschedule();

int fork();

int exec(char *, char **, char **);

int sys_open(char *, uint64_t *);

dir *sys_opendir(char *);
