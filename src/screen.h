#ifndef SCREEN_H
#define SCREEN_H

// screen width in bytes
#define VGA_WIDTH 80
// screen height in scan lines
#define VGA_HEIGHT 480
#define VGA_PIXELS_PER_BYTE 8
#define VGA_PLANES 4

#define CHAR_SIZE 8
#define CHAR_ROWS (VGA_HEIGHT / CHAR_SIZE)
#define CHAR_COLS (VGA_WIDTH * VGA_PIXELS_PER_BYTE / CHAR_SIZE)

#include "types.h"

enum Color {
    COLOR_BLACK     = 0b0000,
    COLOR_GREY      = 0b1000,
    COLOR_RED       = 0b0100,
    COLOR_GREEN     = 0b0010,
    COLOR_BLUE      = 0b0001,
    COLOR_BRED      = 0b1100,
    COLOR_BGREEN    = 0b1010,
    COLOR_BBLUE     = 0b1001,
    COLOR_BROWN     = 0b0110,
    COLOR_MAGENTA   = 0b0101,
    COLOR_CYAN      = 0b0011,
    COLOR_YELLOW    = 0b1110,
    COLOR_BMAGENTA  = 0b1101,
    COLOR_BCYAN     = 0b1011,
    COLOR_BGREY     = 0b0111,
    COLOR_WHITE     = 0b1111
};

void DrawCharAt(char c, int row, int col,
    const uint8 font[128][8], enum Color color);
// Scrolls the screen down by the given number of rows (pixels)
void ScrollScreen(int rows);

void FlushBuffer();
void DisplayBuffer(const uint8* buf);
void ClearScreen(enum Color color);

#endif