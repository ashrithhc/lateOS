#include <sys/syscall`.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/task.h>

extern taskStruct *current, *sleep;

/*
void syscall_handler(struct isr_regs *regs){
	
	uint64_t buf;	
	uint64_t return_val;

	buf = regs->rax;
	if (buf = SYS_opendir) {
		return_val = sys_opendir((char *)regs->rdi);
		regs->rax = return_val;

		*((uint64_t *)current->initKern - 9) = return_val;
	}
*/

/* syscall to open directory*/
dir* sys_opendir(char *path){

        file_t *node;
        file_t *temp_node;
        char *name;
        char path_direct[64];
        int i=0;

        node = root;

        strcpy(path_direct,path);

        name = strtok(path_direct,"/");

        dir* direct_return;
	if ( name ) {

        if ( (strcmp(name,"..") == 0) || (strcmp(name,".") == 0)){
                node = current->curNode;
        }
	}

        while ( name !=NULL ) {
                        temp_node = node;

                        if (strcmp(name,".") == 0 ) {
                                node = node->child[0];

                         }
                        else if (strcmp(name,"..") == 0) {
                                node = node->child[1];
                         }
                        else {

                             for (i=2; i < node->last ; i++) {

                                if (strcmp(name,node->child[i]->name) == 0) {
                                    node = node->child[i];
                                    break;
                                }
                              }

                             if (i == temp_node->last) {
                                return (dir *)NULL;
                             }
                       }

                         name = strtok(NULL,"/");
        }

        if (node->type == DIRECTORY) {
                /*Get free frame call*/
		//direct_return = (dir *);
                direct_return->current = 2;
                direct_return->node = node;
                return direct_return;
        }
        else {
                return (dir *)NULL;
        }
}
