#include "syscall.h"

#include "console.h"
#include "thread.h"
#include "mem_physical.h"
#include "mem_virtual.h"

enum SyscallNumber {
    SYS_PUTSTR = 0,	/* output a string to the screen */
    SYS_GETCHAR,
    SYS_DISPBUF,
    //SYS_SPAWN,	/* create a new process */
    //SYS_YIELD,	/* yield to another process */

    MAX_SYSCALL_NR	/* XXX: always put it at the end of __syscall_nr */
};

enum __error_nr {
	E_SUCC = 0,		/* no errors */
	E_MEM,		    /* memory failure */
	E_IPC,
	E_INVAL_CALLNR,	/* invalid syscall number */
	E_INVAL_ADDR,	/* invalid address */
	E_INVAL_PID,	/* invalid process ID */
	E_INVAL_REG,
	E_INVAL_SEG,
	E_INVAL_EVENT,
	E_INVAL_PORT,
	E_INVAL_HVM,
	E_INVAL_CHID,
	E_INVAL_ID,     /* general invalid id */
	E_DISK_OP,	/* disk operation failure */
	E_HVM_VMRUN,
	E_HVM_MMAP,
	E_HVM_REG,
	E_HVM_SEG,
	E_HVM_NEIP,
	E_HVM_INJECT,
	E_HVM_IOPORT,
	E_HVM_MSR,
	E_HVM_INTRWIN,
    E_EXCEEDS_QUOTA,
    E_MAX_NUM_CHILDEN_REACHED,
    E_INVAL_CHILD_ID,
	E_NEXIST,   // file does not exist
	E_CREATE,   // file does not exist
	E_FNF,      // file not found 
	E_BADF,          // bad file descriptor
	MAX_ERROR_NR	/* XXX: always put it at the end of __error_nr */
};

static char sysBuf[MAX_PROCS][PAGESIZE];

static uint32 SyscallGetArg1(struct TrapFrame* tf)
{
    return tf->regs.eax;
}
static uint32 SyscallGetArg2(struct TrapFrame* tf)
{
    return tf->regs.ebx;
}
static uint32 SyscallGetArg3(struct TrapFrame* tf)
{
    return tf->regs.ecx;
}
static uint32 SyscallGetArg4(struct TrapFrame* tf)
{
    return tf->regs.edx;
}
static uint32 SyscallGetArg5(struct TrapFrame* tf)
{
    return tf->regs.esi;
}
static uint32 SyscallGetArg6(struct TrapFrame* tf)
{
    return tf->regs.edi;
}

static void SyscallSetErrNo(struct TrapFrame* tf, uint32 errno)
{
    tf->regs.eax = errno;
}
static void SyscallSetRetVal1(struct TrapFrame* tf, uint32 retval)
{
    tf->regs.ebx = retval;
}
static void SyscallSetRetVal2(struct TrapFrame* tf, uint32 retval)
{
    tf->regs.ecx = retval;
}
static void SyscallSetRetVal3(struct TrapFrame* tf, uint32 retval)
{
    tf->regs.edx = retval;
}
static void SyscallSetRetVal4(struct TrapFrame* tf, uint32 retval)
{
    tf->regs.esi = retval;
}
static void SyscallSetRetVal5(struct TrapFrame* tf, uint32 retval)
{
    tf->regs.edi = retval;
}

static void SysPutStr(struct TrapFrame* tf)
{
    uint32 pid = GetCurrentID();
    uint32 strVA = SyscallGetArg2(tf);
    uint32 len = SyscallGetArg3(tf);
    uint32 color = SyscallGetArg4(tf);

    if (!(MEM_USERLO <= strVA && strVA + len <= MEM_USERHI)) {
        SyscallSetErrNo(tf, E_INVAL_ADDR);
        return;
    }

    uint32 nBytes;
    uint32 remain = len;
    uint32 curPos = strVA;

    while (remain) {
        if (remain < PAGESIZE - 1) {
            nBytes = remain;
        }
        else {
            nBytes = PAGESIZE - 1;
        }

        if (PTCopyIn(pid, curPos, sysBuf[pid], nBytes) != nBytes) {
            SyscallSetErrNo(tf, E_MEM);
            return;
        }

        sysBuf[pid][nBytes] = '\0';
        PrintfColor(color, sysBuf[pid]);

        remain -= nBytes;
        curPos += nBytes;
    }

    SyscallSetErrNo(tf, E_SUCC);
}

static void SysGetChar(struct TrapFrame* tf)
{
    int ch = GetChar();
    
    SyscallSetErrNo(tf, E_SUCC);
    SyscallSetRetVal1(tf, ch);
}

static void SysDisplayBuffer(struct TrapFrame* tf)
{
    uint32 bufVA = SyscallGetArg2(tf);
    uint32 bufLen = VGA_WIDTH * VGA_HEIGHT * VGA_PLANES;

    if (!(MEM_USERLO <= bufVA && bufVA + bufLen <= MEM_USERHI)) {
        SyscallSetErrNo(tf, E_INVAL_ADDR);
        return;
    }

    DisplayUserBuffer(bufVA);

    SyscallSetErrNo(tf, E_SUCC);
}

/*static void SysSpawn(struct TrapFrame* tf)
{
    Printf("syscall: spawn\n");
}

static void SysYield(struct TrapFrame* tf)
{
    Printf("syscall: yield\n");
}*/

void SyscallDispatch(struct TrapFrame* tf)
{
    uint32 syscallNumber = SyscallGetArg1(tf);

    switch (syscallNumber) {
        case SYS_PUTSTR: {
            SysPutStr(tf);
            break;
        }
        case SYS_GETCHAR: {
            SysGetChar(tf);
            break;
        }
        case SYS_DISPBUF: {
            SysDisplayBuffer(tf);
            break;
        }
        /*case SYS_SPAWN: {
            SysSpawn(tf);
            break;
        }
        case SYS_YIELD: {
            SysYield(tf);
            break;
        }*/
        default: {
            SyscallSetErrNo(tf, E_INVAL_CALLNR);
        }
    }
}