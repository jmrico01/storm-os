#ifndef MEM_VIRTUAL_H
#define MEM_VIRTUAL_H

#include "types.h"

#define PTE_P       0x001 // Present
#define PTE_W       0x002 // Writeable
#define PTE_U       0x004 // User
#define PTE_PWT     0x008 // Write-Through
#define PTE_PCD     0x010 // Cache-Disable
#define PTE_A       0x020 // Accessed
#define PTE_D       0x040 // Dirty
#define PTE_PS      0x080 // Page Size
#define PTE_G       0x100 // Global
#define PTE_COW     0x800 // Avail for system programmer's use

void VirtualMemoryInit();

void SetPageDirectory(uint32 procIndex);
uint32 GetPageTableEntryByVA(uint32 procIndex, uint32 va);

uint32 AllocPage(uint32 procIndex, uint32 va, uint32 perm, int* err);

uint32 PTCopyIn(uint32 pmap_id, uint32 uva, void* kva, uint32 len);
uint32 PTCopyOut(void* kva, uint32 pmap_id, uint32 uva, uint32 len);
uint32 PTMemSet(uint32 pmap_id, uint32 va, char c, uint32 len);

#endif