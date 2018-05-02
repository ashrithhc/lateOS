#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/elf64.h>
#include <sys/file.h>
#include <sys/string.h>
#include <sys/tarfs.h>
#include <sys/process.h>
#include <sys/mem.h>
static struct posix_header_ustar *headers[200];
static int filecount = 0;

unsigned int getsize(const char *headerSize)
{
    int size = 0, count = 1;
    for (int j = 11; j > 0; j--){
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
    if (*(path + index) != '\0') return index;
    return -1;
}

/*void setTruePath(char* absPath){
    char file_path[50];
    if( (*absPath) != '/') {
        strcpy(file_path, &(currentTask->curr_dir[1]));
        strcat(file_path, absPath);
    }
    else strcpy(file_path,absPath+1);
    *(absPath) = '\0';
    int a=0;
    int i=0;
    while(*(file_path+i) != '\0')
    {
        if( ((*(file_path+i))=='.') && ( (*(file_path+i+1))=='.'))
        {
            a--;
            *(absPath+a)='\0';
            while( (*(absPath+a)!='/') && a>=0)
            {
                *(absPath+a)='\0';
                a--;
            }
            i++;
        }
        else
        {
            *(absPath+a) = *(file_path+i);
            a++;
        }
        i++;
    }
    *(absPath+a) = *(file_path+i);
    *(absPath+a+1) = '\0';
}*/

void setTruePath(char* absPath){
    int index;
    char file_path[100];
    if( (*absPath) != '/') {
        strcpy(file_path, &(currentTask->curr_dir[1]));
        strcat(file_path, absPath);
    }
    else strcpy(file_path,absPath+1);
    resetString(absPath);
    int pathOffset = getOffset(absPath, 0);
    for(index = 0; *(file_path+index) != '\0'; index++)
    {
        if(((*(file_path+index))=='.') && ( (*(file_path+index+1))=='.'))
        {
            pathOffset--;
            resetString(absPath + pathOffset);
            while( (*(absPath+pathOffset)!='/') && pathOffset>=0)
            {
                resetString(absPath + pathOffset);
                pathOffset--;
            }
            index++;
        }
        else
        {
            *(absPath + pathOffset) = *(file_path + index);
            pathOffset++;
        }
    }
    *(absPath+pathOffset) = *(file_path+index);
    resetString(absPath+pathOffset + 1);
}

int isfileexists(char* path){
    char fi[100];
    strcpy(fi,path);
    setTruePath(fi);
    char* abs_path = &fi[0];
    int flag = -1;
    for(int i=0; i<32 && headers[i]!=NULL; i++)
    {
        if(strcmp(headers[i]->name,abs_path)==0)
        {
            flag = i;
            break;
        }
    }
    if(flag == -1){
        return  -1;
    }
    return flag;
}
int isValidDirectory(char* path){
    struct posix_header_ustar* h;
    int n = isfileexists(path);
    if(n < 0){
        return -1;
    }
    h = headers[n];
    int l = strlen(h->name);
    if(h->name[l-1] == '/'){
        return n;
    }
    return -1;
}
int open_dir(char* path){
    struct posix_header_ustar* h;
    if(strcmp(path,"/")==0)
    {
	    int fdc = currentTask->fd_c+3;
        currentTask->fd_c++;
        (&(currentTask->fd[fdc]))->entry = 0;
        strcpy(&((&(currentTask->fd[fdc]))->file_name[0]),"/");
        return fdc;
    }
    int file_no = isValidDirectory(path);
    if(file_no == -1){
        return  -1;
    }
    h = headers[file_no];
    int fdc = currentTask->fd_c + 3;
    currentTask->fd_c++;
    strcpy(&((&(currentTask->fd[fdc]))->file_name[0]), h->name);
    (&(currentTask->fd[fdc]))->entry = 0;
    (&(currentTask->fd[fdc]))->aval = 1;
    (&(currentTask->fd[fdc]))->size = (uint64_t)(octal_to_binary((char*)(h->size)));
    (&(currentTask->fd[fdc]))->address = (uint64_t)headers[file_no];
    (&(currentTask->fd[fdc]))->fd = fdc;
    return fdc;
}
int open_tarfs(char* path, int flags)
{
        struct posix_header_ustar* h;
        int file_no = isfileexists(path);
        if(file_no == -1){
            return  -1;
        }
        h = headers[file_no];
	    int fdc = currentTask->fd_c + 3;
	    currentTask->fd_c++;
	    strcpy(&((&(currentTask->fd[fdc]))->file_name[0]), h->name);
        (&(currentTask->fd[fdc]))->flags = flags;
	    (&(currentTask->fd[fdc]))->entry = 0;
        (&(currentTask->fd[fdc]))->aval = 1;
    	(&(currentTask->fd[fdc]))->size = (uint64_t)(octal_to_binary((char*)(h->size)));
    	(&(currentTask->fd[fdc]))->address = (uint64_t)headers[file_no];
	    (&(currentTask->fd[fdc]))->fd = fdc;
        return fdc;
}

ssize_t read_tarfs(int fd, char* buf, int count)
{
    if ((&(currentTask->fd[fd]))->aval == 0) return -1;
    if (count == 0) return 0;
	
	struct file_t* fileDescr = (struct file_t*) &(currentTask->fd[fd]);
    struct posix_header_ustar *header;
    header = (struct posix_header_ustar*) fileDescr->address;
    char* start_address = (char *) (header+1);
    if((fileDescr->size) < count) count = fileDescr->size;
    for(int i=0; i < count; i++) buf[i] = *(start_address+i);
    buf[count] = '\0';
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




