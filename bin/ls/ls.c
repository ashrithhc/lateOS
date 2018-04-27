#include <sys/defs.h>
#include <syscall.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#define BUF_SIZE 1024

DIR* openDirectory(int argc, char *argv[]){
    char dir[100];
    if (argc < 2){
        getCurrentDirectory(dir, -1);
        if (dir[1] != '\0') strcat(dir, "/");
        return opendir(dir);
    }
    else {
        strcpy(dir, argv[1]);
        if (dir[strlen(dir)] != '/') strcat(dir, "/");
        return opendir(dir);
    }
    return NULL;
}

int main(int argc, char *argv[], char* envp[])
{
    // char dir[100];
    DIR* dp;
    // if(argc == 1){
    //     getCurrentDirectory(dir,-1);
    //     if(strlen(dir)>1) {
    //         strcat(dir, "/");
    //     }
    //     dp = opendir(dir);
    // }
    // else{
    //     strcpy(dir,argv[1]);
    //     int l = strlen(dir);
    //     if(dir[l-1] != '/'){
    //         strcat(dir,"/");
    //     }
    //     dp = opendir(dir);
    // }

    dp = openDirectory(argc, argv);

    if (dp->fd == -1) {
        puts("Unknown file path passed\n");
        // puts(dir);
        return 0;
    }
    for ( ; ; ) {
        struct dirent *p = readdir(dp);
		if (p == NULL)
			break;
        puts(p->d_name);
        puts(" ");
    }
    puts("\n");
    close(dp->fd);
    return 0;
}
