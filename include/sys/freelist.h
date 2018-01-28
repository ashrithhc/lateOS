struct freeList{
	uint64_t phyaddr;
	struct freeList *next;
};

void initializeFreelist(uint32_t*, void*, void*);

struct freeList* getCurrentFreeListHead();

uint64_t getFreeFrame();

uint64_t getFreeFrameBefore();

void addFrameToFreeList(uint64_t);

void printFreeList(int);

void updateFreeListStart();
