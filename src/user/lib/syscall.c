
#include "syscall.h"

#define T_SYSCALL 48

enum SyscallNumber {
    SYS_PUTSTR = 0,	/* output a string to the screen */
    SYS_GETCHAR,
    SYS_DISPBUF,
    //SYS_SPAWN,	/* create a new process */
    //SYS_YIELD,	/* yield to another process */

    MAX_SYSCALL_NR	/* XXX: always put it at the end of __syscall_nr */
};

void SysPutStr(const char *str, uint32 len, enum Color color)
{
	asm volatile("int %0" :
        : "i" (T_SYSCALL),
          "a" (SYS_PUTSTR),
          "b" (str),
          "c" (len),
          "d" (color)
        : "cc", "memory");
}

int SysGetChar()
{
	int errno;
	int c;

	asm volatile("int %2"
        : "=a" (errno),
          "=b" (c)
        : "i" (T_SYSCALL),
          "a" (SYS_GETCHAR)
        : "cc", "memory");

	return errno ? -1 : c;
}

void SysDisplayBuffer(const uint8 buf[VGA_WIDTH * VGA_HEIGHT * VGA_PLANES])
{
	asm volatile("int %0" :
        : "i" (T_SYSCALL),
          "a" (SYS_DISPBUF),
          "b" (buf)
        : "cc", "memory");
}

/*int SysSpawn(uint32 exec, uint32 quota)
{
	int errno;
	int pid;

	asm volatile("int %2"
        : "=a" (errno),
          "=b" (pid)
        : "i" (T_SYSCALL),
          "a" (SYS_SPAWN),
          "b" (exec),
          "c" (quota)
        : "cc", "memory");

	return errno ? -1 : pid;
}

void SysYield()
{
	asm volatile("int %0" :
        : "i" (T_SYSCALL),
          "a" (SYS_YIELD)
        : "cc", "memory");
}*/

#include "types.c"
#include "stdio.c"
#include "screen.c"