#include "mem_physical.h"

#include "thread.h"
#include "screen.h"

#define NUM_PHYSICAL_PAGES (1 << 20)

#define MAX_CHILDREN 8

enum PhysicalPagePerm {
    PPAGEPERM_BIOS,
    PPAGEPERM_KERNEL,
    PPAGEPERM_USER
};

struct PhysicalPageInfo {
    int allocated;
    enum PhysicalPagePerm perm;
};
static struct PhysicalPageInfo pPageInfo[NUM_PHYSICAL_PAGES];

struct Container {
    int quota;
    int usage;
    int parent;
    int numChildren;
    int used;
};
static struct Container containers[MAX_PROCS];

int lastFreedPage = -1;
int lastAllocPage = -1;

// Initialize container data for root process (index 0)
static void ContainerInit(int userPages)
{
    containers[0].quota = userPages;
    containers[0].usage = 0;
    containers[0].parent = 0;
    containers[0].numChildren = 0;
    containers[0].used = 1;
}

void PhysicalMemoryInit(struct SMAP* smap)
{
    struct SMAP* p = smap;
    Printf("-- E820 memory map --\n\n");
	while (p->baseAddr != 0 || p->length != 0 || p->type != 0) {
		Printf("At %l\n", p->baseAddr);
        Printf("   length %x\n", p->length);
        Printf("   type %u\n\n", p->type);
		p++;
	}
    Printf("---------------------\n\n");

    // Initialize all page permissions
    int biosPages = 0;
    int kernPages = 0;
    int userPages = 0;
    for (uint32 page = 0; page < NUM_PHYSICAL_PAGES; page++) {
        uint32 pgStart = page * PAGESIZE;
        uint32 pgEnd = pgStart + PAGESIZE;

        int pageReserved = 1;
        p = smap;
        while (p->baseAddr != 0 || p->length != 0 || p->type != 0) {
            uint32 start = (uint32)p->baseAddr;
            uint32 end = start + (uint32)p->length;

            if (pgStart >= start && pgEnd <= end) {
                // Page is inside this SMAP entry's range
                if (p->type == 1) {
                    pageReserved = 0;
                    break;
                }
            }

            p++;
        }

        if (pageReserved) {
            // Reserved by the BIOS
            pPageInfo[page].allocated = 1;
            pPageInfo[page].perm = PPAGEPERM_BIOS;
            biosPages++;
        }
        else {
            if (page >= MEM_USERLO_PI && page < MEM_USERHI_PI) {
                // User page
                pPageInfo[page].allocated = 0;
                pPageInfo[page].perm = PPAGEPERM_USER;
                userPages++;
            }
            else {
                // Kernel page
                pPageInfo[page].allocated = 0;
                pPageInfo[page].perm = PPAGEPERM_KERNEL;
                kernPages++;
            }
        }
    }

    Printf("%d BIOS pages\n", biosPages);
    Printf("%d kernel pages\n", kernPages);
    Printf("%d user pages\n", userPages);
    PrintfColor(COLOR_BGREEN, "Initialized physical memory\n\n");

    ContainerInit(userPages);
}

uint32 PhysicalPageAlloc()
{
    if (lastFreedPage != -1) {
        uint32 page = (uint32)lastFreedPage;
        lastAllocPage = lastFreedPage;
        lastFreedPage = -1;
        return page;
    }
    else if (lastAllocPage != -1) {
        // Start scan from place of last allocation
        for (uint32 page = (uint32)lastAllocPage + 1;
        page < MEM_USERHI_PI; page++) {
            if (!pPageInfo[page].allocated
            && pPageInfo[page].perm == PPAGEPERM_USER) {
                pPageInfo[page].allocated = 1;
                lastAllocPage = (int)page;
                return page;
            }
        }
        // Otherwise, scan the area before last allocation
        for (uint32 page = MEM_USERLO_PI;
        page < (uint32)lastAllocPage + 1; page++) {
            if (!pPageInfo[page].allocated
            && pPageInfo[page].perm == PPAGEPERM_USER) {
                pPageInfo[page].allocated = 1;
                lastAllocPage = (int)page;
                return page;
            }
        }
    }
    else {
        for (uint32 page = MEM_USERLO_PI; page < MEM_USERHI_PI; page++) {
            if (!pPageInfo[page].allocated
            && pPageInfo[page].perm == PPAGEPERM_USER) {
                pPageInfo[page].allocated = 1;
                lastAllocPage = (int)page;
                return page;
            }
        }
    }

    return 0;
}

void PhysicalPageFree(uint32 page)
{
    pPageInfo[page].allocated = 0;
    lastFreedPage = (int)page;
}

/**
 * Dedicates quota pages of memory for a new child process.
 * Assume it is safe to allocate quota pages.
 * Returns the container index for the new child process
 */
uint32 ContainerSplit(uint32 id, uint32 quota)
{
    uint32 numChildren = containers[id].numChildren;
    uint32 childID = id * MAX_CHILDREN + 1 + numChildren;

    containers[childID].used = 1;
    containers[childID].quota = quota;
    containers[childID].usage = 0;
    containers[childID].parent = id;
    containers[childID].numChildren = 0;

    containers[id].usage += quota;
    containers[id].numChildren += 1;

    return childID;
}

/**
 * Allocates one more physical page for process id
 * if its usage would not exceed its quota.
 * Returns the page index of the allocated page, or 0 on failure.
 */
uint32 ContainerAlloc(uint32 id)
{
    if (containers[id].usage == containers[id].quota) {
        return 0;
    }

    containers[id].usage += 1;
    uint32 page = PhysicalPageAlloc();
    return page;
}

void ContainerFree(uint32 id, uint32 pageIndex)
{
    if (pPageInfo[pageIndex].allocated) {
        PhysicalPageFree(pageIndex);
        if (containers[id].usage > 0) {
            containers[id].usage -= 1;
        }
    }
}
