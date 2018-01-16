#ifndef PRINTF_H
#define PRINTF_H

#include "stdarg.h"
#include "screen.h"

int Printf(const char* fmt, ...);
int PrintfColor(enum Color color, const char* fmt, ...);
int VPrintf(enum Color color, const char *fmt, va_list ap);

void SetTextColor(enum Color color);

#endif