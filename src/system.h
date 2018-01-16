#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

void MemZero(void* dst, uint32 size);
void MemSet(void* dst, uint8 value, uint32 size);
void MemCopy(void* dst, const void* src, uint32 size);

#endif