#define KERNEL_STACK_SIZE 512 

typedef struct taskStruct taskStruct;
typedef struct taskList taskList;

struct taskList{
	taskStruct* taskDetails;
	taskList* next;
};

struct taskStruct{
//	char kernelStack[KERNEL_STACK_SIZE];
	uint64_t pid;
	uint64_t rsp;
	uint64_t rbp;
	enum {RUNNING, SLEEPING, ZOMBIE} state;
	int exitStatus;
};

void createKernelProcess(uint64_t);

void schedule();
