#ifndef _FILE_H
#define _FILE_H
#include<sys/defs.h>

typedef struct file_t {
	int fd;
	char file_name[50];
    int aval;
	off_t offset;
	int entry;
	uint64_t flags; 
	uint64_t inode_num;
	uint64_t size;
	uint64_t address;
} file_t;

typedef struct {
	uint64_t size;
	uint64_t magic_no;
	uint64_t inode_start;
	uint64_t block_start;
	uint64_t inode_max;
	uint64_t block_max;
	uint64_t inode_free;
	uint64_t block_free;
} superblock;

typedef struct {
	char file_name[100];
	uint64_t file_size;
	uint64_t block_count;
} inode;

int openDirFile(char* path);
int openTarfsFile(char* file_path, int flags);
ssize_t readTarfsFile(int fd, char* buf, int count);
uint64_t toBinaryFile(const char* octal);
int readDirTarfsFile(int fd, char* buf);
int closeTarfsFile(int fp);

#endif
