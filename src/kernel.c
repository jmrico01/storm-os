#include "screen.h"
#include "console.h"
#include "monitor.h"
#include "interrupt.h"
#include "mem_physical.h"
#include "mem_virtual.h"
#include "thread.h"
#include "printf.h"

struct KernInfo {
    uint32 codeStart;
    uint32 codeEnd;
    uint32 dataStart;
    uint32 dataEnd;
};

void KernelMain(struct SMAP* smap, struct KernInfo kernInfo)
{
    ClearScreen(COLOR_BLACK);
    PrintfColor(COLOR_BGREEN, "Entered kernel\n\n");
    Printf("Code section: %x - %x\n", kernInfo.codeStart, kernInfo.codeEnd);
    Printf("Data section: %x - %x\n\n", kernInfo.dataStart, kernInfo.dataEnd);

    InterruptInit();
    PhysicalMemoryInit(smap);
    VirtualMemoryInit();
    ThreadInit();

    uint32 pid = CreateProcess((void*)(0x10000000), 10000);
    ForceRunProcess(pid);

    MonitorRun();

    Printf("Kernel should never get here...");
    while (1);
}

#include "system.c"
#include "x86.c"
#include "port_io.c"
#include "pic.c"
#include "interrupt.c"
#include "syscall.c"
#include "mem_physical.c"
#include "mem_virtual.c"
#include "thread.c"
#include "elf.c"

#include "string.c"
#include "screen.c"
#include "console.c"
#include "keyboard.c"
#include "printf.c"
#include "monitor.c"

#include "debug.c"