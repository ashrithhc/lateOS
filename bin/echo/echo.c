#include <sys/defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[])
{
	if (argv[1] == NULL) {
	    puts("\n");
		return 0;
	}

	for(int i=1; i<argc; i++)
	{
		puts(argv[i]);
        puts(" ");
	}
    puts("\n");
	return 0;
}
