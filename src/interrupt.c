#include "interrupt.h"

#define N_INTERRUPTS 256

struct IDTEntry idtEntries[N_INTERRUPTS];
struct IDTDescriptor idtDescriptor;

void InitIDT()
{
    idtDescriptor.size = sizeof(struct IDTEntry) * N_INTERRUPTS - 1;
    idtDescriptor.start = (uint32)idtEntries;

    MemSet((void*)idtEntries, 0, sizeof(struct IDTEntry) * N_INTERRUPTS);

    /*asm ("lidt %%eax"
        :
        : "a" (&idtDescriptor));*/
}

void SetIDTEntry()
{
}