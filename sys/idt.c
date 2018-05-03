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
#define PTEmask 0x1FF
#define kernmem 0xffffffff80000000
#define pageMask 0xFFFFFFFFFFFFF000

typedef struct registersAligned{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx;
}registersAligned;

void ISR0();
void ISR14();
void ISR128();
extern void timer();
extern void keyboard();

static struct idt IDTset[256];
static struct idt_ptr IDTloader;
static int timerCount = 0, totalSecs = 0;

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

void checkValid(uint64_t vmem){
    int isValid = 0;
    vmaStruct* taskVMA = currentTask->vm;

    if(((taskVMA->beginAddress + 0x1000) > vmem) && (taskVMA->lastAddress < vmem)) isValid = 1;
    else isValid = checkForVM(taskVMA, vmem);

    if(isValid == 0) exit();
}

void startTimer(){
    uint8_t lobyte = (uint8_t)(0x4A9 & timerMask);
    uint8_t hibyte = (uint8_t)((0x4A9 >> 8) & timerMask);
    outportb(0x40, lobyte);
    outportb(0x40, hibyte);
}

void IDTinitialise(){
	for (int i=0; i<32; i++) initISR(i, (uint64_t)&ISR0);
	initISR(14, (uint64_t)&ISR14);
	initISR(32, (uint64_t)&timer);
	initISR(33, (uint64_t)&keyboard);
	initISR(128, (uint64_t)&ISR128);
	loadIDT();
}

void isr0(){
	kprintf("Invalid Interrupt received");
	outportb(0x20,0x20);
}

uint64_t getCR2(){
    uint64_t retVal;
    __asm__ __volatile__("movq %%cr2, %0;" : "=g"(retVal) : : );
    return retVal;
}

void loadPML4(uint64_t addr){
    __asm__ __volatile__ ("movq %0, %%cr3;" : : "r"(addr) : );
}

uint64_t getPML4(){
    uint64_t retVal;
    __asm__ __volatile__ ("movq %%cr3, %0;" : "=g"(retVal) : : );
    return retVal;
}

void copyNewMemory(uint64_t vaddr, uint64_t addr){
    uint64_t copyFrame = getFreeFrame();
    memcpy((void*)(kernmem + copyFrame), (void *)(vaddr & pageMask), 0x1000);
    switchtokern();
    init_pages_for_process((vaddr & pageMask), copyFrame, (uint64_t *)(currentTask->pml4e + kernmem));
    free(addr);
}

void copyFromPML4(uint64_t vaddr){
    uint64_t taskPML4 = getPML4();
    uint64_t copyFrame = getFreeFrame();
    switchtokern();
    init_pages_for_process((vaddr & pageMask), copyFrame, (uint64_t *)(currentTask->pml4e + kernmem));
    loadPML4(taskPML4);
}

void isr14(){
	uint64_t vaddr = getCR2();
    checkValid(vaddr);
    uint64_t *taskPTE = getPTE(vaddr);

    uint64_t pteOffset = taskPTE[(vaddr >> 12) & PTEmask];
    if (pteOffset & 512){
        uint64_t taskPML4 = getPML4();
        copyNewMemory(vaddr, (pteOffset & pageMask));
        loadPML4(taskPML4);
	}
	else if( (currentTask->vm->beginAddress > vaddr)  && (currentTask->vm->lastAddress < vaddr)) copyFromPML4(vaddr);
    else{
        kprintf("Segmentation Fault\n");
        exit();
    }
}

uint64_t isr128(){
    uint64_t syscode, regsValue, retVal = 0;
    __asm__ __volatile__("movq %%rax, %0;" : "=g"(syscode) : : "memory", "r15", "rax");
    __asm__ __volatile__("movq %%rdi, %0;" : "=g"(regsValue) : : "memory", "rdi", "rax");
    registersAligned *regs = (registersAligned *)regsValue;
    
    switch (syscode){
        case 0:     if (regs->rbx == 0){
                        intRead((char *)regs->rcx);
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
        case 7:     clearScreen(); break;
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

void intTimer(){
    __asm__ __volatile__("cli");
    outportb(0x20, 0x20);
    timerCount++;
    if(timerCount == 5){
        schedule();
        totalSecs++;
        int temp = totalSecs;
        char *reg = (char*)0xffffffff800B8F9E;
        while(temp>0)
        {
            *reg = '0'+temp%10; 
            temp = temp/10;
            reg-=2;         
        }
        timerCount = 0;
        for(int i=0;i<MAX;i++){
            if(taskQueue[i].state == HANG && taskQueue[i].time >0){
                taskQueue[i].time--;
            }
            if(taskQueue[i].time == 0 && taskQueue[i].state == HANG){
                taskQueue[i].state = RUNNING;
            }
        }
    }
}