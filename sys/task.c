#include <sys/defs.h>
#include <sys/freelist.h>
#include <sys/task.h>

static uint64_t generatePid = 0;

static taskList* taskList_current = NULL;

void createKernelProcess(uint64_t processAddress){
	taskStruct* newTask = NULL;

	newTask = (taskStruct *)getFreeFrame();
	newTask->pid = generatePid++;
	newTask->rsp = processAddress + KERNEL_STACK_SIZE;
	newTask->rbp = processAddress;
	newTask->state = RUNNING;
	newTask->exitStatus = 0;

	if (taskList_current == NULL){
		taskList_current = (taskList *)getFreeFrame();
	}
	else {
		taskList_current->next = (taskList *)getFreeFrame();
		taskList_current = taskList_current->next;
	}
	taskList_current->taskDetails = newTask;
	taskList_current->next = NULL;
}
