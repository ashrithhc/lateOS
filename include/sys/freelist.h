struct freeList{
	uint64_t phyaddr;
	struct freeList *next;
};

void initializeFreelist();
