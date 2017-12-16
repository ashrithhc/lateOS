#define FILES 1
#define DIRECTORY 2
#define READONLY 1
#define WRITEONLY 2
#define READWRITE 3

typedef struct file_t file_t;
typedef struct file_t root;
struct file_t{
	uint64_t first;
	uint64_t last;
	uint64_t current;
	file_t *child[15];
	char name[30];
	int type;
	uint64_t inode;
};

typedef struct fd fd;
struct fd{
	uint64_t current;
	uint64_t permission;
	uint64_t inode;
	file_t *node;
};

typedef struct dirent dirent;
struct dirent{
	uint64_t inode;
	char name[30];
};

typedef struct dir dir;
struct dir{
	file_t *node;
	uint64_t current;
	dirent *currentDirectory;
	int fd;
	char buff[30];
};


