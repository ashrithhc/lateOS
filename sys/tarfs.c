#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/elf64.h>
#include <sys/file.h>
#include <sys/string.h>
#include <sys/tarfs.h>
#include <sys/process.h>
#include <sys/mem.h>

#define True 1
#define False 0
#define Faalse -1

static struct posix_header_ustar *headers[200];
static int filecount = 0;

unsigned int getsize(const char *headerSize)
{
    int size = 0, count = 1;
    for (int j = (512 - 501); j > 0; j--){
        size += ((headerSize[j-1] - '0') * count);
        count *= 8;
    }
    return (unsigned int)size;
}

uint64_t get_file_address(char* filename){
    for(int i=0; i<filecount; i++){
        struct posix_header_ustar *pheader = headers[i];
        if(strcmp(filename, pheader->name) == 0) return (uint64_t)headers[i];
    }
    return -1;
}

char* moveTarfsHeader(char *tarfsAddr, struct posix_header_ustar *header){
    unsigned int size = getsize(header->size);
    tarfsAddr += ((size / 512) + 1) * 512;
    if (size % 512) tarfsAddr += 512;
    return tarfsAddr;
}

void init_tarfs()
{
    struct posix_header_ustar *header = (struct posix_header_ustar *)&_binary_tarfs_start;
    char* tarfsAddr = &_binary_tarfs_start;
    while(tarfsAddr < &_binary_tarfs_end){
        headers[filecount] = header;
        tarfsAddr = moveTarfsHeader(tarfsAddr, header);
        header = (struct posix_header_ustar *)tarfsAddr;
        filecount = filecount+1;
    }
}

int previousDir(char path[100], int index){
    if ((*(path + index) == '.') && (*(path + (index+1))) == '.') return True;
    return False;
}

int getOffset(char *path, int index){
    if (*(path + index) != '\0') return 0;
    return 0;
}

void deriveRelative(char* absPath){
    int index;
    char file_path[100];
    if( (*absPath) != '/') {
        strcpy(file_path, &(currentTask->curr_dir[1]));
        strcat(file_path, absPath);
    }
    else strcpy(file_path, absPath+1);
    resetString(absPath);
    int pathOffset = getOffset(absPath, 0);
    for(index = 0; *(file_path+index) != '\0'; index++)
    {
        if(previousDir(file_path, index))
        {
            resetString(absPath + --pathOffset);
            while( (*(absPath+pathOffset)!='/') && pathOffset>=0) resetString(absPath + pathOffset--);
            index++;
        }
        else *(absPath + pathOffset++) = *(file_path + index);
    }
    *(absPath+pathOffset) = *(file_path+index);
    resetString(absPath+pathOffset + 1);
}

int checkFile(char *path){
    for(int fileID=0; fileID < 32; fileID++)
    {
        if (headers[fileID] == NULL) break;
        if(strcmp(headers[fileID]->name, path) == 0) return fileID;
    }
    return Faalse;
}

int isfileexists(char* path){
    char fpath[100];
    strcpy(fpath,path);
    deriveRelative(fpath);
    char* absolutePath = &fpath[0];
    return checkFile(absolutePath);
}

int setCurrentTaskVal(int setflag, int fileID, int flags){
    int fileCounter = currentTask->fd_c + 3;
    currentTask->fd_c++;
    strcpy(&((&(currentTask->fd[fileCounter]))->file_name[0]), headers[fileID]->name);
    (&(currentTask->fd[fileCounter]))->entry = 0;
    (&(currentTask->fd[fileCounter]))->aval = 1;
    (&(currentTask->fd[fileCounter]))->size = (uint64_t)(octal_to_binary((char*)(headers[fileID]->size)));
    (&(currentTask->fd[fileCounter]))->address = (uint64_t)headers[fileID];
    (&(currentTask->fd[fileCounter]))->fd = fileCounter;
    if (setflag != 10)(&(currentTask->fd[fileCounter]))->flags = flags;
    return fileCounter;
}

int isValidDirectory(char* path){
    struct posix_header_ustar* fileheader;
    if(isfileexists(path) == -1) return -1;
    fileheader = headers[isfileexists(path)];
    if(fileheader->name[strlen(fileheader->name)-1] == '/')return isfileexists(path);
    return Faalse;
}

int open_dir(char* path){
    if(strcmp(path, "/")==0)
    {
	    int fdc = currentTask->fd_c+3;
        currentTask->fd_c++;
        (&(currentTask->fd[fdc]))->entry = 0;
        strcpy(&((&(currentTask->fd[fdc]))->file_name[0]),"/");
        return fdc;
    }
    int fileID = isValidDirectory(path);
    if(fileID == -1) return fileID;
    return setCurrentTaskVal(10, fileID, -1);
}

int open_tarfs(char* path, int flags)
{
        int fileID = isfileexists(path);
        if(fileID == -1) return fileID;
        return setCurrentTaskVal(2, fileID, flags);
}

void setFilePath(char *path, int count, struct posix_header_ustar *header){
    for(int i=0; i<count; i++) path[i] = *((char *)header + 1 + i);
    path[count] = '\0';
}

ssize_t read_tarfs(int fd, char* buf, int count)
{
    if ((&(currentTask->fd[fd]))->aval == 0) return -1;
    if (count == 0) return 0;
	
	struct file_t* filedescriptor = (struct file_t*) &(currentTask->fd[fd]);
    struct posix_header_ustar *header = (struct posix_header_ustar*) filedescriptor->address;;
    // header = 
    // char* start_address = (char *) (header+1);
    if((filedescriptor->size) < count) count = filedescriptor->size;
    setFilePath(buf, count, header);
	return count;
}

int readdir_tarfs(int fd, char* buf)
{
    int ret = 0;
    int i=0;
    int count = 0;
    char* dir_name = currentTask->fd[fd].file_name;
    for(i=0; i<32 && headers[i]!=NULL; i++)
    {
        int index = starts_with(headers[i]->name,dir_name);
        int k = strcmp(dir_name,"/");
        if((index>0) || k==0) {
            if(count==0)
            {
                if(currentTask->fd[fd].entry==0)
                {
                    currentTask->fd[fd].entry++;
                }
            }
            else if(count==currentTask->fd[fd].entry)
            {
                strcpy(buf,substring(headers[i]->name,index));
                currentTask->fd[fd].entry++;
                ret = 1;
                break;
            }
            count++;
        }
    }
	return ret;
}

int close_tarfs(int fp)
{
	struct file_t ft = currentTask->fd[fp];
	ft.file_name[0] = '\0';
	ft.offset = 0;
	ft.size = 0;
    ft.aval = 0;
	//.address = NULL;
	return ft.fd;
}




