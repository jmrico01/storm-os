#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include "types.h"
#include "screen.h"

void SysPutStr(const char *str, uint32 len, enum Color color);
int SysGetChar();
/*int SysSpawn(uint32 exec, uint32 quota);
void SysYield();*/

void SysDisplayBuffer(const uint8 buf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES]);

#endif
