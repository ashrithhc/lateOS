#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <sys/wait.h>

static int isBackground = 0;
static char input[1000]={'\0'};
static char pwd[100];
static char com[1000]={'\0'};
static char arg[1000][1000]={'\0'};
static char prompt[100]={'\0'};

static char cm1[20][100];
static char *cm[20];
int readstring(int fd,char* buf,int size){
    long retVal;
    __asm__ volatile ("movq %1, %%rax; movq %2, %%rbx; movq %3, %%rcx; movq %4, %%rdx; int $0x80; movq %%rax, %0;" : "=m" (retVal) : "g"(0), "r"((long)(fd)), "r"((long)(buf)), "r"((long)(size)) : "rax", "memory", "rbx", "rcx", "rdx");
    return (int)(retVal);
}

void setprompt(char *sbush){
    getCurrentDirectory(pwd, -1);
    strcpy(prompt, sbush);
    strcat(prompt, ":");
    strcat(prompt, pwd);
    strcat(prompt, ">");
}

void makeNullExecvp(char* strs[], int index, int j){
    if(j == 0) strs[index] = NULL;
    strs[index+1] = NULL; //Because execvp expects a NULL pointed args[] as the end
}

void changeDirectory(char **args){
    if(args[1] == NULL) puts("Usage : cd <path>\n");
    else if(chdir(args[1]) != 0) puts("Invalid path\n");
    setprompt(prompt);
}

int skipspaces(char* str, int k){
    while(str[k]!='\n'){
        if( k == 0 && str[k] == ' '){
            k++;
            continue;
        }
        else break;
    }
    return k;
}

int isBackgroundTask(char* strs[], int index){
    if(strcmp(strs[index], "&") == 0) return 1;
    return 0;
}

void parseNEWprompt(char* str, char delimiter, char* strs[]){
    int i, j, k;
    i=j=k=0;
    while(str[k] != '\n'){
        k = skipspaces(str, k);
        if(str[k] == '"' && delimiter == '='){
            k++;
            continue;
        }
        strs[i][j++]=str[k];
        ++k;
    }
}

static char *in = &input[0] ;
static char *args[1000] ;
static char *command = &com[0];
static char *envpe[100];

void clearArguments(){
    for(int i=0; i<1000; i++){
        args[i] = &arg[i][0];
        for(int j=0; j<1000; j++) arg[i][j] = '\0' ;
    }
}

/* Implemented based on : https://stackoverflow.com/questions/28931379/implementation-of-strtok-function*/
void strtokBeta(char* str, char delimiter, char* strs[]){
    int i, j, k;
    i=j=k=0;
    while(str[k] != '\n'){
        k = skipspaces(str, k);
        if(str[k] == delimiter || str[k] == '\0'){
            strs[i][j]='\0';
            isBackground = isBackgroundTask(strs, i);
            if (isBackground) strs[i] = NULL;
            if(str[k]=='\0'){
                makeNullExecvp(strs, i, j);
                return;
            }
            i++; j=0;
        }
        else{
            if(str[k] == '"' && delimiter == '='){
                k++;
                continue;
            }
            strs[i][j++]=str[k];
        }
        k++;
    }
}

void changeSbush(char *args[]){
    char temp[10][1000];
    char *splitStrings[3];
    splitStrings[0] = &temp[0][0];
    splitStrings[1] = &temp[1][0];
    splitStrings[2] = &temp[2][0];
    strtokBeta(args[1], '=', splitStrings);
    if(strcmp("PS1", splitStrings[0]) == 0) setprompt(splitStrings[1]);
}

void execCommand(char* command,char* ag[]){
    pid_t pid;
    if ((pid = fork()) == 0) {
        if(execvp(command, ag,envpe) == -1){
            puts("-sbush: ");
            puts(command);
            puts(": No such command. \n");
        }
        exit(1);
    }
    else if (pid < 0) {
        puts("Failed to fork\n");
    }
    else {
        if(isBackground == 1){
            return;
        }
        waitpid(pid,&pid);
    }
}

void readInput(){
    in = &input[0];
    gets(in);
}

void getPWD(){
    getCurrentDirectory(pwd, -1);
    puts(pwd);
    puts("\n");
}

void parseInput(){
    strtokBeta(in,'|',args);
    clearArguments();
    strtokBeta(in,' ',args);
    command = args[0];
    if(strcmp(command, "cd") == 0) changeDirectory(args);
    else if(strcmp(command, "export") == 0) changeSbush(args);
    else if(strcmp(command, "clear") == 0) clrscr();
    else if(strcmp(command, "pwd") == 0) getPWD();
    else execCommand(command,args);
}

int getInputArgCounts(){
    int i=0;
    while(args[i]!=NULL)
    {
        i=i+1;
    }
    return i;
}


void clearInput(){
    for(int i =0;i<1000;i++) {
        input[i] = '\0';
    }
}

void clearCommand(){
    for(int i=0;i<sizeof(com)/sizeof(char);i++){
        com[i] = '\0';
    }
}

void setenvs(){
    for(int i=0; i < getenvlength(); i++) envpe[i] = getallenv(i);
}

int main(int argc, char *argv[], char *envp[]) {
    for(int i=0;i<1000;i++){
        args[i] = &arg[i][0];
    }
    setprompt("sbush");
    setenvs();
    if(argc==1){
        clrscr();
        while(1){
            puts(prompt);
            readInput();
            if(input[0] == '\0'){
                continue;
            }
            if((strcmp(in,"exit") == 0) || (strcmp(in,"quit") == 0)){
                return 0;
            }
            parseInput();
            //execCommand();

            clearInput();
            clearCommand();
            clearArguments();
        }
    }else{
        int fp;
        fp = fopen(argv[1],"r");
        if(fp == -1){
            puts("File not found, Exiting!");
            return 0;
        }
        char buf[4096];
        readstring(fp,buf,4096);
        int c = 0,l =0;
        for(int i=0;i<20;i++){
            cm[i] = &cm1[i][0];
        }
        int len = strlen(buf);
        in = &input[0];
        for(int i =0;i<len;i++){
            if(*(buf+i) =='\n'){
                cm[c][l++] = '\0';
                c++;
                l =0;
                continue;
            }
            cm[c][l++] = *(buf+i);
        }
        for(int k =0;k<c;k++){
                strcpy(in,cm[k]);
                parseInput();
            //    i=0;
                clearInput();
                clearCommand();
                clearArguments();
        }
    }
    return 0;
}
