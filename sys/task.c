#include <sys/defs.h>
#include <sys/freelist.h>
#include <sys/gdt.h>
#include <sys/pagetable.h>
#include <sys/task.h>
#include <sys/kprintf.h>
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/syscall.h>

taskStruct *cleanupTask, *current, *zombieTask, *first, *sleepTask = NULL;

void setPID(){
	int i;
	for (i=0; i<100; i++) pid[i] = 0;
}

int getPID(){
	int i;
	for (i=0; i<100; i++){
		if (pid[i] == 0){
			pid[i] = 1;
			return i;
		}
	}
	return -1;
}

void killZombies(){
	if (zombieTask == NULL) return;

	taskStruct *task;
	while (zombieTask){
		task = zombieTask;
		zombieTask = zombieTask->next;
		exitTask(task);
	}
}

void initSchedule(){
	while (1) schedule();
}

void createInitTask(){
	taskStruct *init = (taskStruct *) getFreeFrame();

	init->pid = getPID();
	init->ppid = 0;
	init->mm = NULL;
	init->sleep = 0;
	init->alarm = 0;
	init->lastChildPID = -1;
	init->cr3 = (uint64_t)currentCR3();
	init->rip = (uint64_t)&initSchedule;
	init->numChild = 0;
	strcpy(init->pname, "init");

	init->kernelStack = (uint64_t)getFreeFrame() + 0x1000 - 16;
	init->rsp = init->kernelStack;
	init->initKern = init->kernelStack;

	init->next = init;

	first = init;
	current = init;
}

taskStruct *createUserTask(char *fname){
	tarfsHeader *file = readELF(fname);
	if(file == NULL){
		kprintf("Not supported\n");
		return NULL;
	}

	taskStruct *newTask = (taskStruct *)getFreeFrame();
	newTask->pid = getPID();
	newTask->state = RUNNING;
	newTask->lastChildPID = -1;
	newTask->numChild = 0;
	newTask->alarm = 0;
	strcpy(newTask->curDir, "/rootfs/bin/");
	
	dir *retDir = sys_opendir(newTask->curDir);
	newTask->curNode = retDir->node;
	strcpy(newTask->pname, fname);

	newTask->cr3 = (uint64_t) userAddressSpace();
	PML4E *currentcr3 = (PML4E *) currentCR3();

	setCR3((PML4E *)newTask->cr3);

	mmStruct *mm = (mmStruct *) getFreeFrame();
	newTask->mm = mm;

	newTask->kernelStack = (uint64_t)getFreeFrame() + 0x1000 - 16;

	int error = loadEXE(newTask, (void *)(file+1));
	if (error) return NULL;

	setCR3(currentcr3);

	current->next = newTask;
	newTask->next = current;
	current = newTask;

	return newTask;
}

void switchToRing3(taskStruct *task){
	task->state = RUNNING;

	volatile unsigned int a = 0x2B;

	__asm__ volatile ("movq %0, %%rax;\n\t"
			"ltr (%%rax);" : : "r" (&a));

	set_tss_rsp((void *)task->initKern);
//	tss.rsp0 = task->initKern;

	__asm__ volatile (
		"sti;"
		"movq %0, %%cr3;"
		"mov $0x23, %%ax;"
		"mov %%ax, %%ds;"
		"mov %%ax, %%es;"
		"mov %%ax, %%fs;"
		"mov %%ax, %%gs;"
		"movq %1, %%rax;"
		"pushq $0x23;"
		"pushq %%rax;"
		"pushfq;"
		"popq %%rax;"
		"orq $0x200, %%rax;"
		"pushq %%rax;"
		"pushq $0x1B;"
		"pushq %2;"
		"movq $0x0, %%rdi;"
		"movq $0x0, %%rsi;"
		"iretq;"
		: : "r" (task->cr3), "r" (task->rsp), "r" (task->rip));
}

void *copyTaskStruct(taskStruct *parent){
	taskStruct *childTask = (taskStruct *)getFreeFrame();

	childTask->pid = getPID();
	childTask->ppid = parent->pid;
	childTask->state = READY;
	childTask->next = NULL;
	childTask->mm = NULL;
	childTask->sleep = 0;
	childTask->alarm = 0;
	childTask->lastChildPID = -1;
	parent->numChild = parent->numChild + 1;
	childTask->numChild = 0;
	strcpy(childTask->pname, current->pname);

	childTask->cr3 = (uint64_t)getFreeFrame();
	setChildPagetables(childTask->cr3);

	setCR3((PML4E *)childTask->cr3);

	memcpy(&(childTask->fd[0]), &(parent->fd[0]), (sizeof(parent->fd[0])* 100));

	childTask->mm = (mmStruct *)getFreeFrame();
	memcpy(childTask->mm, parent->mm, sizeof(mmStruct));

	vmaStruct *parentVma = parent->mm->mmap;
	vmaStruct *childVma = NULL;
	int first = 0;

	while (parentVma){
		if (childVma == NULL) first = 1;
		childVma = (vmaStruct *)getFreeFrame();

		memcpy(childVma, parentVma, sizeof(vmaStruct));

		if (parentVma->file != NULL){
			childVma->file = (file *)getFreeFrame();
			memcpy(childVma->file, parentVma->file, sizeof(struct file));
		}

		if (first){
			childTask->mm->mmap = childVma;
			first = 0;
		}

		if (childVma->next) childVma = childVma->next;
		parentVma = parentVma->next;
	}

	if (!childVma){
		childTask->mm->mmap = NULL;
		goto ret;
	}
	childVma->next = NULL;

	ret : return (void *)childTask;
}

void contextSwitching(taskStruct *next, taskStruct *prev){
	set_tss_rsp((void *)next->initKern);
//	tss.rsp0 = next->initKern;

	__asm__ volatile (
		"sti;"
		"movq %%rsp, (%0)" : : "r" (&(prev->kernelStack))
		);

	__asm__ volatile (
		"movq %0, %%rsp;" : : "r" (next->kernelStack)
		);

	__asm__ volatile (
		"movq %0, %%cr3;" : : "r" (next->cr3)
		);

	__asm__ volatile (
                        "movq $1f, %0;"
                        "pushq %1;"
                        "retq;"
                        "1:\t"
                        : "=g" (current->rip)
                        : "r" (next->rip)
		);
}

void schedule(){
	killZombies();
	if (current != current->next) contextSwitching(current, current->next);
}

void freeMMStruct(mmStruct *mm){
	if (mm->mmap == NULL) return;

	vmaStruct *mmVma = mm->mmap;
	while (mmVma){
		vmaStruct *freeVma = mmVma;
		mmVma = mmVma->next;
		freeThisFrame((uint64_t)freeVma);
	}
}

void exitTask(taskStruct *task){
	freeMMStruct(task->mm);
	freeThisFrame((uint64_t) task->mm);
	task->mm = NULL;

	freeThisFrame((uint64_t)task->initKern);
	freeThisFrame((uint64_t)task);
}

void cleanUp(){
	taskStruct *next = cleanupTask->next;
	taskStruct *parent = NULL, *temp;

	temp = sleepTask;

	while (next->next != cleanupTask) next = next->next;

	if (sleepTask){
		parent = temp->next;
		if (temp->lastChildPID == cleanupTask->pid){
			__asm__ volatile("movq %1, %%rax;"
					"movq %%rax, %0;" : "=r" (sleepTask) : "r" (temp->next) : "%rax"
					);
			temp->numChild = temp->numChild - 1;
			parent = temp;
		}
		else {
			while (parent){
				if (parent->lastChildPID == cleanupTask->pid){
					parent->numChild -= 1;
					parent->state = RUNNING;
					parent->lastChildPID = -1;
					temp->next = parent->next;
				}
				parent = parent->next;
				temp = temp->next;
			}
		}
	}
	if (parent){
		parent->next = cleanupTask->next;
		next->next = parent;
		freePageTable((uint64_t)parent->cr3);
	}
	else {
		next->next = cleanupTask->next;
	}

	if (cleanupTask->numChild > 0){
		taskStruct *child = next->next;

		while (child->next != next->next){
			if (child->ppid == cleanupTask->pid) child->ppid = 0;
			child = child->next;
		}
		child = sleepTask;

		while (child){
			if (child->ppid == cleanupTask->pid) child->ppid = 0;
			child = child->next;
		}
	}

	if (cleanupTask->ppid == 0){
		cleanupTask->state = ZOMBIE;
		if (zombieTask == NULL){
			zombieTask = cleanupTask;
			zombieTask->next = NULL;
		}
		else {
			taskStruct *task = zombieTask;
			while (task->next) task = task->next;
			task->next = cleanupTask;
		}
	}

	current = next->next;

	if (cleanupTask->numChild == 0 && cleanupTask->ppid != 0) exitTask(cleanupTask);
	pid[cleanupTask->pid] = 0;
	contextSwitching(current, cleanupTask);
}


void killTask(int taskPid, int flag){
	int running = 0;
	int sleep = 0;

	if (current->pid == taskPid){
		running = 1;
		cleanupTask = current;
		goto cleanup;
	}

	taskStruct *prev = current;

	while (prev->next != current){
		if (taskPid == prev->pid){
			cleanupTask = prev;
			running = 1;
			break;
		}
		prev = prev->next;
	}

	if (prev->pid == taskPid) {
		cleanupTask = prev;
		running = 1;
	}

	cleanup:
		if (running){
			if (flag) kprintf("Killed process\n");
			cleanUp();
		}
		if (sleepTask->pid == taskPid){
			sleep = 1;
			cleanupTask = sleepTask;
			sleepTask = sleepTask->next;
			goto out;
		}

		taskStruct *temp = sleepTask;
		while (temp->next){
			if (temp->next->pid == taskPid){
				sleep = 1;
				cleanupTask = temp->next;
				temp->next = temp->next->next;
				break;
			}
			temp = temp->next;
		}

	out:
		if (sleep){
			kprintf("Killed process\n");
			pid[cleanupTask->pid] = 0;
			exitTask(cleanupTask);
		}
		else kprintf("Task not found\n");
}

void exit(int status){
	cleanupTask = current;
	cleanUp();
}

void outb (uint16_t port, uint16_t val){
	__asm__ volatile ("outb %%al, %%dx"::"d" (port), "a" (val));
}

int fork(){
	taskStruct *temp, *parent = NULL;
	taskStruct *child = (taskStruct *)copyTaskStruct((taskStruct *)current);

	temp = current->next;
	current->next = child;
	child->next = temp;

	parent = current;

	void *kstack = (void *)getFreeFrame();
	child->initKern = child->kernelStack = ((uint64_t)kstack + 0x1000 - 16);
	memcpy((void *)(child->initKern - 0x1000 + 16), (void *)(parent->initKern - 0x1000 + 16), 0x1000 - 16);

	volatile uint64_t stackPos;
	__asm__ __volatile__ ("mov %0, %%cr3": : "a"(parent->cr3));
	__asm__ __volatile__ ("movq $2f, %0;"
				"2:\t"
				:"=g" (child->rip));
	__asm__ __volatile__ ("movq %%rsp, %0;"
				: "=r" (stackPos));

	if (current == parent){
		child->kernelStack = child->initKern - (parent->initKern - stackPos);
		return child->pid;
	}
	else {
		outb(0x20, 0x20);
		return 0;
	}
}

int exec(char *fname, char **argv, char **envp){
	int i, argc = 0;
	char childPointer[6][64];
	for (i=0; i<6; i++) childPointer[i][0] = '\0';

	strcpy(childPointer[argc], fname);
	argc++;

	int childargc = 0;
	if (argv != NULL){
		while(argv[childargc] != NULL){
			strcpy(childPointer[argc], argv[childargc]);
			argc++; childargc++;
		}
	}

	taskStruct *task = (taskStruct *)getFreeFrame();
	task->pid = current->pid;
	task->ppid = current->ppid;
	strcpy(task->pname, fname);

	memcpy(&(task->fd[0]), &(current->fd[0]), (sizeof(current->fd[0])*100));

	tarfsHeader *file = readELF(fname);

	if (file == NULL){
		kprintf("From exec : invalid binary\n");
		return -1;
	}
	task->cr3 = (uint64_t)userAddressSpace();
	PML4E *currentcr3 = (PML4E *)currentCR3();
	__asm__ __volatile__ ("movq %0, %%cr3": : "a" (task->cr3));
	void *kstack = (void *)getFreeFrame();
	task->initKern = task->kernelStack = ((uint64_t)kstack + 0x1000 - 16);

	mmStruct *mm = (mmStruct *)getFreeFrame();
	task->mm = mm;
	int error = loadEXE(task, (void *)(file + 1));
	if (error) return -1;

	void *pointer = (void *)(STACKTOP + 0x1000 - 16 - sizeof(childPointer));
	memcpy(pointer, (void *)childPointer, sizeof(childPointer));

	for (i = argc; i>0; i--) *(uint64_t *)(pointer - 8*i) = (uint64_t)pointer + (argc-i)*64;

	pointer = pointer - 8*argc;
	task->rsp = (uint64_t)pointer;
	__asm__ __volatile__ ("movq %0, %%cr3": : "a" ((uint64_t)currentcr3));

	taskStruct *prev = current->next;
	while(prev->next != current) prev = prev->next;
	prev->next = task;
	task->next = current->next;
	current = task;

	strcpy(task->curDir, "/rootfs/bin");
	char temp[30] = "\0";
	strcpy(temp, task->curDir);

	task->lastChildPID = -1;
	task->state = RUNNING;

	set_tss_rsp((void *)task->initKern);

	__asm__ __volatile__ (
		"sti;"
		"movq %0, %%cr3;"
		"movq %1, %%rsp;"
		"mov $0x23, %%ax;"
		"mov %%ax, %%ds;"
		"mov %%ax, %%es;"
		"mov %%ax, %%fs;"
		"mov %%ax, %%gs;"
		"mov %2, %%rax;"
		"pushq $0x23;"
		"pushq %%rax;"
		"pushfq;"
		"popq %%rax;"
		"orq $0x200, %%rax;"
		"pushq %%rax;"
		"pushq $0x1B;"
		"pushq %3;"
		"movq %4, %%rsi;"
		"movq %5, %%rdi;"
		"iretq;"
		: : "r"(task->cr3), "r"(task->kernelStack), "r"(task->rsp), "r"(task->rip), "r"(pointer), "r"(argc));

	return -1;
}

/*
static uint64_t generatePid = 0;

static taskList* taskList_current = NULL, *taskList_last = NULL;

void addToTaskList(taskStruct *newTask){
	if (taskList_current == NULL){
                taskList_current = (taskList *)getFreeFrame();
                taskList_last = taskList_current;
        }
        else {
                taskList_current->next = (taskList *)getFreeFrame();
                taskList_current = taskList_current->next;
        }
        taskList_current->taskDetails = newTask;
        taskList_current->next = taskList_last;
}

void createKernelProcess(uint64_t processAddress){
	taskStruct *newTask;

	newTask = (taskStruct *)getFreeFrame();
	newTask->pid = generatePid++;
	newTask->rsp = processAddress + KERNEL_STACK_SIZE;
	newTask->rip = processAddress;
	newTask->state = RUNNING;

	addToTaskList(newTask);
}

void loadRIPwith(uint64_t newRIP){
	__asm__ volatile ("call %0;": : "m" (newRIP) : );
}

void contextSwitch(taskStruct *current, taskStruct *next){
//	__asm__ volatile ("movq %%rsp, %0": : "r" (&(current->kernelStack)));
//	__asm__ volatile ("movq %0, %%rsp": : "r" (current->next->kernelStack));
	__asm__ volatile (
			"sti;"
			"movq %%rsp, %0" : : "r" (&(current->kernelStack))
			);

	__asm__ volatile (
			"movq %0, %%rsp;" : : "r" (next->kernelStack)
			);

	__asm__ volatile ("movq %0, %%cr3" : : "r" (next->cr3));

	__asm__ volatile (
			"movq $1f, %0;"
			"pushq %1;"
			"retq;"
			"1:\t"
			: "=g" (current->rip)
			: "r" (next->rip));
}

void schedule(){
	taskStruct *current, *next;

	current = taskList_current->taskDetails;
	next = taskList_current->next->taskDetails;
	taskList_current = taskList_current->next;
	taskList_last = taskList_last->next;

	contextSwitch(current, next);
}

void initschedule(){
	taskStruct *current;

	taskList_current = taskList_current->next;
	taskList_last = taskList_last->next;
	current = taskList_current->taskDetails;
	
	loadRIPwith(current->rip);
}
*/
