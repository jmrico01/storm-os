#include "screen.h"

#include "syscall.h"

/*void DrawRect(uint8* buf, enum Color color,
    int startX, int endX, int startY, int endY)
{

}*/

void DisplayBuffer(const uint8 buf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES])
{
    SysDisplayBuffer(buf);
}