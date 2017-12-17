#include <sys/pagetable.h>
#include <sys/task.h>
#include <sys/freelist.h>
#include <sys/tarfs.h>
#include <sys/dirent.h>
#include <sys/string.h>
#include <sys/defs.h>

//file_t *root;

void parseTarfs(char *path, int type, uint64_t first, uint64_t last){
	file_t *temp, *current, *new;

	char *dirpath = (char *)getFreeFrame();
	strcpy(dirpath, path);
	current = root->child[2];

	char *name = strtok(dirpath, "/");

	if (name == NULL) return;

	while (name != NULL){
		temp  = current;

		int i;
		for (i=2; i<current->last; i++){
			if (strcmp(name, current->child[i]->name) == 0){
				current = (file_t *)current->child[i];
				break;
			}
		}

		if (i == temp->last){
			new = createNode(name, current, type, first, last, 0);
			current->child[current->last] = new;
			current->last += 1;
		}

		name = strtok(NULL, "/");
	}
}

file_t *createNode(char *name, file_t *parent, uint64_t type, uint64_t first, uint64_t last, uint64_t inode){
	file_t *node = (file_t *)getFreeFrame();
	node->type = type;
	node->first = first;
	node->last = last;
	node->current = first;
	node->child[0] = node;
	node->child[1] = parent;
	strcpy(node->name, name);
	node->inode =inode;

	return node;
}

void initTarfs(){
	file_t *node;

	root = (file_t *)getFreeFrame();
	root->type = DIRECTORY;
	root->first = 0;
	root->last = 2;
	root->current = 0;
	root->child[0] = root;
	root->child[1] = root;
	root->inode = 0;
	strcpy(root->name, "/");
	node = createNode("rootfs", root, DIRECTORY, root->first, root->last, root->inode);
	root->last += 1;
	root->child[2] = node;

	tarfsHeader *tarfsHead = (tarfsHeader *)&_binary_tarfs_start;
	uint32_t *end = (uint32_t *)tarfsHead;

	uint64_t decimal;
	while (*end++ || *end++ || *end){
		int j, k = 0;
		decimal = 0;
		for (j=10; j>=0; j--) decimal += (tarfsHead->size[j] - 48) * power(8, k++);
		if (decimal % 512 != 0){
			decimal = (decimal/512) * 512;
			decimal += 512;
		}
		if (strcmp(tarfsHead->typeflag, "5") == 0) parseTarfs(tarfsHead->name, DIRECTORY, 0, 2);
		else parseTarfs(tarfsHead->name, FILES, (uint64_t)(tarfsHead+1), (uint64_t)((void *)tarfsHead + decimal + sizeof(tarfsHeader)));

		tarfsHead = (tarfsHeader *)((uint64_t)tarfsHead + decimal + sizeof(tarfsHeader));
		end = (uint32_t *)tarfsHead;
	}
}

int power(uint64_t a, int n){
	if (n == 0) return 1;
	return a*power(a, n-1);
}
