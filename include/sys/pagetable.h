#include <sys/defs.h>

typedef struct {
	uint8_t p:1;
	uint8_t rw:1;
	uint8_t us:1;
	uint8_t pwt:1;
	uint8_t pcd:1;
	uint8_t a:1;
	uint8_t ign:1;
	uint8_t mbz:2;
	uint8_t avl:3;
	uint64_t pageDirectoryPointer:40;
	uint16_t available:11;
	uint8_t nx:1;
} __attribute__ ((__packed__)) PML4E;

typedef struct {
	uint8_t p:1;
	uint8_t rw:1;
	uint8_t us:1;
	uint8_t pwt:1;
	uint8_t pcd:1;
	uint8_t a:1;
	uint8_t ign:1;
	uint8_t keepzero:1;
	uint8_t mbz:1;
	uint8_t avl:3;
	uint64_t pageDirectoryBase:40;
	uint16_t available:11;
	uint8_t nx:1 ;
} __attribute__ ((__packed__)) PDPE;

typedef struct {
	uint8_t p:1;
	uint8_t rw:1;
	uint8_t us:1;
	uint8_t pwt:1;
	uint8_t pcd:1;
	uint8_t a:1;
	uint8_t ign:1;
	uint8_t keepzero:1;
	uint8_t ign2:1;
	uint8_t avl:3;
	uint64_t pageTableBase:40;
	uint16_t available:11;
	uint8_t nx:1;
} __attribute__ ((__packed__)) PDE;

typedef struct {
	uint8_t p:1;
	uint8_t rw:1;
	uint8_t us:1;
	uint8_t pwt:1;
	uint8_t pcd:1;
	uint8_t a:1;
	uint8_t d:1;
	uint8_t pat:1;
	uint8_t g:1;
	uint8_t avl:3;
	uint64_t physicalPageBase:40;
	uint16_t available:11;
	uint8_t nx:1;
} __attribute__ ((__packed__)) PTE;

void setPageTables();

uint64_t currentCR3();

void setCR3(PML4E *);

void *userAddressSpace();

void setChildPagetables(uint64_t);

void freeThisFrame(uint64_t);

void freePageTable(uint64_t);

void *memset(void *, int, size_t);

void *memcpy(void *, void *, int);
