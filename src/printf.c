#include "printf.h"

#include "stdarg.h"
#include "console.h"

struct PrintBuf
{
    // buffer index
    int index;
    // bytes printed so far
    int count;
    char buf[CONSOLE_BUFFER_SIZE];
};

static void PrintfPutCh(int ch, struct PrintBuf* buf)
{
    buf->buf[buf->index++] = (char)ch;
    if (buf->index >= CONSOLE_BUFFER_SIZE - 1) {
        buf->buf[buf->index] = 0;
        PutStr(buf->buf);
        buf->index = 0;
    }
    buf->count++;
}

/*
 * Print a positive number (base <= 16) in reverse order,
 * using specified putCh function and associated pointer putData.
 */
static void
PrintNum(
    void (*putCh)(int, void*), void *putData,
    uint64 num, int base)
{
	// first recursively print all preceding (more significant) digits
	if (num >= (uint64)base) {
		PrintNum(putCh, putData, Divide64by32(num, (uint32)base), base);
	}

	// then print this (the least significant) digit
	putCh("0123456789abcdef"[Modulo64by32(num, (uint32)base)], putData);
}

static void VPrintFmt(
    void (*putCh)(int, void*), void *putData,
    const char *fmt, va_list ap)
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
			putCh(ch, putData);
		}

		// Process a %-escape sequence
		switch (ch = *(unsigned char *) fmt++) {
            // character
            case 'c': {
                putCh(va_arg(ap, int), putData);
                break;
            }

            // string
            case 's': {
                if ((p = va_arg(ap, char*)) == 0) {
                    p = "(null)";
                }
                while ((ch = *p++) != '\0') {
                    putCh(ch, putData);
                }
                break;
            }

            // signed decimal
            case 'd': {
                int numSigned = va_arg(ap, int);
                if (numSigned < 0) {
                    putCh('-', putData);
                    numSigned = -numSigned;
                }
                base = 10;
                PrintNum(putCh, putData, (uint64)numSigned, base);
                break;
            }

            // unsigned decimal
            case 'u': {
                num = va_arg(ap, unsigned int);
                base = 10;
                PrintNum(putCh, putData, num, base);
                break;
            }

            // unsigned 64-bit (as hex, note non-standard syntax)
            case 'l': {
                num = va_arg(ap, uint64);
                base = 16;
                PrintNum(putCh, putData, num, base);
                break;
            }

            // unsigned octal
            case 'o': {
                num = va_arg(ap, unsigned int);
                base = 8;
                PrintNum(putCh, putData, num, base);
                break;
            }

            // pointer
            case 'p': {
                num = (uint32)va_arg(ap, void*);
                base = 16;
                putCh('0', putData);
                putCh('x', putData);
                PrintNum(putCh, putData, num, base);
                break;
            }

            // unsigned hexadecimal
            case 'x': {
                num = va_arg(ap, unsigned int);
                base = 16;
                PrintNum(putCh, putData, num, base);
                break;
            }

            // escaped '%' character
            case '%':
                putCh(ch, putData);
                break;

            // unrecognized escape sequence - just print it literally
            default:
                putCh('%', putData);
                for (fmt--; fmt[-1] != '%'; fmt--)
                    /* do nothing */;
                break;
		}
	}
}

int VPrintf(const char *fmt, va_list ap)
{
    struct PrintBuf buf;

    buf.index = 0;
    buf.count = 0;
    VPrintFmt((void*)PrintfPutCh, &buf, fmt, ap);

    buf.buf[buf.index] = 0;
    PutStr(buf.buf);

    return buf.count;
}

int Printf(const char *fmt, ...)
{
    va_list ap;
    int count;

    va_start(ap, fmt);
    count = VPrintf(fmt, ap);
    va_end(ap);

    return count;
}