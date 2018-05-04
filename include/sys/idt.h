#ifndef _IDT_H_
#define _IDT_H_
#include <sys/defs.h>

struct IDTtable{
	uint16_t lower_offset;
	uint16_t selector;
	uint8_t zero_1;
	uint8_t type;
	uint16_t mid_offset;
	uint32_t high_offset;
	uint32_t zero_2;
}__attribute__((packed));

struct IDTaddr{
	uint16_t size;
	uint64_t base;
}__attribute__((packed));

void IDTinitialise();
extern void startTimer();
void outportb(uint16_t, uint8_t);
extern void intTimer();

#endif
