#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, char *buf, int count);
unsigned int sleep(unsigned int seconds);
pid_t getTaskPID(void);
pid_t getTaskPPID(void);
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, mode_t mode);ssize_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);
int chdir(const char *path);
char *getCurrentDirectory(char *buf, size_t size);
pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(pid_t pid, pid_t *status);
int pipe(int pipefd[2]);
void ps();
#endif
