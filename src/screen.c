#include "screen.h"

#include "system.h"
#include "port_io.h"
#include "font8x8.h"

#define VGA_ADDRESS 0xa0000
// screen width in bytes
#define VGA_WIDTH 80
// screen height in scan lines
#define VGA_HEIGHT 480
#define VGA_PIXELS_PER_BYTE 8
#define VGA_PLANES 4

#define CHAR_SIZE 8
#define CHAR_ROWS (VGA_HEIGHT / CHAR_SIZE)
#define CHAR_COLS (VGA_WIDTH * VGA_PIXELS_PER_BYTE / CHAR_SIZE)

struct CursorPos {
    int row;
    int col;
} cursorPos;

uint16 planeMasks[VGA_PLANES] = { 0x0102, 0x0202, 0x0402, 0x0802 };

uint8 screenBuf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES];

enum Color lastClearColor;

static void DrawCharAt(char c, int row, int col, const uint8 font[128][8])
{
    // TODO maybe do these checks somewhere less critical
    if (c < 0) {
        return;
    }
    if (row < 0 || row >= CHAR_ROWS) {
        return;
    }
    if (col < 0 || col >= CHAR_COLS) {
        return;
    }

    const uint8* fontChar = font[(int)c];
    int start = row * CHAR_SIZE * VGA_WIDTH + col;
    for (int p = 0; p < VGA_PLANES; p++) {
        int offset = start + p * VGA_WIDTH * VGA_HEIGHT;
        for (int i = 0; i < CHAR_SIZE; i++) {
            screenBuf[offset] |= reverseBits[fontChar[i]];
            offset += VGA_WIDTH;
        }
    }
}

void MoveCursor(int dRow, int dCol)
{
    cursorPos.row += dRow;
    if (cursorPos.row >= CHAR_ROWS) {
        cursorPos.row = CHAR_ROWS - 1;
        uint8 planeColor[VGA_PLANES] = { 0, 0, 0, 0 };
        for (int p = 0; p < VGA_PLANES; p++) {
            if ((lastClearColor & (0x1 << p)) != 0) {
                planeColor[p] = 0xff;
            }
        }
        for (int p = 0; p < VGA_PLANES; p++) {
            uint8* buf = screenBuf + p * VGA_WIDTH + VGA_HEIGHT;
            MemCopy(buf, buf + VGA_WIDTH * CHAR_SIZE,
                VGA_WIDTH * VGA_HEIGHT - VGA_WIDTH * CHAR_SIZE);
            MemSet(buf + VGA_WIDTH * VGA_HEIGHT - VGA_WIDTH * CHAR_SIZE,
                planeColor[p], VGA_WIDTH * CHAR_SIZE);
        }
    }

    cursorPos.col += dCol;
    if (cursorPos.col >= CHAR_COLS) {
        MoveCursor(1, -cursorPos.col);
    }
}

void PutChar(char c)
{
    if (c == '\n') {
        MoveCursor(1, -cursorPos.col);
    }
    else {
        DrawCharAt(c, cursorPos.row, cursorPos.col, fontBasic);
        MoveCursor(0, 1);
    }
}

void PutStr(const char* str)
{
    while (*str != 0) {
        PutChar(*str);
        str++;
    }

    DisplayBuffer(screenBuf);
}

void DisplayBuffer(const uint8* buf)
{
    uint8* screen = (uint8*)VGA_ADDRESS;

    PortWordOut(0x3ce, 0x5);
    for (int p = 0; p < VGA_PLANES; p++) {
        const uint8* planeBuf = buf + p * VGA_WIDTH * VGA_HEIGHT;
        PortWordOut(0x3c4, planeMasks[p]);
        MemCopy(screen, planeBuf, VGA_WIDTH * VGA_HEIGHT);
    }
    PortWordOut(0x3c4, 0x0f02);
}

void ClearScreen(enum Color color)
{
    uint8* screen = (uint8*)VGA_ADDRESS;

    uint8 planeColor[VGA_PLANES] = { 0, 0, 0, 0 };
    for (int p = 0; p < VGA_PLANES; p++) {
        if ((color & (0x1 << p)) != 0) {
            planeColor[p] = 0xff;
        }
    }

    PortWordOut(0x3ce, 0x5);
    for (int p = 0; p < VGA_PLANES; p++) {
        PortWordOut(0x3c4, planeMasks[p]);
        MemSet(screenBuf + p * VGA_WIDTH * VGA_HEIGHT,
            planeColor[p], VGA_WIDTH * VGA_HEIGHT);
        MemSet(screen, planeColor[p], VGA_WIDTH * VGA_HEIGHT);
    }
    PortWordOut(0x3c4, 0x0f02);

    lastClearColor = color;
}

#if 0
#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define ATTR_WHITE_ON_BLACK 0x0f

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

static int OffsetInc(int offset)
{
    uint8* videoMemory = (uint8*)VIDEO_ADDRESS;

    offset += 2;
    if (offset / MAX_ROWS / 2 == MAX_COLS) {
        // Scroll down one line.
        MemCopy((void*)videoMemory, (void*)&videoMemory[MAX_COLS * 2],
            (MAX_ROWS - 1) * MAX_COLS * 2);
        MemSet((void*)&videoMemory[(MAX_ROWS - 1) * MAX_COLS * 2],
            0, MAX_COLS * 2);
        offset -= MAX_COLS * 2;
    }

    return offset;
}

static int GetCursor()
{
    // The screen device uses its control register as an index
    // to select its internal registers.
    // reg 14: high byte of the cursor's offset
    // reg 15: low byte of the cursor's offset
    PortByteOut(REG_SCREEN_CTRL, 14);
    int offset = PortByteIn(REG_SCREEN_DATA) << 8;
    PortByteOut(REG_SCREEN_CTRL, 15);
    offset += PortByteIn(REG_SCREEN_DATA);

    // Cursor offset reported by the VGA hardware is the
    // number of characters, so multiply by 2.
    return offset * 2;
}

static void SetCursor(int offset)
{
    offset /= 2;
    PortByteOut(REG_SCREEN_CTRL, 14);
    PortByteOut(REG_SCREEN_DATA, (uint8)(offset >> 8));
    PortByteOut(REG_SCREEN_CTRL, 15);
    PortByteOut(REG_SCREEN_DATA, (uint8)(offset & 0x0000FFFF));
}

static void PrintCharAtCursor(char c, char attr)
{
    uint8* videoMemory = (uint8*)VIDEO_ADDRESS;

    int offset = GetCursor();
    if (c == '\n') {
        offset = (offset / MAX_COLS / 2 + 1) * MAX_COLS * 2 - 2;
    }
    else {
        videoMemory[offset] = c;
        videoMemory[offset + 1] = attr;
    }

    SetCursor(OffsetInc(offset));
}

void PrintString(const char* str)
{
    const char* c = str;
    while (*c != 0) {
        PrintCharAtCursor(*c, ATTR_WHITE_ON_BLACK);
        c++;
    }
}

void ClearScreen()
{
    MemSet((void*)VIDEO_ADDRESS, 0, MAX_ROWS * MAX_COLS * 2);
    SetCursor(0);
}

#endif