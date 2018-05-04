#include <sys/defs.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define True 1

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

void printList(DIR *dirPointer){
    while(True){
        struct dirent *file = readdir(dirPointer);
        if (file == NULL) break;
        puts(file->d_name);
        puts(" ");
    }
    puts("\n");
}

int main(int argc, char *argv[], char* envp[])
{
    DIR* dirPointer;
    dirPointer = openDirectory(argc, argv);

    if (dirPointer->fd == -1) {
        puts("Incorrect parameter\n");
        return 0;
    }

    printList(dirPointer);
    close(dirPointer->fd);
    return 0;
}
