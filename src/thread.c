#include "thread.h"

#include "gcc.h"

uint8 kernelStack[4096] gcc_aligned(4096);
uint8 procStacks[64][4069] gcc_aligned(4096);

void ThreadInit()
{
}