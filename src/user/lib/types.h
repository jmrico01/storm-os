#ifndef USER_TYPES_H
#define USER_TYPES_H

typedef char        int8;
typedef short       int16;
typedef int         int32;
typedef long long   int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

uint32 DivideAndMod64by32(uint64* n, uint32 base);
uint64 Divide64by32(uint64 a, uint32 b);
uint32 Modulo64by32(uint64 a, uint32 b);

uint32 RoundDown(uint32 a, uint32 b);
uint32 RoundUp(uint32 a, uint32 b);

#endif