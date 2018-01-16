#ifndef USER_STDIO_H
#define USER_STDIO_H

#include "syscall.h"
#include "stdarg.h"
#include "screen.h"

#define GetChar()               SysGetChar()
#define PutStr(str, len, color) SysPutStr((str), (len), (color))

int Printf(const char* fmt, ...);
int PrintfColor(enum Color color, const char* fmt, ...);

#endif