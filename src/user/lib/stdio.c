#include "stdio.h"

#include "screen.h"

#define PRINT_BUFFER_SIZE 512

struct PrintBuf
{
    // buffer index
    int index;
    // bytes printed so far
    int count;
    char buf[PRINT_BUFFER_SIZE];
};

enum Color textColor = COLOR_WHITE;

static void PrintfPutCh(int ch, enum Color color, struct PrintBuf* buf)
{
    buf->buf[buf->index++] = (char)ch;
    if (buf->index >= PRINT_BUFFER_SIZE - 1) {
        buf->buf[buf->index] = 0;
        PutStr(buf->buf, buf->index, color);
        buf->index = 0;
    }
    buf->count++;
}

/*
 * Print a positive number (base <= 16) in reverse order,
 * using specified putCh function and associated pointer putData.
 */
static void PrintNum(
    void (*putCh)(int, enum Color, void*), void *putData,
    enum Color color, uint64 num, int base)
{
	// first recursively print all preceding (more significant) digits
	if (num >= (uint64)base) {
		PrintNum(putCh, putData, color, Divide64by32(num, (uint32)base), base);
	}

	// then print this (the least significant) digit
	putCh("0123456789abcdef"[Modulo64by32(num, (uint32)base)], color, putData);
}

static void VPrintFmt(
    void (*putCh)(int, enum Color, void*), void *putData,
    enum Color color, const char *fmt, va_list ap)
{
    int ch;
    const char* p;
    uint64 num;
    int base;

	while (1) {
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0') {
                return;
            }
			putCh(ch, color, putData);
		}

		// Process a %-escape sequence
		switch (ch = *(unsigned char *) fmt++) {
            // character
            case 'c': {
                putCh(va_arg(ap, int), color, putData);
                break;
            }

            // string
            case 's': {
                if ((p = va_arg(ap, char*)) == 0) {
                    p = "(null)";
                }
                while ((ch = *p++) != '\0') {
                    putCh(ch, color, putData);
                }
                break;
            }

            // signed decimal
            case 'd': {
                int numSigned = va_arg(ap, int);
                if (numSigned < 0) {
                    putCh('-', color, putData);
                    numSigned = -numSigned;
                }
                base = 10;
                PrintNum(putCh, putData, color, (uint64)numSigned, base);
                break;
            }

            // unsigned decimal
            case 'u': {
                num = va_arg(ap, unsigned int);
                base = 10;
                PrintNum(putCh, putData, color, num, base);
                break;
            }

            // unsigned 64-bit (as hex, note non-standard syntax)
            case 'l': {
                num = va_arg(ap, uint64);
                base = 16;
                PrintNum(putCh, putData, color, num, base);
                break;
            }

            // unsigned octal
            case 'o': {
                num = va_arg(ap, unsigned int);
                base = 8;
                PrintNum(putCh, putData, color, num, base);
                break;
            }

            // pointer
            case 'p': {
                num = (uint32)va_arg(ap, void*);
                base = 16;
                putCh('0', color, putData);
                putCh('x', color, putData);
                PrintNum(putCh, putData, color, num, base);
                break;
            }

            // unsigned hexadecimal
            case 'x': {
                num = va_arg(ap, unsigned int);
                base = 16;
                PrintNum(putCh, putData, color, num, base);
                break;
            }

            // escaped '%' character
            case '%':
                putCh(ch, color, putData);
                break;

            // unrecognized escape sequence - just print it literally
            default:
                putCh('%', color, putData);
                for (fmt--; fmt[-1] != '%'; fmt--)
                    /* do nothing */;
                break;
		}
	}
}

int VPrintf(enum Color color, const char *fmt, va_list ap)
{
    struct PrintBuf buf;

    buf.index = 0;
    buf.count = 0;
    VPrintFmt((void*)PrintfPutCh, &buf, color, fmt, ap);

    buf.buf[buf.index] = 0;
    PutStr(buf.buf, buf.index, color);

    return buf.count;
}

int Printf(const char *fmt, ...)
{
    va_list ap;
    int count;

    va_start(ap, fmt);
    count = VPrintf(textColor, fmt, ap);
    va_end(ap);

    return count;
}

int PrintfColor(enum Color color, const char *fmt, ...)
{
    va_list ap;
    int count;

    va_start(ap, fmt);
    count = VPrintf(color, fmt, ap);
    va_end(ap);

    return count;
}