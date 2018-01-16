#include "thread.h"

#include "gcc.h"
#include "interrupt.h"
#include "mem_physical.h"
#include "elf.h"

enum ThreadState {
    TSTATE_READY,
    TSTATE_RUN,
    TSTATE_SLEEP,
    TSTATE_DEAD
};

struct KernelContext {
    void*   esp;
    uint32  edi;
    uint32  esi;
    uint32  ebx;
    uint32  ebp;
    void*   eip;
};

/**
 * The structure for the thread control block.
 * We are storing the set of TCBs in doubly linked lists.
 * To this purpose, in addition to the thread state, you also
 * need to save the thread (process) id of the previous and
 * the next TCB.
 * Since the value 0 is reserved for thread id 0, we use MAX_PROCS
 * to represent the null index.
 */
struct TCB {
    enum ThreadState state;
    uint32 prev;
    uint32 next;
};

/**
 * The structure for thread queues.
 * The queue structure only needs to record
 * the head and tail index, since we've already implemented
 * the doubly linked list in the TCB structure.
 * This implementation is valid if at any given time, a thread
 * is in at most one thread queue.
 */
struct ThreadQueue {
	uint32 head;
	uint32 tail;
};

uint8 procStacks[MAX_PROCS][PAGESIZE] gcc_aligned(PAGESIZE);

struct KernelContext    kernelContexts[MAX_PROCS];
struct TrapFrame        userContexts[MAX_PROCS];

struct TCB tcbs[MAX_PROCS];
struct ThreadQueue threadQueues[MAX_PROCS + 1];

uint32 currentID;

extern void ContextSwitch(
    struct KernelContext* from, struct KernelContext* to);

/**
 * Insert the TCB #id2 into the tail of the thread queue #id1.
 */
static void TQEnqueue(uint32 id1, uint32 id2)
{
    uint32 oldTailID = threadQueues[id1].tail;
    if (oldTailID == MAX_PROCS) {
        // empty queue
        threadQueues[id1].head = id2;
        threadQueues[id1].tail = id2;
        tcbs[id2].next = MAX_PROCS;
        tcbs[id2].prev = MAX_PROCS;
    }
    else {
        // non-empty queue
        tcbs[oldTailID].next = id2;
        tcbs[id2].prev = oldTailID;
        tcbs[id2].next = MAX_PROCS;
        threadQueues[id1].tail = id2;
    }
}

/**
 * Reverse the action of TQEnqueue.
 * Returns the poped thread's ID, or MAX_PROCS if empty.
 */
static uint32 TQDequeue(uint32 pid)
{
    uint32 head = threadQueues[pid].head;
    if (head != MAX_PROCS) {
        uint32 next = tcbs[head].next;
        tcbs[head].next = MAX_PROCS;

        threadQueues[pid].head = next;
        if (next == MAX_PROCS) {
            // head is the only node in list
            threadQueues[pid].tail = MAX_PROCS;
        }
        else {
            tcbs[next].prev = MAX_PROCS;
        }
    }

    return head;
}

static void TQRemove(uint32 id1, uint32 id2)
{
    uint32 prev = tcbs[id2].prev;
    uint32 next = tcbs[id2].next;
    tcbs[id2].prev = MAX_PROCS;
    tcbs[id2].next = MAX_PROCS;

    if (prev == MAX_PROCS) {
        // id2 was head
        threadQueues[id1].head = next;
    }
    else {
        tcbs[prev].next = next;
    }
    if (next == MAX_PROCS) {
        // id2 was tail
        threadQueues[id1].tail = prev;
    }
    else {
        tcbs[next].prev = prev;
    }
}

void ThreadInit()
{
    // Initialize all TCBs
    for (int i = 0; i < MAX_PROCS; i++) {
        tcbs[i].state = TSTATE_DEAD;
        tcbs[i].prev = MAX_PROCS;
        tcbs[i].next = MAX_PROCS;
    }

    // Initialize all thread queues
    for (int i = 0; i < MAX_PROCS + 1; i++) {
        threadQueues[i].head = MAX_PROCS;
        threadQueues[i].tail = MAX_PROCS;
    }

    currentID = 0;
    tcbs[0].state = TSTATE_RUN;
}

uint32 GetCurrentID()
{
    return currentID;
}

/**
 * Allocates new child thread context, set the state of the
 * new child thread as ready, and pushes it to the ready queue.
 * Returns the child thread id.
 */
uint32 ThreadSpawn(uint32 parent, uint32 quota, void* entry)
{
    uint32 id = ContainerSplit(parent, quota);
    kernelContexts[id].eip = entry;
    kernelContexts[id].esp = &procStacks[id][PAGESIZE - 1];
    tcbs[id].state = TSTATE_READY;
    TQEnqueue(MAX_PROCS, id);

    return id;
}

/**
 * Yield to the next thread in the ready queue.
 * Sets the currently running thread state as ready,
 * and pushes it back to the ready queue.
 */
void ThreadYield()
{
    tcbs[currentID].state = TSTATE_READY;
    TQEnqueue(MAX_PROCS, currentID);

    uint32 nextID = TQDequeue(MAX_PROCS);
    tcbs[nextID].state = TSTATE_RUN;
    if (nextID != currentID) {
        currentID = nextID;
        ContextSwitch(&kernelContexts[currentID], &kernelContexts[nextID]);
    }
}

void ProcessStartUser()
{
    SwitchTSS(currentID);
    SetPageDirectory(currentID);

    TrapReturn((void*)&userContexts[currentID]);
}

uint32 CreateProcess(void* elfAddr, uint32 quota)
{
    uint32 pid = ThreadSpawn(currentID, quota, (void*)ProcessStartUser);

    ELFLoad(elfAddr, pid);

    userContexts[pid].es = CPU_GDT_UDATA | 3;
    userContexts[pid].ds = CPU_GDT_UDATA | 3;
    userContexts[pid].cs = CPU_GDT_UCODE | 3;
    userContexts[pid].ss = CPU_GDT_UDATA | 3;
    userContexts[pid].esp = (void*)MEM_USERHI;
    userContexts[pid].eflags = FL_IF;
    userContexts[pid].eip = ELFEntry(elfAddr);

    return pid;
}