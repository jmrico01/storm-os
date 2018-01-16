#ifndef MEM_VIRTUAL_H
#define MEM_VIRTUAL_H

#include "types.h"

void VirtualMemoryInit();

void SetPageDirectory(uint32 procIndex);
uint32 GetPageTableEntryByVA(uint32 procIndex, uint32 va);

uint32 AllocPage(uint32 procIndex, uint32 va, uint32 perm, int* err);

#endif