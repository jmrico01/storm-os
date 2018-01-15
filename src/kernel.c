#include "types.h"
#include "screen.h"

#include "port_io.h"

void KernelMain()
{
    ClearScreen(COLOR_MAGENTA);
    /*PrintString("Hello, sailor\n");
    PrintString("Hello!\n\n\n");
    PrintString("Hiya");
    PrintString("... and hi again!\n\n\n\n\n\n\n\n\n\ncafe babe");
    PrintString("\ns\nc\nr\no\nl\nl\ni\nn\ng\n_\nt\ne\ns\nt\nboo!\n!!!!!");

    ClearScreen();
    PrintString("Hi from the dark");*/

    while (1) {}
}

#include "system.c"
#include "port_io.c"
#include "screen.c"
#include "interrupt.c"