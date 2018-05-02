#include <sys/defs.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/string.h>
#include <sys/tarfs.h>
#include <sys/elf64.h>
#include <sys/terminal.h>

static taskStruct *duplicateTask;

int stillAlive(int i){
    if((&taskQueue[i])->state == RUNNING || (&taskQueue[i])->state == SLEEP || (&taskQueue[i])->state == WAIT || (&taskQueue[i])->state == HANG) {
        return 1;
    }
    return 0;
}

void ps()
{
    kprintf("------START-------\n");
    for(int i=0; i<MAX; i++)
        if(stillAlive(i) == 1) {
            kprintf("PID : %d\n", (&taskQueue[i])->pid);
            kprintf("Process : %s\n", (&taskQueue[i])->name);
            kprintf("\n");
        }
    kprintf("------END-------\n");
}

int checkTaskState(int i){
    if ((taskQueue + i)->state == READY) return True;
    return False;
}

int newPID(){
    int pid;
	for(pid = 0; pid<MAX; pid++) if(checkTaskState(pid)) return pid;
	return -1;
}

void init_proc(){
    for(int i=0; i<MAX; i++){
        (&(taskQueue[i]))->state = READY;
        (&(taskQueue[i]))->fd_c = 0;
        (&(taskQueue[i]))->time = 0;
    }
}

void loadPML4E(uint64_t newPML4){
    __asm__ __volatile__("movq %0, %%cr3;" : : "r"(newPML4) : );
}

void *memcpy(void *dest, const void *src, int n){
    unsigned char *retDest = (unsigned char *)dest;
    const unsigned char *retSrc = (unsigned char *)src;

    if (retSrc < retDest) for(retDest+=n, retSrc+=n; n--;) *(--retDest) = *(--retSrc);
    else while(n--) *retDest++ = *retSrc++;
    return dest;
}

uint64_t getCurrentCR3(){
    uint64_t processCR3;
    __asm__ __volatile__ ("movq %%cr3,%0" : "=r"(processCR3) : : );
    return processCR3;    
}

void loadCR3(uint64_t toLoad){
    __asm__ __volatile__ ("movq %0, %%cr3;" : : "r"(toLoad));
}

void loadCR3Virtual(uint64_t toLoad){
    __asm__ __volatile__ ("movq %0, %%cr3;" : : "r"(( uint64_t*)(toLoad - (uint64_t)kernbase)));
}

void in(){
    while(True) wait();
}

void idle(){
    while(True) {
        __asm__ __volatile__("sti");
        __asm__ __volatile__("hlt");
        __asm__ __volatile__("cli");
        schedule();
    }
}

void setupTask(char *name, uint64_t function){
    int pid = newPID();
    (taskQueue + pid)->pid = pid;
    strcpy((taskQueue + pid)->name, name);
    (taskQueue + pid)->state = RUNNING;
    (&(taskQueue + pid)->regs)->rip = function;
    (&(taskQueue + pid)->regs)->rsp = (uint64_t)(&((taskQueue + pid)->kstack[511]));
    (taskQueue + pid)->pml4e = getCurrentCR3();
}

uint64_t setupVMA(vmaStruct* vm, Elf64_Phdr* ep){
    // vm = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
    vm->beginAddress = ep->p_vaddr;
    vm->lastAddress = ep->p_vaddr+ep->p_memsz;
    uint64_t memCount = vm->beginAddress;
    if((((uint64_t)(ep->p_vaddr))% ((uint64_t)pageSize)) != 0){
        memCount = (uint64_t)((uint64_t)ep->p_vaddr & (uint64_t)VADDR_MASK);
    }
    return memCount;
}

void initTask(){
    setupTask("init", (uint64_t)&in);
    setupTask("idle", (uint64_t)&idle);
}

taskStruct* initTaskVariables(char *filename){
    int pid = newPID();
    taskStruct *task = (taskStruct *) &taskQueue[pid];
    strcpy(task->name, filename);
    strcpy(task->curr_dir, "/");
    task->ppid = 0;
    task->pid = pid;
    task->vm = NULL;
    task->state = RUNNING;

    return task;
}

vmaStruct* validateTaskVM(taskStruct *task){
    if (task->vm == NULL) return NULL;
    return task->vm;
}

uint64_t* readELFandFork(uint64_t fileAddress, taskStruct *ts){
    Elf64_Ehdr* eh = (Elf64_Ehdr*)(fileAddress);
    uint64_t* pml4 = (uint64_t *)getNewPage();
    memset(pml4,0,pageSize);
    ts->pml4e =( uint64_t )((uint64_t)pml4 - (uint64_t)kernbase);
    ts->regs.rip = eh->e_entry;
    for(int i=eh->e_phnum;i>0;i--){
        Elf64_Phdr* ep = (Elf64_Phdr*)(fileAddress + (eh->e_phoff));
        ep = ep + (i-1);
        if(ep->p_type != 1) continue;
        vmaStruct* vm = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
        uint64_t k = setupVMA(vm, ep); 
        vm->next = validateTaskVM(ts);
        ts->vm = vm;
        while(k<(vm->lastAddress)){
            uint64_t yy = getFreeFrame();
            init_pages_for_process(k,yy, pml4);
            k+=pageSize;
        }
        uint64_t currentCR3 = getCurrentCR3();
        uint64_t* pl =( uint64_t*)((uint64_t)pml4 - (uint64_t)kernbase);
        __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(pl));
        memcpy((void*)vm->beginAddress,(void*)(eh + ep->p_offset), (uint64_t)(ep->p_filesz));
        memset((void*)(vm->beginAddress + (uint64_t)(ep->p_filesz)), 0, (uint64_t)(ep->p_memsz) - (uint64_t)(ep->p_filesz));
        __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(currentCR3));
    }
    return pml4;
}

uint64_t* readELFandExec(uint64_t fileAddress, taskStruct *ts){
    Elf64_Ehdr* eh = (Elf64_Ehdr*)(fileAddress);
    ts->regs.rip = eh->e_entry;
    uint64_t* pml4 = (uint64_t *)(ts->pml4e + kernbase);
    dealloc_pml4((ts->pml4e));

    for(int i=eh->e_phnum;i>0;i--){
        Elf64_Phdr* ep = (Elf64_Phdr*)(fileAddress + (eh->e_phoff));
        ep = ep + (i-1);
        if(ep->p_type != 1) continue;
        vmaStruct* vm = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
        uint64_t k = setupVMA(vm, ep); 
        vm->next = validateTaskVM(ts);
        ts->vm = vm;
        for(;k<( vm->lastAddress);k+=pageSize){
            uint64_t yy = getFreeFrame();
            init_pages_for_process(k,yy, pml4);
        }

        uint64_t pcr3;
        __asm__ __volatile__ ("movq %%cr3,%0;" :"=r"(pcr3)::);
        uint64_t* pl =( uint64_t* )((uint64_t)pml4 - (uint64_t)kernbase);
        __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(pl));
        memcpy((void*)vm->beginAddress,(void*)(eh + ep->p_offset), (uint64_t)(ep->p_filesz));
        memset((void*)(vm->beginAddress + (uint64_t)(ep->p_filesz)), 0, (uint64_t)(ep->p_memsz) - (uint64_t)(ep->p_filesz));
        __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(pcr3));
    }
    return pml4;
}

void setMyVMA(taskStruct *ts, uint64_t from, uint64_t toend){
    vmaStruct* taskVMA = (vmaStruct *) kmalloc(sizeof(struct vmaStruct));
    taskVMA->beginAddress = from;
    taskVMA->lastAddress = toend;
    taskVMA->next = ts->vm;
    ts->vm = taskVMA;
}

void shiftTaskVMA(taskStruct *ts, uint64_t from, uint64_t toend){
    vmaStruct* vm2 = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
    vm2->beginAddress = from;
    vm2->lastAddress = toend;
    vm2->next = ts->vm;
    ts->vm = vm2;
}

void setNewVMA(taskStruct *ts, uint64_t* pml4, uint64_t from, uint64_t toend){
    init_pages_for_process(from,getFreeFrame(),pml4);
    ts->ustack = (uint64_t*)from;
    ts->rsp = (uint64_t *)((uint64_t)ts->ustack + (510 * 8));
    vmaStruct* vm = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
    vm->beginAddress = from;
    vm->lastAddress = toend;
    vm->next = ts->vm;
    ts->vm = vm;
}

void setTaskRSP(char *name, taskStruct *ts){
    int len = strlen(name)+1;
    ts->rsp = ts->rsp - len;
    memcpy(ts->rsp,name,len);

    int i;
    for (i=0; i<4; i++){
        ts->rsp -= 1;
        if (i<2) *(ts->rsp) = 0;
        else if (i==2) *(ts->rsp) = (uint64_t)((ts->rsp)+1);
        else *(ts->rsp) = 0x1;
    }
    currentTask = ts;
}

void setStackTask(taskStruct *task){
    task->kstack[14] = posInfinity;
}

void createNewTask(char* filename){
	uint64_t fileAddress = get_file_address(filename) + 512;
/*	if(fileAddress < 512){
		kprintf("No such file\n");
		return;
	}
*/
    char tempFilename[100];
    strcpy(tempFilename,filename);
    taskStruct *newTask = initTaskVariables(filename);

    uint64_t* pml4 = readELFandFork(fileAddress, newTask);
    setMyVMA(newTask, 0x4B0FFFFF0000, 0x4B0FFFFF0000);
	init_pages_for_process(STACK_S, (uint64_t)getFreeFrame(), pml4);
	newTask->ustack = (uint64_t*)STACK_S;
	newTask->rsp = (uint64_t *)((uint64_t)newTask->ustack + (510 * 8));
    setMyVMA(newTask, STACK_S, 0x100FFEFF0000);
	set_tss_rsp(&(newTask->kstack[511]));
    loadCR3Virtual((uint64_t)pml4);

    setTaskRSP(tempFilename, newTask);

	__asm__ __volatile__("\
        pushq $0x23;\
        pushq %0;\
        pushf;\
        pushq $0x2B;" : : "r"(newTask->rsp) : "%rax", "%rsp");
	__asm__ __volatile__("pushq %0" : : "r"((&(newTask->regs))->rip) : "memory");
	__asm__ __volatile__("iretq");
}

void copyVMA(taskStruct *curTask, taskStruct *copyTask){
    vmaStruct* a = curTask->vm;
    vmaStruct* p = NULL;
    copyTask->vm = NULL;
    while(a!=NULL){ 
        vmaStruct* duplicateTask = (vmaStruct *)kmalloc(sizeof(struct vmaStruct));
        memcpy(duplicateTask,a,sizeof(struct vmaStruct));
        if(p == NULL){
            p = duplicateTask;
            copyTask->vm = p;  
        }
        else{
            p->next = duplicateTask;
            p = duplicateTask;
        }
        a = a->next;
    }
}

uint64_t getMemorysize(int num){
    return 511*num;
}

void copytask(taskStruct *c){
	c->ppid = currentTask->pid;

	c->pml4e = (uint64_t)((uint64_t)getNewPage() - (uint64_t)kernbase);
    memset((uint64_t*)(c->pml4e+(kernbase)),0,pageSize);
	strcpy(c->name,currentTask->name);
    strcpy(c->curr_dir,currentTask->curr_dir);

    copytables(currentTask, c);
	copyVMA(currentTask, c);
}

void increaseChildCount(taskStruct *currentTask){
    currentTask->child_count += 1;
}

void createChild(){
    int dupPID = newPID();
    duplicateTask = (taskStruct *) &taskQueue[dupPID];
    duplicateTask->pid = dupPID;
    copytask(duplicateTask);    
    duplicateTask->ustack = (uint64_t*)STACK_S;
    duplicateTask->rsp = (uint64_t *)((uint64_t)STACK_S + 511*8);
    duplicateTask->state = RUNNING;
}

int fork(){
    createChild();

    uint64_t currentCR3 = getCurrentCR3();
    loadCR3(currentCR3);

    increaseChildCount(currentTask);
	
	memcpy(&(duplicateTask->kstack[0]), &(currentTask->kstack[0]), 512*8);
    setStackTask(duplicateTask);

	__asm__ __volatile__("movq 8(%%rsp), %%rax;movq %%rax, %0;" : "=g"((&(duplicateTask->regs))->rip) : : "memory", "%rax");
    uint64_t rspAddress;
	__asm__ __volatile__("movq %%rsp, %0;" : "=g"(rspAddress) : : "memory");

    (&(duplicateTask->regs))->rsp = (uint64_t) ((uint64_t)&(duplicateTask->kstack[511]) - (uint64_t)((uint64_t)&(currentTask->kstack[511]) - (uint64_t)rspAddress));
    return duplicateTask->pid;
}

int validPath(char *ref){
    if (ref[0] == '.' && ref[1] == '/') return 1;
    return 0;
}

int setFileAddress(char* path, char *file, uint64_t *fileAddress, taskStruct *ts, int *argc, char args[100][100], char *argv[]){
    if(validPath(path)){
        strcpy(file, &(currentTask->curr_dir[1]));
        strcat(file, path+2);
        uint64_t scriptChar = get_file_address(file)+512;
        if(scriptChar<512){
            return -1;
        }
        if( *((char *)scriptChar) == '!' && *((char *)scriptChar+1) == '#') {
            char ex[100];
            int q= 0;
            scriptChar+=2;
            while( (*(char *) scriptChar) != '\n'){
                if( (*(char *) scriptChar) != ' ') {
                    ex[q++] = *(char *) scriptChar;
                }
                scriptChar = scriptChar+1;
            }
            ex[q++]='\0';
            if(strlen(ex) == 0){
                strcpy(ex,"bin/sbush");
            }
            *fileAddress = get_file_address(ex) + 512;
            strcpy(args[*argc++], argv[0]);
            strcpy(args[*argc++], path+2);
        }
        else {
            *fileAddress = get_file_address("bin/sbush") + 512;
            strcpy(args[*argc++], argv[0]);
            strcpy(args[*argc++], path+2);
        }
    }
    else {
        strcpy(file, "bin/");
        strcat(file, path);
        strcpy(ts->name, file);


        *fileAddress = get_file_address(file) + 512;
        if (*fileAddress < 512) {
            return -1;
        }

        return 0;
    }

    return 1;
}

void setRSPandExec(taskStruct *ts, int envl, int argc, char envs[100][100], char args[100][100]){
    uint64_t* temp1[envl];
    for(int i=envl-1;i>=0;i--){
        int l = strlen(envs[i])+1;
        ts->rsp = ts->rsp-l;
        memcpy(ts->rsp,envs[i],l);
        temp1[i] = ts->rsp;
    }

    uint64_t* temp[argc];
    for(int i=argc-1;i>=0;i--){
        int l = strlen(args[i])+1;
        ts->rsp = ts->rsp-l;
        memcpy(ts->rsp,args[i],l);
        temp[i] = ts->rsp;
    }

    ts->rsp -= 1;
    *(ts->rsp) = 0;
    for(int i=envl-1;i>=0;i--){
        (ts->rsp)-=1;
        *(ts->rsp) = (uint64_t)temp1[i];
    }

    ts->rsp -= 1;
    *(ts->rsp) = 0;

    for(int i=argc-1;i>=0;i--){
        (ts->rsp)-=1;
        *(ts->rsp) = (uint64_t)temp[i];
    }
    (ts->rsp)-=1;
    *(ts->rsp) = argc;
    set_tss_rsp(&(ts->kstack[511]));
    __asm__ __volatile__("\
    push $0x23;\
    push %0;\
    pushf;\
    push $0x2B;\
    push %1"::"g"(ts->rsp),"g"(ts->regs.rip):"memory");
    __asm__ __volatile__("iretq;");
}

int execvpe(char* path, char *argv[],char* env[]){
	taskStruct *ts = currentTask;
    char file[100];
    int argc = 0, envl = 0;
    uint64_t fileAddress = 0 ;
    char args[100][100];
    char envs[100][100]; 
    int binValue = setFileAddress(path, file, &fileAddress, ts, &argc, args, argv);
    if (binValue == -1) return -1;
    else if (binValue == 0) {
        while (argv[argc]) {
            strcpy(args[argc], argv[argc]);
            argc++;
        }
    }
    while (env[envl]) {
        strcpy(envs[envl], env[envl]);
        argc++;
    }
    if (fileAddress < 512) {
        return -1;
    }

    uint64_t* pml4 = (uint64_t*)readELFandExec(fileAddress, ts);

    shiftTaskVMA(ts, 0x4B0FFFFF0000, 0x4B0FFFFF0000);

    setNewVMA(ts, pml4, STACK_S, 0x100FFEFF0000);

    setRSPandExec(ts, envl, argc, envs, args);
	return 1;
}

void exit(){
    currentTask->state = ZOMBIE;
    for (int i = 0; i < MAX; ++i) {
        if((taskQueue + i)->ppid == currentTask->pid){
            (taskQueue + i)->ppid = 0;
        }
    }
    if((taskQueue + currentTask->ppid)->state ==  WAIT){
        (taskQueue + currentTask->ppid)->state =  RUNNING;
    }
    schedule();
}
void removeProcess(int i){
    vmaStruct* vm = (taskQueue + i)->vm;
    while(vm){
        uint64_t k = (uint64_t)vm;
        vm = vm->next;
        free(k-kernbase);
    }
    dealloc_pml4((taskQueue + i)->pml4e);
    free((taskQueue + i)->pml4e);
}
int wait(){
    while(1) {
        for (int i = 0; i < MAX; ++i) {
            if (((taskQueue + i)->ppid == currentTask->pid) && ((taskQueue + i)->state == ZOMBIE)) {
                removeProcess(i);
                (taskQueue + i)->state = READY;
                return i;
            }
        }
        currentTask->state = WAIT;
        schedule();
        for (int i = 0; i < MAX; ++i) {
            if (((taskQueue + i)->ppid == currentTask->pid) && ((taskQueue + i)->state == ZOMBIE)) {
                removeProcess(i);
                (taskQueue + i)->state = READY;
                return i;
            }
        }
    }
}

int kill(int pid){
    (taskQueue + pid)->state = ZOMBIE;
    for (int i = 0; i < MAX; ++i) {
        if((taskQueue + i)->ppid == pid){
            (taskQueue + i)->ppid = 0;
        }
    }
    if((taskQueue + (taskQueue + pid)->ppid)->state == WAIT){
        (taskQueue + (taskQueue + pid)->ppid)->state = RUNNING;
    }
    return 1;
}

int waitpid(int pid){
    int i = pid;
    if(((taskQueue + i)->ppid == currentTask->pid) && ((taskQueue + i)->state == ZOMBIE)){
        removeProcess(i);
        (taskQueue + i)->state = READY;
        return i;
    }
    currentTask->state = WAIT;
    while(1){
        schedule();
        if(((taskQueue + i)->ppid == currentTask->pid) && ((taskQueue + i)->state == ZOMBIE)){
            removeProcess(i);
            (taskQueue + i)->state = READY;
            return i;
        }
    }
}

pid_t getTaskPID(void){
	return currentTask->pid;
}

pid_t getTaskPPID(void){
	return currentTask->ppid;
}

unsigned int sleep(unsigned int seconds){
    if(seconds <= 0){
        return 0;
    }
    currentTask->time = seconds;
    currentTask->state = HANG;
    schedule();
    return 0;
}

void schedule(){
    taskStruct *finalTask = currentTask;
    int nextPID = (finalTask->pid+1) % MAX;
    while(nextPID != finalTask->pid){         
        if(taskQueue[nextPID].state == RUNNING){
            break;
        }
        nextPID = (nextPID+1)%MAX;
    }
    currentTask = (taskStruct *)&taskQueue[nextPID];
    set_tss_rsp((uint64_t*)&(currentTask->kstack[511]));
    __asm__ __volatile__("movq %%rsp, %0;" : "=g"((&(finalTask->regs))->rsp) : : "memory");
    __asm__ __volatile__("movq %0, %%rsp;" : : "r"((&(currentTask->regs))->rsp) : "memory");
    loadPML4E(currentTask->pml4e);
    __asm__ __volatile__ ("movq $1f, %0" : "=g"((&(finalTask->regs))->rip) : : );
    __asm__ __volatile__ ("pushq %0;" : : "r"((&(currentTask->regs))->rip) : );
    __asm__ __volatile__("retq");
    __asm__ __volatile__ ("1:\t");
}

int chdir(char* path){
    if(strcmp("../",path) == 0){
        char k[100];
        strcpy(k,path);
        deriveRelative(k);
        if(strcmp(k,"")==0){
            strcpy(currentTask->curr_dir,"/");
            return 0;
        }
    }
    if((isValidDirectory(path)) > -1){
        char k[100];
        strcpy(k,path);
        deriveRelative(k);
        char l[100];
        l[0] = '/';
        l[1] = '\0';
        strcat(l,k);
        strcpy(currentTask->curr_dir,l);
        return 0;
    }
    return  -1;
}

void getCurrentDirectory(char *buf, int size){

    strcpy(buf,currentTask->curr_dir);
    int l = strlen(buf);
    if(l == 1){
        return;
    }
    else{
        buf[l-1] = '\0';
    }
}

void* malloc(int no_of_bytes){
    vmaStruct* vm = currentTask->vm->next;

    uint64_t ret = vm->lastAddress;
    for(int i =0;i<((no_of_bytes/pageSize))+1;i++){
        uint64_t s_add = getFreeFrame();
        init_pages_for_process(vm->lastAddress,s_add,(uint64_t *)(currentTask->pml4e+kernbase));
        vm->lastAddress = vm->lastAddress + pageSize;
    }
    return (uint64_t*)ret;
}
