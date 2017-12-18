#include "screen.h"

#include "types.h"

/*static inline int RowColToOffset(int row, int col)
{
    return col * 2 + row * 2 * MAX_COLS;
}*/

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

/**
 * Prints the character c with attributes attr
 * at the given row and column.
 */
static void PrintChar(char c, char attr, int row, int col)
{
    uint8* videoMemory = (uint8*)VIDEO_ADDRESS;

    int offset = col * 2 + row * 2 * MAX_COLS;
    videoMemory[offset] = c;
    videoMemory[offset + 1] = attr;

    SetCursor(offset + 2);
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
    /*while (*c != 0) {
        //PrintCharAtCursor(*c, ATTR_WHITE_ON_BLACK);
        c++;
    }*/

    char* videoMemory = (char*)0xb8000;
    *videoMemory = 'P';
}
