#include "port_io.h"

// TODO make these inline?
uint8 PortByteIn(uint16 port)
{
    uint8 result;
    asm ("inb %%dx , %%al"
        : "=a" (result)
        : "d" (port));
    
    return result;
}

void PortByteOut(uint16 port, uint8 byte)
{
    asm ("outb %%al, %%dx"
        :
        : "a" (byte),
          "d" (port));
}

uint16 PortWordIn(uint16 port)
{
    uint16 result;
    asm ("inw %%dx, %%ax"
        : "=a" (result)
        : "d" (port));
    
    return result;
}

void PortWordOut(uint16 port, uint16 word)
{
    asm ("outw %%ax, %%dx"
        :
        : "a" (word),
          "d" (port));
}