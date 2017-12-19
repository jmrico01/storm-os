#include "screen.h"

#include "port_io.h"

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define ATTR_WHITE_ON_BLACK 0x0f

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

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
    videoMemory[offset] = c;
    videoMemory[offset + 1] = attr;

    SetCursor(offset + 2);
}

void PrintString(const char* str)
{
    const char* c = str;
    while (*c != 0) {
        if (*c == '\n') {
            int offset = GetCursor();
            offset = (offset / MAX_COLS / 2 + 1) * MAX_COLS * 2;
            SetCursor(offset);
        }
        else {
            PrintCharAtCursor(*c, ATTR_WHITE_ON_BLACK);
        }
        c++;
    }
}
