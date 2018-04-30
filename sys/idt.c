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

typedef struct registersAligned{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx;
}registersAligned;

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

int checkForVM(vmaStruct *vm, uint64_t cr2Val){
    int flag = 0;
    while (vm != NULL) {
        if ((vm->beginAddress < cr2Val) && (vm->lastAddress > cr2Val)) {
            flag = 1;
            break;
        }
        vm = vm->next;
    }
    return flag;
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
	uint64_t cr2Val;
    int isValid = 0;
	__asm__ __volatile__("movq %%cr2, %0;" : "=g"(cr2Val) : : );

    vmaStruct* vm = currentTask->vm;
    if(((vm->beginAddress + 0x1000) > cr2Val) && (vm->lastAddress < cr2Val)) isValid = 1;
    else isValid = checkForVM(vm, cr2Val);
    if(isValid == 0) exit();

    uint64_t* k = getPTE(cr2Val);
    if( k[(cr2Val>>12)&0x1FF] & 0x0000000000000200){     //COW
        uint64_t k1;
        __asm__ __volatile__("movq %%cr3,%0;":"=g"(k1)::);
        uint64_t  add = (k[(cr2Val>>12)&0x1FF] & 0xFFFFFFFFFFFFF000);
        int i = 2;//getrefcount(add);
        if(i == 2){
            uint64_t p_n = getFreeFrame();
            memcpy((void*)(0xffffffff80000000 + p_n),(void *)(cr2Val&0xFFFFFFFFFFFFF000),0x1000);
            switchtokern();
            init_pages_for_process((cr2Val&0xFFFFFFFFFFFFF000),p_n,(uint64_t *)(currentTask->pml4e + 0xffffffff80000000));
            free(add);
        } else if(i == 1){
            k[(cr2Val>>12)&0x1FF] = (k[(cr2Val>>12)&0x1FF] | 0x2) & 0xFFFFFFFFFFFFFDFF;
        } else{
            kprintf("Should never be here\n");
            while(1);
        }
        __asm__ __volatile__("movq %0,%%cr3;"::"r"(k1):);
	}
	else if( (currentTask->vm->beginAddress > cr2Val)  && (currentTask->vm->lastAddress < cr2Val)){   //Auto Growing stack
        uint64_t k;
		__asm__ __volatile__("movq %%cr3,%0;":"=g"(k)::);
		uint64_t p_n = getFreeFrame();
		switchtokern();
		init_pages_for_process((cr2Val&0xFFFFFFFFFFFFF000),p_n,(uint64_t *)(currentTask->pml4e + 0xffffffff80000000));
		 __asm__ __volatile__("movq %0,%%cr3;"::"r"(k):);
	}
    else{
        kprintf("Segmentation Fault: Address:%p \n",cr2Val);
        exit();
        while(1);
    }

}

uint64_t isr128(){
    uint64_t syscode, regsValue, retVal = 0;
    __asm__ __volatile__("movq %%rax, %0;" : "=g"(syscode) : : "memory", "r15", "rax");
    __asm__ __volatile__("movq %%rdi, %0;" : "=g"(regsValue) : : "memory", "rdi", "rax");
    registersAligned *regs = (registersAligned *)regsValue;
    
    switch (syscode){
        case 0:     if (regs->rbx == 0){
                        read_input((char *)regs->rcx);
                        retVal = 0;
                    }
                    else {
                        retVal = read_tarfs((int) regs->rbx, (char*) regs->rcx, (int) regs->rdx);
                    }
                    break;
        case 1:     if (regs->rbx == 1){
                        kprintf("%s", regs->rcx);
                        retVal = 0;
                    }
                    break;
        case 2:     retVal = open_tarfs((char*) regs->rbx, (int) regs->rcx);
                    break;
        case 3:     retVal = close_tarfs((int)regs->rbx); break;
        case 7:     clrscr(); break;
        case 9:     retVal = (uint64_t) malloc((int)regs->rbx); break;
        case 16:    retVal = open_dir((char *)regs->rbx); break;
        case 35:    retVal = sleep((int)regs->rbx); break;
        case 39:    retVal = getTaskPID(); break;
        case 57:    retVal = (uint64_t)fork();
                    if((uint64_t)currentTask->kstack[14] == 9999){
                        retVal = 0;
                        currentTask->kstack[14] = 0;
                        __asm__ __volatile__("popq %%rax":::"%rax");
                        __asm__ __volatile__("popq %%rax":::"%rax");
                    }
                    break;
        case 59:    retVal = execvpe((char *)regs->rbx,(char **)regs->rcx,(char **) regs->rdx); break;
        case 60:    exit(); break;
        case 62:    retVal = kill((int)regs->rbx); break;
        case 78:    retVal = readdir_tarfs((int) regs->rbx, (char *) regs->rcx); break;
        case 79:    getCurrentDirectory((char*)regs->rbx,(int)regs->rcx); break;
        case 80:    retVal = chdir((char*)regs->rbx); break;
        case 110:   retVal = getTaskPPID(); break;
        case 247:   retVal = (uint64_t) waitpid((int)regs->rbx); break;
        case 299:   ps(); break;
        default:    retVal = 0; break;
    }
    outportb(0x20, 0x20);
    return retVal;
}