#ifndef _TERMINAL_H
#define _TERMINAL_H

void intWrite();
void clearScreen();
int getoffset();
void setoffset(int i);
void wake_process();
void intRead(char* b);
#endif

