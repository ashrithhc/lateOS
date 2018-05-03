#ifndef _TERMINAL_H
#define _TERMINAL_H

void intWrite();
void clrscr();
int getoffset();
void setoffset(int i);
void wake_process();
void read_input(char* b);
#endif

