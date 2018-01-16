#include "screen.h"
#include "console.h"
#include "mem_physical.h"
#include "mem_virtual.h"
#include "thread.h"
#include "printf.h"

void KernelMain(struct SMAP* smap)
{
    ClearScreen(COLOR_BLACK);
    Printf("Entered kernel\n\n");

    PhysicalMemoryInit(smap);
    VirtualMemoryInit();
    ThreadInit();

    int ch;
    while (1) {
        ch = GetChar();
        if (ch != 0) {
            PutChar((char)ch);
            FlushBuffer();
        }
    }

    while (1);
    ClearScreen(COLOR_BLUE);
    Printf("Hello, sailor\n\n\n...hi? yes %d", 1125);
    Printf("Hello, sailor hex %x\n", 0xcafebabe);
    Printf("Hello!\n\n\n");
    Printf("Hiya ");
    Printf("... hi again!\n\n\n\n\n\n\n\n\n\ncafe babe");
    Printf("\ns\nc\nr\no\nl\nl\ni\nn\ng\n \nt\ne\ns\nt\nboo!\n!!!!!");
    Printf("now scrolling forever\n\n\n\n\n\n\n\n\n\n\n\n");
    Printf("64-bit number incoming!!!!! %l\n", 0xdeafcafebabeface);
    //Printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    Printf("done\n");
    Printf("...really, I'm done %s\n", "...except for this hidden message\n");
    Printf("Symbols: !@#$%^&*()-=_+\\][|}{';\":/.,?><\n\n");
    Printf("Now testing backspace\b\bd\n");

    /*ClearScreen(COLOR_BLACK);
    ResetCursor();
    Printf("Hi from the dark");*/

    while (1);
}

#include "system.c"
#include "x86.c"
#include "port_io.c"
#include "mem_physical.c"
#include "mem_virtual.c"
#include "thread.c"

#include "string.c"
#include "screen.c"
#include "console.c"
#include "keyboard.c"
#include "printf.c"
#include "interrupt.c"