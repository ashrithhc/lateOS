#include <sys/defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int validateARGS(char *str){
	if (str == NULL){
		puts("Usage : echo <type-something>\n");
		return -1;
	}
	return 0;
}

void printEmAll(int argc, char *argv[]){
	for (int index = 1; index < argc; index++){
		strcat(argv[index], " ");
		puts(argv[index]);
	}
	puts("\n");
}

int main(int argc, char* argv[], char* envp[])
{
	if (validateARGS(argv[1]) == -1) return -1;
	printEmAll(argc, argv);
	return 0;
}
