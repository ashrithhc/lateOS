#include <sys/process.h>
#include <sys/gdt.h>
#include <sys/mem.h>
#include <sys/kprintf.h>
int np = 0;



void switchRSP(taskStruct *finalTask){
	__asm__ __volatile__("movq %%rsp, %0;" : "=g"() : : "memory");
	__asm__ __volatile__("movq %0, %%rsp;" : : "r"() : "memory");
}






