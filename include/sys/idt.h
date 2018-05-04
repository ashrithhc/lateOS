#ifndef _IDT_H_
#define _IDT_H_
#include <sys/defs.h>

void intWrite();
void clearScreen();
int getoffset();
void setoffset(int i);
void wake_process();
void intRead(char* b);
void IDTinitialise();
extern void startTimer();
void outportb(uint16_t, uint8_t);
extern void intTimer();
pid_t wait(int *status);

struct idt{
	uint16_t lower_offset;
	uint16_t selector;
	uint8_t zero_1;
	uint8_t type;
	uint16_t mid_offset;
	uint32_t high_offset;
	uint32_t zero_2;
}__attribute__((packed));

struct idt_ptr{
	uint16_t size;
	uint64_t base;
}__attribute__((packed));

#endif
