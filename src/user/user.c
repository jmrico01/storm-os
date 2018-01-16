#include "lib/types.h"
#include "lib/stdio.h"
#include "lib/screen.h"

/**
 * The user must allocate a buffer to modify the pixels of the screen.
 * This is buffer is not memory mapped. Rather, the user must call
 * "DisplayBuffer" to copy this buffer into the region in kernel memory
 * which IS memory mapped to VGA memory.
 * 
 * The reason this isn't directly memory mapped to VGA memory is because
 * for the video mode I'm using (12H), VGA memory isn't even mapped entirely
 * in the kernel. Only one color plane of the pixels in memory is ever mapped
 * at the same time. It's necessary to send data through the I/O ports to
 * change which color plane is mapped.
 */

uint8 screenBuffer[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES];

int main(int argc, char **argv)
{
    /*Printf("Hello, %s", "sailor\n");

    Printf("What if I wanted to print a really long string?\n\n\n\n\n\n%d",
        0xffffffff);
    Printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nwould is still scroll, even???#@$$#@");
    Printf("Probably...\n\n\n\n\n\n\n yepyepyep");
    PrintfColor(COLOR_RED, "And what about red text? ==%s==", "WARNING: DRAMATIC");
    PrintfColor(COLOR_BCYAN, "\n\n\n\nwoohoo\n\n");*/

    PrintfColor(COLOR_BLACK,
        "Welcome to the user program.\n\n");
    PrintfColor(COLOR_BLACK,
        "Controls: W, A, S, D, Q, E\n");
    PrintfColor(COLOR_BROWN,
        "It's not a game, it's just strange colors...\n");
    PrintfColor(COLOR_BLACK,
        "Press 'V' to begin.\n");

    while (1) {
        int ch = GetChar();
        if (ch > 0) {
            if (ch == 'V' || ch == 'v') {
                break;
            }
        }
    }

    uint8 pattern[4][32] = {
        {
            0b11110000, 0b11101000, 0b11011000, 0b10111000,
            0b01111000, 0b01110100, 0b01101100, 0b01011100,
            0b00111100, 0b00111010, 0b00110110, 0b00101110,
            0b00011110, 0b00011101, 0b00011011, 0b00010111,
            0b00001111, 0b10001110, 0b10001101, 0b10001011,
            0b10000111, 0b01000111, 0b11000110, 0b11000101,
            0b11000011, 0b10100011, 0b01100011, 0b11100010,
            0b11100001, 0b11010001, 0b10110001, 0b01110001
        },
        {
            0b00011000, 0b00100100, 0b01000010, 0b10000001,
            0b11000011, 0b01100110, 0b00111100, 0b00011000,
            0b00111100, 0b11011011, 0b01100110, 0b00111100,
            0b10111101, 0b01100110, 0b11011011, 0b10011001,
            0b00011000, 0b00100100, 0b01000010, 0b10000001,
            0b11000011, 0b01100110, 0b00111100, 0b00011000,
            0b00111100, 0b11011011, 0b01100110, 0b00111100,
            0b10111101, 0b01100110, 0b11011011, 0b10011001

        }, { 0 }, { 0xff, 0, 0, 0, 0, 0, 0xff, 0, 0, 0, 0,
        0, 0, 0, 0xff, 0xfd, 0x0d, 0xca, 0xfe, 0xba, 0xbe }
        /*{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },*/
    };
    int offsetX = 0;
    int offsetY = 0;
    int offsetZ = 0;
    for (int p = 0; p < VGA_PLANES; p++) {
        uint8* buf = screenBuffer + p * VGA_WIDTH * VGA_HEIGHT;
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            buf[i] = pattern[p][offsetY % 32] >> offsetX;
        }
    }
    DisplayBuffer(screenBuffer);

    while (1) {
        int ch = GetChar();
        // The most ridiculous series of if statements ever.
        if (ch == 'A' || ch == 'a' || ch == 'D' || ch == 'd'
        || ch == 'W' || ch == 'w' || ch == 'S' || ch == 's'
        || ch == 'Q' || ch == 'q' || ch == 'E' || ch == 'e') {
            if (ch == 'A' || ch == 'a') {
                offsetX--;
                if (offsetX < 0) {
                    offsetX = 7;
                }
            }
            else if (ch == 'D' || ch == 'd') {
                offsetX = (offsetX + 1) % 8;
            }
            else if (ch == 'S' || ch == 's') {
                offsetY++;
                if (offsetY < 0) {
                    offsetY = 31;
                }
            }
            else if (ch == 'W' || ch == 'w') {
                offsetY++;
            }
            else if (ch == 'Q' || ch == 'q') {
                offsetZ--;
                if (offsetZ < 0) {
                    offsetZ = 3;
                }
            }
            else if (ch == 'E' || ch == 'e') {
                offsetZ++;
            }
            for (int p = 0; p < VGA_PLANES; p++) {
                uint8* buf = screenBuffer + p * VGA_WIDTH * VGA_HEIGHT;
                for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
                    buf[i] = pattern[(p + offsetZ) % 4][offsetY % 32]
                        >> (offsetX + i / 11);
                }
            }
            DisplayBuffer(screenBuffer);
        }
    }

    return 0;
}
