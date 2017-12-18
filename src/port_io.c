#include "types.h"

// TODO make these inline?
uint8 PortByteIn(uint16 port)
{
    uint8 result;
    __asm__("in %%dx , %%al"
        : "=a" (result)
        : "d" (port));
    
    return result;
}

void PortByteOut(uint16 port, uint8 byte)
{
    __asm__("out %%al, %%dx"
        :
        : "a" (byte),
          "d" (port));
}

uint16 PortWordIn(uint16 port)
{
    uint16 result;
    __asm__("in %%dx, %%ax"
        : "=a" (result)
        : "d" (port));
    
    return result;
}

void PortWordOut(uint16 port, uint16 word)
{
    __asm__("out %%ax, %%dx"
        :
        : "a" (word),
          "d" (port));
}