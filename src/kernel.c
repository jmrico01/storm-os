#include "types.h"
#include "screen.h"

#include "port_io.h"

uint8 kernelStack[4096];

void KernelMain()
{
    ClearScreen(COLOR_BLUE);
    PutStr("Hello, sailor\n\n\n...hi? yes");
    PutStr("Hello, sailor\n");
    PutStr("Hello!\n\n\n");
    PutStr("Hiya");
    PutStr("... and hi again!\n\n\n\n\n\n\n\n\n\ncafe babe");
    PutStr("\ns\nc\nr\no\nl\nl\ni\nn\ng\n \nt\ne\ns\nt\nboo!\n!!!!!");
    PutStr("now scrolling forever\n\n\n\n\n\n\n\n\n\n\n\n");
    PutStr("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    PutStr("done");

    //ClearScreen(COLOR_BLACK);
    //PutStr("Hi from the dark");

    while (1) {}
}

#include "system.c"
#include "port_io.c"
#include "screen.c"
#include "interrupt.c"