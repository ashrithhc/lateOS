#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/mem.h>
#include <sys/process.h>
#include <sys/file.h>
#include <sys/terminal.h>

#define idtLowerMask 0xFFFF
#define idtMidMask 0xFFFF
#define idthighMask 0xFFFFFFFF
#define timerMask 0xFF

void isr_0();
void isr_14();
void isr_128();
extern void timer();
extern void kb1();

static struct idt IDTset[256];
static struct idt_ptr IDTloader;

static inline uint8_t inportb(uint64_t port)
{
        uint8_t retVal;
        __asm__ __volatile__("inb %1, %0" : "=a"(retVal) : "Nd"(port));  
        return retVal;
}

void outportb(uint16_t port, uint8_t data){
	__asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
}

void setIDTtype(uint16_t interrupt){
	if(interrupt == 128){
		(&(IDTset[interrupt]))->type = 0xEE;
	}
	else{
		(&(IDTset[interrupt]))->type = 0x8E;
	}
}

void initISR(uint16_t interrupt, uint64_t function)
{
	(&(IDTset[interrupt]))->selector  = 0x08;
	(&(IDTset[interrupt]))->lower_offset = function & idtLowerMask ;
	(&(IDTset[interrupt]))->mid_offset = (function >> 16) & idtMidMask;
	(&(IDTset[interrupt]))->high_offset = (function >> 32) & idthighMask;
	(&(IDTset[interrupt]))->zero_1 = 0;
	(&(IDTset[interrupt]))->zero_2 = 0;
	setIDTtype(interrupt);
}

void loadIDT(){
	(&IDTloader)->size = (sizeof(struct idt) * 256) - 1 ;
	(&IDTloader)->base = (uint64_t)IDTset;
	__asm__ __volatile__ ("lidt (%0)" : : "r"(&IDTloader));
}

void startTimer(){
    uint8_t lobyte = (uint8_t)(0x4A9 & timerMask);
    uint8_t hibyte = (uint8_t)((0x4A9 >> 8) & timerMask);
    outportb(0x40, lobyte);
    outportb(0x40, hibyte);
}

void init_idt(){
	for (int i=0; i<32; i++) initISR(i, (uint64_t)&isr_0);
	initISR(14, (uint64_t)&isr_14);
	initISR(32, (uint64_t)&timer);
	initISR(33, (uint64_t)&kb1);
	initISR(128, (uint64_t)&isr_128);
	loadIDT();
}

void isr0(){
	kprintf("Invalid Interrupt received");
	outportb(0x20,0x20);
}

void isr14(){

	uint64_t bb;
    int flag = 0;
	__asm__ __volatile__("movq %%cr2,%0;":"=g"(bb)::);
    vmaStruct* vm = currentTask->vm;
    if(((vm->beginAddress + 4096) > bb) && (vm->lastAddress < bb)){
        flag =1;
    }
    if(flag == 0) {
        while (vm != NULL) {
            if ((vm->beginAddress < bb) && (vm->lastAddress > bb)) {
                flag = 1;
                break;
            }
            vm = vm->next;
        }
    }
    if(flag == 0){
       // kprintf("New allocated stack at %p - range %p,%p \n",bb,currentTask->vm->beginAddress+4096,currentTask->vm->lastAddress);

        kprintf("Segmentation Fault: Address:%p \n",bb);
            exit();
            while(1);
    }
    uint64_t* k = getPTE(bb);
    if( k[(bb>>12)&0x1FF] & 0x0000000000000200){     //COW
        uint64_t k1;
        __asm__ __volatile__("movq %%cr3,%0;":"=g"(k1)::);
        uint64_t  add = (k[(bb>>12)&0x1FF] & 0xFFFFFFFFFFFFF000);
        int i = 2;//getrefcount(add);
        if(i == 2){
            uint64_t p_n = getFreeFrame();
            memcpy((void*)(0xffffffff80000000 + p_n),(void *)(bb&0xFFFFFFFFFFFFF000),4096);
            switchtokern();
            init_pages_for_process((bb&0xFFFFFFFFFFFFF000),p_n,(uint64_t *)(currentTask->pml4e + 0xffffffff80000000));
            free(add);
        } else if(i == 1){
            k[(bb>>12)&0x1FF] = (k[(bb>>12)&0x1FF] | 0x2) & 0xFFFFFFFFFFFFFDFF;
        } else{
            kprintf("Should never be here\n");
            while(1);
        }
        __asm__ __volatile__("movq %0,%%cr3;"::"r"(k1):);
	}
	else if( (currentTask->vm->beginAddress > bb)  && (currentTask->vm->lastAddress < bb)){   //Auto Growing stack
        uint64_t k;
		__asm__ __volatile__("movq %%cr3,%0;":"=g"(k)::);
		//uint64_t n_s = currentTask->vm->beginAddress - 4096;
		uint64_t p_n = getFreeFrame();
		switchtokern();
		init_pages_for_process((bb&0xFFFFFFFFFFFFF000),p_n,(uint64_t *)(currentTask->pml4e + 0xffffffff80000000));
	//	currentTask->vm->beginAddress = n_s;
		 __asm__ __volatile__("movq %0,%%cr3;"::"r"(k):);
//		while(1);
	}
    else{
        kprintf("Segmentation Fault: Address:%p \n",bb);
        exit();
        while(1);
    }

}
typedef struct registers_t{
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx;
}registers_t;

/*uint64_t isr128(){
	uint64_t cval;
	 uint64_t as,ret = 0;
	__asm__ __volatile__("movq %%rax,%0;":"=g"(cval)::"memory","r15","rax");
	__asm__ __volatile__("movq %%rdi,%0;":"=g"(as)::"memory","rdi","rax");
	registers_t *y = (registers_t *)as;

    	if(cval == 0 && y->rbx == 0){
        	read_input((char *)y->rcx);
    	}
    	else if(cval == 1 && y->rbx == 1){ //This is a write syscall to stdout
		kprintf("%s",y->rcx);
	}
	else if(cval == 57){
		ret = (uint64_t)fork();
        if((uint64_t)currentTask->kstack[14] == 9999){
            ret = 0;
            currentTask->kstack[14] = 0;
            __asm__ __volatile__("popq %%rax":::"%rax");
            __asm__ __volatile__("popq %%rax":::"%rax");
        }
	}
	else if(cval == 59){
		ret = execvpe((char *)y->rbx,(char **)y->rcx,(char **) y->rdx);
	}
	else if(cval == 0){
		ret = read_tarfs((int) y->rbx, (char*) y->rcx, (int) y->rdx);			
	}
	else if(cval == 2){
		ret = open_tarfs((char*) y->rbx, (int) y->rcx);
	}else if(cval == 3) {
            ret = close_tarfs((int)y->rbx);
    }
   else if(cval == 16){
        ret =  open_dir((char *)y->rbx);
    }
   else if(cval == 78){
		ret = readdir_tarfs((int) y->rbx, (char *) y->rcx);
   }
    	else if(cval == 60){
      	 	exit();
    	}
    	else if(cval == 247){
        	ret = (uint64_t)waitpid((int)y->rbx);
    	}
	else if(cval == 39){
		ret = getTaskPID();
	}
	else if(cval == 110){
		ret = getTaskPPID();
	}
    else if(cval == 7){
            clrscr();
    }
    else if(cval == 62){
       ret = kill((int)y->rbx);
    }
    else if(cval == 35){
            ret = sleep((int)y->rbx);
    }
    else if(cval == 79){
            getCurrentDirectory((char*)y->rbx,(int)y->rcx);
    }
    else if(cval == 80){
            ret = chdir((char*)y->rbx);
    }
	else if(cval == 299){
		ps();
	}else if(cval == 9){
            ret = (uint64_t) malloc((int)y->rbx);
        }
	// schedule();
	outportb(0x20,0x20);
	return ret;
}*/

uint64_t isr128(){
    uint64_t cval;
    uint64_t as, retVal = 0;
    __asm__ __volatile__("movq %%rax,%0;":"=g"(cval)::"memory","r15","rax");
    __asm__ __volatile__("movq %%rdi,%0;":"=g"(as)::"memory","rdi","rax");
    registers_t *y = (registers_t *)as;
    
    switch (cval){
        case 0:     if (y->rbx == 0){
                        read_input((char *)y->rcx);
                        retVal = 0;
                    }
                    else {
                        retVal = read_tarfs((int) y->rbx, (char*) y->rcx, (int) y->rdx);
                    }
        case 1:     if (y->rbx == 1){
                        kprintf("%s", y->rcx);
                        retVal = 0;
                    }
        case 2:     retVal = open_tarfs((char*) y->rbx, (int) y->rcx);
        case 3:     retVal = close_tarfs((int)y->rbx);
        case 7:     clrscr();
        case 9:     retVal = (uint64_t) malloc((int)y->rbx);
        case 16:    retVal = open_dir((char *)y->rbx);
        case 35:    retVal = sleep((int)y->rbx);
        case 39:    retVal = getTaskPID();
        case 57:    retVal = (uint64_t)fork();
                    if((uint64_t)currentTask->kstack[14] == 9999){
                        retVal = 0;
                        currentTask->kstack[14] = 0;
                        __asm__ __volatile__("popq %%rax":::"%rax");
                        __asm__ __volatile__("popq %%rax":::"%rax");
                    }
        case 59:    retVal = execvpe((char *)y->rbx,(char **)y->rcx,(char **) y->rdx);
        case 60:    exit();
        case 62:    retVal = kill((int)y->rbx);
        case 78:    retVal = readdir_tarfs((int) y->rbx, (char *) y->rcx);
        case 79:    getCurrentDirectory((char*)y->rbx,(int)y->rcx);
        case 80:    retVal = chdir((char*)y->rbx);
        case 110:   retVal = getTaskPPID();
        case 247:   retVal = (uint64_t) waitpid((int)y->rbx);
        case 299:   ps();
    }
    outportb(0x20,0x20);
    return retVal;
}