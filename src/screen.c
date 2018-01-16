#include "screen.h"

#include "system.h"
#include "port_io.h"
#include "font8x8.h"

#define VGA_ADDRESS 0xa0000

static uint16 planeMasks[VGA_PLANES] = { 0x0102, 0x0202, 0x0402, 0x0802 };
static uint8 screenBuf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES];

static enum Color lastClearColor;

static void PaintRange(int start, int size, enum Color color)
{
    if (start < 0 || start > VGA_WIDTH * VGA_PIXELS_PER_BYTE * VGA_HEIGHT) {
        return;
    }
    if (size < 0 || (start + size)
    > VGA_WIDTH * VGA_PIXELS_PER_BYTE * VGA_HEIGHT) {
        return;
    }

    uint8 planeColor[VGA_PLANES] = { 0, 0, 0, 0 };
    for (int p = 0; p < VGA_PLANES; p++) {
        if ((color & (0x1 << p)) != 0) {
            planeColor[p] = 0xff;
        }
    }

    // First full byte to paint
    int startByte = (start - 1) / VGA_PIXELS_PER_BYTE + 1;
    // Number of full bytes to paint
    int sizeBytes = size / VGA_PIXELS_PER_BYTE;
    for (int p = 0; p < VGA_PLANES; p++) {
        uint8* buf = screenBuf + p * VGA_WIDTH * VGA_HEIGHT;
        MemSet(buf + startByte, planeColor[p], sizeBytes);
    }

    if (startByte * VGA_PIXELS_PER_BYTE != start) {
        // TODO draw incomplete pixels
    }
    if (sizeBytes * VGA_PIXELS_PER_BYTE != size) {
        // TODO draw incomplete pixels
    }
}

void DrawCharAt(char c, int row, int col, const uint8 font[128][8])
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

void ScrollScreen(int rows)
{
    MemCopy(screenBuf, screenBuf + VGA_WIDTH * rows,
        VGA_WIDTH * VGA_HEIGHT * VGA_PLANES - VGA_WIDTH * rows);
    PaintRange(VGA_WIDTH * VGA_PIXELS_PER_BYTE * (VGA_HEIGHT - rows),
        VGA_WIDTH * VGA_PIXELS_PER_BYTE * rows, lastClearColor);
}

void FlushBuffer()
{
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
