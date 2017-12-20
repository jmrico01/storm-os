#include "system.h"

// TODO big-deal functions: make them faster

void MemSet(void* dst, uint8 value, uint32 size)
{
    uint8* dstC = (uint8*)dst;
    for (uint32 i = 0; i < size; i++) {
        dstC[i] = value;
    }
}

void MemCopy(void* dst, const void* src, uint32 size)
{
    uint8* dstC = (uint8*)dst;
    const uint8* srcC = (const uint8*)src;
    for (uint32 i = 0; i < size; i++) {
        dstC[i] = srcC[i];
    }
}