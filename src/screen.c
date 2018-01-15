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

uint16 planeMasks[VGA_PLANES] = { 0x0102, 0x0202, 0x0402, 0x0802 };

uint8 screenBuf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES];

static void DrawChar(char c, int row, int col, const uint8** font)
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
    int start = row * CHAR_SIZE + col;
    for (int p = 0; p < VGA_PLANES; p++) {
        int offset = p * VGA_WIDTH * VGA_HEIGHT + start;
        for (int i = 0; i < CHAR_SIZE; i++) {
            screenBuf[offset] = fontChar[i];
            offset += VGA_WIDTH;
        }
    }

    for (int i = 0; i < VGA_WIDTH; i++) {
        screenBuf[i] = 0xff;
        screenBuf[i + VGA_WIDTH * 10] = 0xff;
    }
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

    MemSet(screenBuf, 0, VGA_WIDTH * VGA_HEIGHT * VGA_PLANES);

    PortWordOut(0x3ce, 0x5);
    for (int p = 0; p < VGA_PLANES; p++) {
        PortWordOut(0x3c4, planeMasks[p]);
        MemSet(screen, planeColor[p], VGA_WIDTH * VGA_HEIGHT);
    }
    PortWordOut(0x3c4, 0x0f02);

    //DrawChar('A', 5, 5, (const uint8**)fontBasic);
    /*MemSet(screenBuf, 0xff, VGA_WIDTH * VGA_HEIGHT);
    DisplayBuffer((const uint8*)screenBuf);*/
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