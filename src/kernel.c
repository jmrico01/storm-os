#include "screen.h"

void KernelMain()
{
    char* videoMemory = (char*)0xb8000;
    *videoMemory = 'X';

    PrintString("Hello, sailor");

    //char* videoMemory = (char*)0xb8000;
    *videoMemory = 'D';

    while(1);
}

#include "port_io.c"
#include "screen.c"