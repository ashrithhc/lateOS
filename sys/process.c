#include <sys/defs.h>
#include <sys/process.h>
#include <sys/mem.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/string.h>
#include <sys/tarfs.h>
#include <sys/elf64.h>
#include <sys/terminal.h>

# define kernbase 0xffffffff80000000
# define validatebits 0xFFFFFFFFFFFFF000
# define pageSize 0x1000

static task_struct* p;
static task_struct* new;

int newPID(){
	for(int i=0; i<MAX; i++) if(taskQueue[i].state == READY) return i;
	return -1;
}

void *memcpy(void *dest, const void *src, int n){
    unsigned char *retDest = (unsigned char *)dest;
    const unsigned char *retSrc = (unsigned char *)src;

    if (retSrc < retDest) for(retDest+=n, retSrc+=n; n--;) *(--retDest) = *(--retSrc);
    else while(n--) *retDest++ = *retSrc++;
    return dest;
}

void in(){
    while(1){
        wait();
    }
}
void idle(){
    while(1) {
        __asm__ volatile("sti");
        __asm__ volatile("hlt");
        __asm__ volatile("cli");
        yield();
    }
}

void setupTask(char *name, uint64_t function){
    int pid = newPID();
    (taskQueue + pid)->pid = pid;
    strcpy((taskQueue + pid)->name, name);
    (taskQueue + pid)->state = RUNNING;
    (&(taskQueue + pid)->regs)->rip = function;
    (&(taskQueue + pid)->regs)->rsp = (uint64_t)(&((taskQueue + pid)->kstack[511]));
    uint64_t pcr3;
    __asm__ volatile ("movq %%cr3,%0;" :"=r"(pcr3)::);
    (taskQueue + pid)->pml4e = pcr3;
}

void init_p(){
    setupTask("init", (uint64_t)&in);
    setupTask("idle", (uint64_t)&idle);
}

int stillAlive(int i){
    if((&taskQueue[i])->state == RUNNING || (&taskQueue[i])->state == SLEEPING || (&taskQueue[i])->state == WAIT || (&taskQueue[i])->state == SUSPENDED) {
        return 1;
    }
    return 0;
}

void ps()
{
	kprintf("pid  |  process\n");
	for(int i=0; i<MAX; i++)
	{
        if(stillAlive(i) == 1) {
            kprintf("%d  |", (&taskQueue[i])->pid);
            kprintf("  %s", (&taskQueue[i])->name);
            kprintf("\n");
        }
	}
}

void initTaskVariables(task_struct *task, char *filename, int pid){
    strcpy(task->name, filename);
    strcpy(task->curr_dir, "/");
    task->ppid = 0;
    task->pid = pid;
    task->vm = NULL;
    task->state = RUNNING;
}

uint64_t* yappaFunction(uint64_t fileAddress, task_struct* ts){
    Elf64_Ehdr* eh = (Elf64_Ehdr*)(fileAddress);
    uint64_t* pml4 = (uint64_t *)getNewPage();
    memset(pml4,0,pageSize);
    ts->pml4e =( uint64_t )((uint64_t)pml4 - (uint64_t)kernbase);
    ts->regs.rip = eh->e_entry;
    for(int i=eh->e_phnum;i>0;i--){
        Elf64_Phdr* ep = (Elf64_Phdr*)(fileAddress + (eh->e_phoff));
        ep = ep + (i-1);
        if(ep->p_type == 1){               

            vma* vm = (vma *)kmalloc(sizeof(struct vmaStruct));
            vm->beginAddress = ep->p_vaddr;
            vm->lastAddress = ep->p_vaddr+ep->p_memsz;
            uint64_t k = vm->beginAddress;
            if((((uint64_t)(ep->p_vaddr))% ((uint64_t)pageSize)) != 0){
                k = (uint64_t)((uint64_t)ep->p_vaddr & (uint64_t)0xFFFFFFFFFFFFF000);
            }
            if(ts->vm == NULL){
                vm->next = NULL;
                ts->vm = vm;
            }
            else{
                vm->next = ts->vm;
                ts->vm = vm;
            }
            while(k<(vm->lastAddress)){
                uint64_t yy = getFreeFrame();
                init_pages_for_process(k,yy, pml4);
                k+=pageSize;
            }
            uint64_t pcr3;
            __asm__ __volatile__ ("movq %%cr3,%0;" :"=r"(pcr3)::);
            uint64_t* pl =( uint64_t*)((uint64_t)pml4 - (uint64_t)kernbase);

            __asm__ volatile ("movq %0, %%cr3;" :: "r"(pl));
            memcpy((void*)vm->beginAddress,(void*)(eh + ep->p_offset), (uint64_t)(ep->p_filesz));
            memset((void*)(vm->beginAddress + (uint64_t)(ep->p_filesz)), 0, (uint64_t)(ep->p_memsz) - (uint64_t)(ep->p_filesz));
            __asm__ volatile ("movq %0, %%cr3;" :: "r"(pcr3));
        }
    }
    return pml4;
}

uint64_t* gammaFunction(uint64_t fileAddress, task_struct* ts){
    Elf64_Ehdr* eh = (Elf64_Ehdr*)(fileAddress);
    ts->regs.rip = eh->e_entry;
    uint64_t* pml4 = (uint64_t *)(ts->pml4e + kernbase);
    dealloc_pml4((ts->pml4e));

    for(int i=eh->e_phnum;i>0;i--){
        Elf64_Phdr* ep = (Elf64_Phdr*)(fileAddress + (eh->e_phoff));
        ep = ep + (i-1);
        if(ep->p_type == 1){               

            vma* vm = (vma *)kmalloc(sizeof(struct vmaStruct));
            vm->beginAddress = ep->p_vaddr;
            vm->lastAddress = ep->p_vaddr+ep->p_memsz;
            uint64_t k = vm->beginAddress;
            if((((uint64_t)(ep->p_vaddr))% ((uint64_t)pageSize)) != 0){
                k = (uint64_t)((uint64_t)ep->p_vaddr & (uint64_t)0xFFFFFFFFFFFFF000);
            }
            if(ts->vm == NULL){
                vm->next = NULL;
                ts->vm = vm;
            }
            else{
                vm->next = ts->vm;
                ts->vm = vm;
            }
            for(;k<( vm->lastAddress);k+=pageSize){
                uint64_t yy = getFreeFrame();
                init_pages_for_process(k,yy, pml4);
            }

            uint64_t pcr3;
            __asm__ __volatile__ ("movq %%cr3,%0;" :"=r"(pcr3)::);
            uint64_t* pl =( uint64_t* )((uint64_t)pml4 - (uint64_t)kernbase);
            __asm__ volatile ("movq %0, %%cr3;" :: "r"(pl));
            memcpy((void*)vm->beginAddress,(void*)(eh + ep->p_offset), (uint64_t)(ep->p_filesz));
            memset((void*)(vm->beginAddress + (uint64_t)(ep->p_filesz)), 0, (uint64_t)(ep->p_memsz) - (uint64_t)(ep->p_filesz));
            __asm__ volatile ("movq %0, %%cr3;" :: "r"(pcr3));
        }
    }
    return pml4;
}

void setMyVMA(task_struct *ts, uint64_t from, uint64_t toend){
    vma* taskVMA = (vma *) kmalloc(sizeof(struct vmaStruct));
    taskVMA->beginAddress = from;
    taskVMA->lastAddress = toend;
    taskVMA->next = ts->vm;
    ts->vm = taskVMA;
}

void shiftTaskVMA(task_struct *ts, uint64_t from, uint64_t toend){
    vma* vm2 = (vma *)kmalloc(sizeof(struct vmaStruct));
    vm2->beginAddress = from;
    vm2->lastAddress = toend;
    vm2->next = ts->vm;
    ts->vm = vm2;
}

void loadPML4(uint64_t toLoad){
    __asm__ volatile ("movq %0, %%cr3;" :: "r"(( uint64_t*)(toLoad - (uint64_t)kernbase)));
}

void setTaskRSP(char *name, task_struct *ts){
    int len = strlen(name)+1;
    ts->rsp = ts->rsp - len;
    memcpy(ts->rsp,name,len);

    ts->rsp -= 1;
    *(ts->rsp) = 0;

    ts->rsp -= 1;
    *(ts->rsp) = 0;

    (ts->rsp)-=1;
    *(ts->rsp) = (uint64_t)((ts->rsp)+1);

    (ts->rsp)-=1;
    *(ts->rsp) = 0x1;

    r = ts;
}

void create_process(char* filename){
	uint64_t fileAddress = get_file_address(filename) +512;
	if(fileAddress < 512){
		kprintf("No such file\n");
		return;
	}

    char tempFilename[50];
    strcpy(tempFilename,filename);
	int pid = newPID();
	task_struct* ts = (task_struct *) &taskQueue[pid];
    initTaskVariables(ts, filename, pid);

    uint64_t* pml4 = yappaFunction(fileAddress, ts);
    setMyVMA(ts, 0x4B0FFFFF0000, 0x4B0FFFFF0000);
	init_pages_for_process(0x100FFFFF0000,(uint64_t)getFreeFrame(),pml4);
	ts->ustack = (uint64_t*)0x100FFFFF0000;
	ts->rsp = (uint64_t *)((uint64_t)ts->ustack + (510 * 8));
    setMyVMA(ts, 0x100FFFFF0000, 0x100FFEFF0000);
	set_tss_rsp(&(ts->kstack[511]));
    loadPML4((uint64_t)pml4);

    setTaskRSP(tempFilename, ts);

	__asm__ volatile("pushq $0x23;pushq %0;pushf;pushq $0x2B;"::"r"(ts->rsp):"%rax","%rsp");
	__asm__ ("pushq %0"::"r"(ts->regs.rip):"memory");

	__asm__("iretq");
}

void copyVMA(task_struct *curTask, task_struct *copyTask){
    vma* a = curTask->vm;
    vma* p = NULL;
    copyTask->vm = NULL;
    while(a!=NULL){ 
        vma* new = (vma *)kmalloc(sizeof(struct vmaStruct));
        memcpy(new,a,sizeof(struct vmaStruct));
        if(p == NULL){
            p = new;
            copyTask->vm = p;  
        }
        else{
            p->next = new;
            p = new;
        }
        a = a->next;
    }
}

uint64_t getMemorysize(int num){
    return 511*num;
}

void copytask(task_struct* c){
	c->ppid = r->pid;

	c->pml4e = (uint64_t)((uint64_t)getNewPage() - (uint64_t)kernbase);
    memset((uint64_t*)(c->pml4e+(kernbase)),0,pageSize);
	strcpy(c->name,r->name);
    strcpy(c->curr_dir,r->curr_dir);

    copytables(r,c);
	copyVMA(r, c);
}
int fork(){

    int pid = newPID();
	new = (task_struct *) &taskQueue[pid];
	new->pid = pid;
	p = r;
	copytask(new);	

	uint64_t s_add ;
	new->ustack = (uint64_t*)STACK_S;
	new->rsp = (uint64_t *)((uint64_t)STACK_S + 511*8);
	new->state = RUNNING;
	uint64_t pcr3;	
	__asm__ volatile ("movq %%cr3,%0;" :"=r"(pcr3)::);
	__asm__ volatile ("movq %0, %%cr3;" :: "r"(pcr3));	

	r->child_count+=1;
	memcpy(&(new->kstack[0]),&(r->kstack[0]),512*8);
    new->kstack[14] = 9999;
	__asm__ __volatile__(
            "movq 8(%%rsp),%%rax;movq %%rax, %0;"
			:"=g"(new->regs.rip)::"memory","%rax"
			);
	__asm__ __volatile__(
			"movq %%rsp, %0;"
			:"=g"(s_add)::"memory"
			);

    new->regs.rsp = (uint64_t) ((uint64_t)&(new->kstack[511]) -(uint64_t)((uint64_t)&(r->kstack[511]) - (uint64_t)s_add));
    return new->pid;
}

int validPath(char *ref){
    if (ref[0] == '.' && ref[1] == '/') return 1;
    return 0;
}

int setFileAddress(char* path, char *file, uint64_t *fileAddress, task_struct* ts, int *argc, char args[10][80], char *argv[]){
    if(validPath(path)){
        strcpy(file, &(r->curr_dir[1]));
        strcat(file, path+2);
        uint64_t scriptChar = get_file_address(file)+512;
        if(scriptChar<512){
            return -1;
        }
        if( *((char *)scriptChar) == '!' && *((char *)scriptChar+1) == '#') {
            char ex[50];
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

int execvpe(char* path, char *argv[],char* env[]){
	task_struct* ts = r;
    char file[80];
    int argc = 0, envl = 0;
    uint64_t fileAddress = 0 ;
    char args[10][80];
    char envs[40][80]; 
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
/*	Elf64_Ehdr* eh = (Elf64_Ehdr*)(fileAddress);
	ts->regs.rip = eh->e_entry;
	uint64_t* pml4 = (uint64_t *)(ts->pml4e + kernbase);
    dealloc_pml4((ts->pml4e));

	for(int i=eh->e_phnum;i>0;i--){
		Elf64_Phdr* ep = (Elf64_Phdr*)(fileAddress + (eh->e_phoff));
		ep = ep + (i-1);
		if(ep->p_type == 1){               

			vma* vm = (vma *)kmalloc(sizeof(struct vmaStruct));
			vm->beginAddress = ep->p_vaddr;
			vm->lastAddress = ep->p_vaddr+ep->p_memsz;
            uint64_t k = vm->beginAddress;
            if((((uint64_t)(ep->p_vaddr))% ((uint64_t)pageSize)) != 0){
                k = (uint64_t)((uint64_t)ep->p_vaddr & (uint64_t)0xFFFFFFFFFFFFF000);
            }
			if(ts->vm == NULL){
				vm->next = NULL;
				ts->vm = vm;
			}
			else{
				vm->next = ts->vm;
				ts->vm = vm;
			}
			for(;k<( vm->lastAddress);k+=pageSize){
				uint64_t yy = getFreeFrame();
				init_pages_for_process(k,yy, pml4);
			}

			uint64_t pcr3;
			__asm__ __volatile__ ("movq %%cr3,%0;" :"=r"(pcr3)::);
			uint64_t* pl =( uint64_t* )((uint64_t)pml4 - (uint64_t)kernbase);
			__asm__ volatile ("movq %0, %%cr3;" :: "r"(pl));
			memcpy((void*)vm->beginAddress,(void*)(eh + ep->p_offset), (uint64_t)(ep->p_filesz));
			memset((void*)(vm->beginAddress + (uint64_t)(ep->p_filesz)), 0, (uint64_t)(ep->p_memsz) - (uint64_t)(ep->p_filesz));
			__asm__ volatile ("movq %0, %%cr3;" :: "r"(pcr3));
		}
	}*/
    uint64_t* pml4 = (uint64_t*)gammaFunction(fileAddress, ts);

    // vma* vm2 = (vma *)kmalloc(sizeof(struct vmaStruct));
    // vm2->beginAddress = 0x4B0FFFFF0000;
    // vm2->lastAddress = 0x4B0FFFFF0000;
    // vm2->next = ts->vm;
    // ts->vm = vm2;

    shiftTaskVMA(ts, 0x4B0FFFFF0000, 0x4B0FFFFF0000);


    uint64_t s_add = getFreeFrame();
	init_pages_for_process(0x100FFFFF0000,s_add,pml4);
	ts->ustack = (uint64_t*)0x100FFFFF0000;
	ts->rsp = (uint64_t *)((uint64_t)ts->ustack + (510 * 8));
	vma* vm = (vma *)kmalloc(sizeof(struct vmaStruct));
	vm->beginAddress = 0x100FFFFF0000;
	vm->lastAddress = 0x100FFEFF0000;
	vm->next = ts->vm;
	ts->vm = vm;

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
	__asm__ volatile("\
	push $0x23;\
	push %0;\
	pushf;\
	push $0x2B;\
	push %1"::"g"(ts->rsp),"g"(ts->regs.rip):"memory");
	__asm__ volatile("iretq;");
	return 1;
}

void exit(){
    r->state = ZOMBIE;
    for (int i = 0; i < MAX; ++i) {
        if(taskQueue[i].ppid == r->pid){
            taskQueue[i].ppid = 0;
        }
    }
    if(taskQueue[r->ppid].state ==  WAIT){
        taskQueue[r->ppid].state =  RUNNING;
    }
    yield();
}
void removeProcess(int i){
    vma* vm = taskQueue[i].vm;
    while(vm){
        uint64_t k = (uint64_t)vm;
        vm = vm->next;
        free(k-kernbase);
    }
    dealloc_pml4(taskQueue[i].pml4e);
    free(taskQueue[i].pml4e);
}
int wait(){
    while(1) {
        for (int i = 0; i < MAX; ++i) {
            if ((taskQueue[i].ppid == r->pid) && (taskQueue[i].state == ZOMBIE)) {
                removeProcess(i);
                taskQueue[i].state = READY;
                return i;
            }
        }
        r->state = WAIT;
        yield();
        for (int i = 0; i < MAX; ++i) {
            if ((taskQueue[i].ppid == r->pid) && (taskQueue[i].state == ZOMBIE)) {
                removeProcess(i);
                taskQueue[i].state = READY;
                return i;
            }
        }
    }
}
int kill(int pid){
    (taskQueue + pid)->state = ZOMBIE;
    for (int i = 0; i < MAX; ++i) {
        if(taskQueue[i].ppid == pid){
            taskQueue[i].ppid = 0;
        }
    }
    if(taskQueue[(taskQueue + pid)->ppid].state == WAIT){
        taskQueue[(taskQueue + pid)->ppid].state = RUNNING;
    }
    return 1;
}
int waitpid(int pid){
    int i = pid;
    if((taskQueue[i].ppid == r->pid) && (taskQueue[i].state == ZOMBIE)){
        removeProcess(i);
        taskQueue[i].state = READY;
        return i;
    }
    r->state = WAIT;
    while(1){
        yield();
        if((taskQueue[i].ppid == r->pid) && (taskQueue[i].state == ZOMBIE)){
            removeProcess(i);
            taskQueue[i].state = READY;
            return i;
        }
    }
}

pid_t getpid(void){
	return r->pid;
}

pid_t getppid(void){
	return r->ppid;
}

unsigned int sleep(unsigned int seconds){
    if(seconds <= 0){
        return 0;
    }
    r->time = seconds;
    r->state = SUSPENDED;
    yield();
    return 0;
}
int chdir(char* path){
    if(strcmp("../",path) == 0){
        char k[100];
        strcpy(k,path);
        setTruePath(k);
        if(strcmp(k,"")==0){
            strcpy(r->curr_dir,"/");
            return 0;
        }
    }
    if((isValidDirectory(path)) > -1){
        char k[100];
        strcpy(k,path);
        setTruePath(k);
        char l[100];
        l[0] = '/';
        l[1] = '\0';
        strcat(l,k);
        strcpy(r->curr_dir,l);
        return 0;
    }
    return  -1;
}
void getcwd(char *buf, int size){

    strcpy(buf,r->curr_dir);
    int l = strlen(buf);
    if(l == 1){
        return;
    }
    else{
        buf[l-1] = '\0';
    }
}
void* malloc(int no_of_bytes){
    vma* vm = r->vm->next;

    uint64_t ret = vm->lastAddress;
    for(int i =0;i<((no_of_bytes/pageSize))+1;i++){
        uint64_t s_add = getFreeFrame();
        init_pages_for_process(vm->lastAddress,s_add,(uint64_t *)(r->pml4e+kernbase));
        vm->lastAddress = vm->lastAddress + pageSize;
    }
    return (uint64_t*)ret;
}
