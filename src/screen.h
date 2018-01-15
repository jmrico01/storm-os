#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

enum Color {
    COLOR_BLACK     = 0b0000,
    COLOR_GREY      = 0b1000,
    COLOR_RED       = 0b0100,
    COLOR_GREEN     = 0b0010,
    COLOR_BLUE      = 0b0001,
    COLOR_BROWN     = 0b0110,
    COLOR_CYAN      = 0b0011,
    COLOR_MAGENTA   = 0b0101,
    COLOR_BRED      = 0b1100,
    COLOR_BGREEN    = 0b1010,
    COLOR_BBLUE     = 0b1001,
    COLOR_YELLOW    = 0b1110,
    COLOR_BCYAN     = 0b1011,
    COLOR_BMAGENTA  = 0b1101,
    COLOR_WHITE     = 0b1111
};

void PrintStr(const char* str);

void DisplayBuffer(uint8* buf);
void ClearScreen(enum Color color);

#endif