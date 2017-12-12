#include <sys/string.h>
#include <sys/tarfs.h>
//#include <sys/dirent.h>

uint64_t exponent( uint64_t x, int e) {
	if (e == 0) return 1;
	return x * exponent(x, e-1);
}

struct file_t *create_node(char *name, file_t *parent_node, uint64_t type, uint64_t first, uint64_t last, uint64_t inode_no){
	//file_t *new_node = (file_t *)
	new_node->type = type;
	new_node->first = first;
	new_node->last = last;
	new_node->current =first;
	new_node->child[0] = new_node;
	new_node->child[1] = parent_node;
	strcpy(new_node->name,name);
	new_node->inode_no = inode_no;

	return new_node;
}


void parse_tarfs(char *path, int type, uint64_t first, uint64_t last){
	file_t *buffer_node;
	file_t *node_current;
	file_t *new_temp_node;
	char *name;
	char *dirpath;
	
	//dirpath = allocate virtual page
	strcpy(dirpath, path);
	node_current = root->child[2];
	name = strtok(dirpath,"/");
	if(name == NULL) return;
	
	while(name != NULL){
		buffer_node = node_current;
		
		for( i = 2; i< node_current->last; i++){
			if(strcmp(name, node_current->child[i]->name) == 0) {
				node_current = (file_t *) node_current->child[i];
				break;
			}
		}
		
		if( i == buffer_node->last){
			new_temp_node = create_node(name, node_current, type, first, last, 0);
			node_cuurent->child[node_current->last] = new_temp_node;
			node_current->last +=1;
		}
			name = strtok(NULL, "/");
	}
}



/* Tarfs functionality */

void init_tarfs(){
	file_t *node;
	//root = (file_t *)...;
	root->type = DIRECTORY; //type flag = 2
	root->first = 0;
	root->last = 2;
	root->current = 0;
	root->child[0] = root;
	root->child[1] = root;
	root->inode_no = 0;
	
	strcpy(root->name, "/");
	//Refernce to rootfs which is the second node
	node = create_node("rootfs", root, DIRECTORY, 0, 2, 0);
	root->last += 1;
	root->child[2] = node
	uint64_t size_value;
	// Tarfs head 
	struct posix_header_ustar *head = (struct posix_header_ustar *)&_binary_tarfs_start;
	uint32_t *end = (uint32_t *)head;

	while(*end++ || *end++ || *end){
		size_value = 0;
		int k = 0;
		for(int j =10; j>= 0 ; --j){
			size_value = size_value + (head->size[j] - 48) * exponent(8, k++);
		}
		if(size_value%512 != 0){
			size_value = (size_value/512) * 512;
			size_value += 512;
	}
	if (strcmp(head->typeflag, "5") == 0){
		parse_tarfs(head->name, DIRECTORY, 0, 2 );
	}
	else{
		parse_tarfs(head->name , FILE, (uint64_t)(head+1), (uint64_t)((void*)head + size_value + sizeof(struct posix_header_ustar)));
	}

	head = (struct posix_head_ustar *)((uint64_t)head + size_value + sizeof(struct posiz_header_ustar));
	end = (uint32_t *) head;
	}
}
