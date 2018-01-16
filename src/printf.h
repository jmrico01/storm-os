#ifndef PRINTF_H
#define PRINTF_H

#include "screen.h"

int Printf(const char* fmt, ...);
int PrintfColor(enum Color color, const char* fmt, ...);

void SetTextColor(enum Color color);

#endif