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

int isfileexists(char* path){
    char fpath[100];
    strcpy(fpath,path);
    deriveRelative(fpath);
    char* absolutePath = &fpath[0];
    int flag = -1;
    for(int i=0; i<32 && headers[i]!=NULL; i++)
    {
        if(strcmp(headers[i]->name,absolutePath)==0)
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




