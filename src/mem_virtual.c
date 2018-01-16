#include "mem_virtual.h"

#include "gcc.h"
#include "x86.h"
#include "mem_physical.h"
#include "thread.h"

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

#define PTE_PWU (PTE_P | PTE_W | PTE_U)

#define NUM_TABLES (PAGESIZE / sizeof(uint32))

#define MEM_USERLO_PDIR (MEM_USERLO_PI / NUM_TABLES)
#define MEM_USERHI_PDIR (MEM_USERHI_PI / NUM_TABLES)

#define VA_PDE_INDEX(va) (va / (PAGESIZE * NUM_TABLES))
#define VA_PTE_INDEX(va) ((va / PAGESIZE) % NUM_TABLES)

/**
 * Addresses of page tables for each process.
 * PageTables[procIndex] represents the page directory for process procIndex.
 */
uint32 pageDirEntries[MAX_PROCS][NUM_TABLES] gcc_aligned(PAGESIZE);

/**
 * Identity map page directory, used by kernel memory.
 */
uint32 pageDirIdentity[NUM_TABLES][NUM_TABLES] gcc_aligned(PAGESIZE);

static void SetPageDirEntry(uint32 procIndex, uint32 pdeIndex, uint32 page)
{
    pageDirEntries[procIndex][pdeIndex] = page * PAGESIZE + PTE_PWU;
}

static void SetPageDirEntryIdentity(uint32 procIndex, uint32 pdeIndex)
{
    pageDirEntries[procIndex][pdeIndex] =
        (uint32)pageDirIdentity[pdeIndex] + PTE_PWU;
}

static void RemovePageDirEntry(uint32 procIndex, uint32 pdeIndex)
{
    pageDirEntries[procIndex][pdeIndex] = 0;
}

static uint32 GetPageTableEntry(
    uint32 procIndex, uint32 pdeIndex, uint32 pteIndex)
{
    uint32 addr = (pageDirEntries[procIndex][pdeIndex] & 0xfffff000)
        + pteIndex * sizeof(uint32);
    return *((uint32*)addr);
}

static void SetPageTableEntry(
    uint32 procIndex, uint32 pdeIndex, uint32 pteIndex,
    uint32 page, uint32 perm)
{
    uint32 addr = (pageDirEntries[procIndex][pdeIndex] & 0xfffff000)
        + pteIndex * sizeof(uint32);
    *((uint32*)addr) = page * PAGESIZE + perm;
}

static void RemovePageTableEntry(
    uint32 procIndex, uint32 pdeIndex, uint32 pteIndex)
{
    uint32 addr = (pageDirEntries[procIndex][pdeIndex] & 0xfffff000)
        + pteIndex * sizeof(uint32);
    *((uint32*)addr) = 0;
}

/**
 * Allocates a page for the page table, and registers it in the page directory
 * for the given process and virtual address.
 * Clears all page table entries for this new page table.
 * Returns the page index of the new physical page, or 0 on failure.
 */
static uint32 AllocPageTable(uint32 procIndex, uint32 va)
{
    uint32 page = ContainerAlloc(procIndex);
    if (page != 0) {
        uint32 pdeIndex = VA_PDE_INDEX(va);
        SetPageDirEntry(procIndex, pdeIndex, page);
        for (uint32 i = 0; i < 1024; i++) {
            RemovePageTableEntry(procIndex, pdeIndex, i);
        }
    }

    return page;
}

/**
 * Reverses AllocPageTable
 */
static void FreePageTable(uint32 procIndex, uint32 va)
{
    uint32 pdeIndex = VA_PDE_INDEX(va);
    uint32 pde = pageDirEntries[procIndex][pdeIndex];
    RemovePageDirEntry(procIndex, pdeIndex);
    ContainerFree(procIndex, pde / PAGESIZE);
}

/**
 * Maps the physical page for the given process and virtual address.
 * Allocates the page table if the page directory entry is not set up.
 * Returns the physical page index registered in the page directory.
 * Sets the err argument on failure.
 */
static uint32 MapPage(uint32 procIndex, uint32 va, uint32 page, uint32 perm, int* err)
{
    uint32 result;
    uint32 pdeIndex = VA_PDE_INDEX(va);
    uint32 pde = pageDirEntries[procIndex][VA_PDE_INDEX(va)];
    if (pde != 0) {
        result = pde / PAGESIZE;
    }
    else {
        result = AllocPageTable(procIndex, va);
        if (result == 0) {
            *err = 1;
            return 0;
        }
    }

    SetPageTableEntry(procIndex, pdeIndex, VA_PTE_INDEX(va), page, perm);
    *err = 0;
    return result;
}

/**
 * Reverses MapPage. Does nothing if there is no mapping.
 * Returns the page table entry for the mapping.
 */
static uint32 UnmapPage(uint32 procIndex, uint32 va)
{
    uint32 pte = GetPageTableEntryByVA(procIndex, va);
    if (pte != 0) {
        RemovePageTableEntry(procIndex, VA_PDE_INDEX(va), VA_PTE_INDEX(va));
    }
    return pte;
}

static void EnablePaging()
{
    // Enable global pages
    uint32 cr4 = GetCR4();
    cr4 |= CR4_PGE;
    SetCR4(cr4);

    // Turn on paging
    /*uint32 cr0 = GetCR0();
    cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_MP;
    cr0 &= ~(CR0_EM | CR0_TS);
    SetCR0(cr0);*/
}

void VirtualMemoryInit()
{
    // Initialize identity page table
    for (uint32 i = 0; i < NUM_TABLES; i++) {
        uint32 perm;
        if (i >= MEM_USERLO_PDIR && i < MEM_USERHI_PDIR) {
            // User memory
            perm = PTE_P | PTE_W;
        }
        else {
            // Kernel memory
            perm = PTE_P | PTE_W | PTE_G;
        }

        for (uint32 j = 0; i < NUM_TABLES; i++) {
            pageDirIdentity[i][j] = (i * NUM_TABLES + j) * PAGESIZE + perm;
        }
    }

    // Identity-map kernel memory for all processes.
    // Identity-map all memory for process 0.
    for (uint32 i = 0; i < MAX_PROCS; i++) {
        for (uint32 j = 0; j < NUM_TABLES; j++) {
            if (j >= MEM_USERLO_PDIR && j < MEM_USERHI_PDIR && i != 0) {
                RemovePageDirEntry(i, j);
            }
            else {
                SetPageDirEntryIdentity(i, j);
            }
        }
    }

    SetPageDirectory(0);
    EnablePaging();

    Printf("Initialized virtual memory\n\n");
}

void SetPageDirectory(uint32 procIndex)
{
    SetCR3((uint32)pageDirEntries[procIndex]);
}

/**
 * Returns the page table entry corresponding to the virtual address
 * for the given process.
 * Returns 0 if the mapping does not exist.
 */
uint32 GetPageTableEntryByVA(uint32 procIndex, uint32 va)
{
    uint32 pdeIndex = VA_PDE_INDEX(va);
    uint32 pde = pageDirEntries[procIndex][pdeIndex];
    if (pde == 0) {
        return 0;
    }

    uint32 pteIndex = VA_PTE_INDEX(va);
    uint32 pte = GetPageTableEntry(procIndex, pdeIndex, pteIndex);
    return pte;
}

/**
 * This function will be called when there's no mapping for va in procIndex
 * Most commonly, this will be called by the page fault handler.
 * Allocates a physical page and maps it to the given virtual address.
 * Returns the physical page index of the new page.
 * Sets err argument on failure.
 */
uint32 AllocPage(uint32 procIndex, uint32 va, uint32 perm, int* err)
{
    uint32 result;
    uint32 page = ContainerAlloc(procIndex);

    if (page == 0) {
        *err = 1;
        return 0;
    }
    else {
        int mapPageErr;
        result = MapPage(procIndex, va, page, perm, &mapPageErr);
        if (mapPageErr) {
            *err = 1;
            return 0;
        }
    }

    *err = 0;
    return result;
}