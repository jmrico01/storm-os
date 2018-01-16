#include "screen.h"
#include "console.h"
#include "monitor.h"
#include "interrupt.h"
#include "mem_physical.h"
#include "mem_virtual.h"
#include "thread.h"
#include "printf.h"

void KernelMain(struct SMAP* smap)
{
    ClearScreen(COLOR_BLACK);
    Printf("Entered kernel\n\n");

    InterruptInit();
    PhysicalMemoryInit(smap);
    VirtualMemoryInit();
    ThreadInit();

    MonitorRun();

    Printf("Kernel should never get here...");
    while (1);
}

#include "system.c"
#include "x86.c"
#include "port_io.c"
#include "interrupt.c"
#include "pic.c"
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