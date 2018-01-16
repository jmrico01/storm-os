#include "debug.h"

#include "types.h"
#include "stdarg.h"
#include "printf.h"

#define DEBUG_TRACEFRAMES 10

static void DebugTrace(uint32 ebp, uint32* eips)
{
    int i;
	uint32* frame = (uint32*)ebp;

	for (i = 0; i < DEBUG_TRACEFRAMES && frame; i++) {
		eips[i] = frame[1];         /* saved %eip */
		frame = (uint32*)frame[0]; /* saved %ebp */
	}
	for (; i < DEBUG_TRACEFRAMES; i++) {
		eips[i] = 0;
    }
}

void DebugPanic(const char* file, int line, const char* fmt,...)
{
	uint32 eips[DEBUG_TRACEFRAMES];
	va_list ap;

	Printf("[P] %s:%d: ", file, line);

	va_start(ap, fmt);
	VPrintf(COLOR_WHITE, fmt, ap);
	va_end(ap);

	uint32 ebp;
	__asm __volatile("movl %%ebp,%0" : "=rm" (ebp));
	DebugTrace(ebp, eips);
	for (int i = 0; i < DEBUG_TRACEFRAMES && eips[i] != 0; i++) {
		Printf("\tfrom 0x%08x\n", eips[i]);
    }

	Printf("KERNEL PANIC !!!\n");

    while (1);
}