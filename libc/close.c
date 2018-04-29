#include "../include/syscall.h"
#include <stdio.h>
int close(int fd)
{
	_syscall1(int, close, fd);
}
int fclose(int fd)
{
	return 	close(fd);
}
