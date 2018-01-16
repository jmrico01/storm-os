#include "console.h"

#include "screen.h"
#include "font8x8.h"
#include "keyboard.h"

struct {
	char buf[CONSOLE_BUFFER_SIZE];
	int readPos, writePos;
} console = { 0 };

struct {
    int row;
    int col;
} cursorPos = {
    0, 0
};

static void MoveCursor(int dRow, int dCol)
{
    cursorPos.row += dRow;
    if (cursorPos.row < 0) {
        cursorPos.row = 0;
    }
    if (cursorPos.row >= CHAR_ROWS) {
        cursorPos.row = CHAR_ROWS - 1;
        ScrollScreen(CHAR_SIZE);
    }

    cursorPos.col += dCol;
    if (cursorPos.col < 0) {
        cursorPos.col = 0;
    }
    if (cursorPos.col >= CHAR_COLS) {
        MoveCursor(1, -cursorPos.col);
    }
}

int GetChar()
{
    int ch;

    // Poll for any pending input characters
    while ((ch = KeyboardGetData()) != -1) {
        if (ch == 0) {
            continue;
        }
        console.buf[console.writePos++] = (int)ch;
        if (console.writePos == CONSOLE_BUFFER_SIZE) {
            console.writePos = 0;
        }
    }

    // Grab the next character from input buffer
    if (console.readPos != console.writePos) {
        ch = console.buf[console.readPos++];
        if (console.readPos == CONSOLE_BUFFER_SIZE) {
            console.readPos = 0;
        }
        return ch;
    }

    return 0;
}

void PutChar(char c, enum Color color)
{
    if (c == '\n') {
        MoveCursor(1, -cursorPos.col);
    }
    else if (c == '\b') {
        MoveCursor(0, -1);
        DrawCharAt('\b', cursorPos.row, cursorPos.col, fontBasic, color);
    }
    else {
        DrawCharAt(c, cursorPos.row, cursorPos.col, fontBasic, color);
        MoveCursor(0, 1);
    }
}

void PutStr(const char* str, enum Color color)
{
    while (*str != 0) {
        PutChar(*str, color);
        str++;
    }

    FlushBuffer();
}

void ResetCursor()
{
    cursorPos.row = 0;
    cursorPos.col = 0;
}
