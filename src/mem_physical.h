#ifndef MEM_PHYSICAL_H
#define MEM_PHYSICAL_H

#include "types.h"

#define PAGESIZE 4096
#define MEM_USERLO 0x40000000
#define MEM_USERLO_PI (MEM_USERLO / PAGESIZE)
#define MEM_USERHI 0xF0000000
#define MEM_USERHI_PI (MEM_USERHI / PAGESIZE)

struct SMAP {
	uint32 size;
	uint64 baseAddr;
	uint64 length;
	uint32 type;
} gcc_packed;

void PhysicalMemoryInit(struct SMAP* smap);

uint32 PhysicalPageAlloc();
void PhysicalPageFree(uint32 page);

uint32 ContainerSplit(uint32 id, uint32 quota);
uint32 ContainerAlloc(uint32 id);
void ContainerFree(uint32 id, uint32 pageIndex);

#endif