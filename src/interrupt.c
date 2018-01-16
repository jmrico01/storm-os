#include "interrupt.h"

#include "system.h"
#include "types.h"
#include "x86.h"
#include "gcc.h"
#include "pic.h"
#include "thread.h"
#include "mem_physical.h"
#include "mem_virtual.h"

/* Application segment type bits ('app' bit = 1) */
#define STA_X		0x8	    /* Executable segment */
#define STA_E		0x4	    /* Expand down (non-executable segments) */
#define STA_C		0x4	    /* Conforming code segment (executable only) */
#define STA_W		0x2	    /* Writeable (non-executable segments) */
#define STA_R		0x2	    /* Readable (executable segments) */
#define STA_A		0x1	    /* Accessed */

/* System segment type bits ('app' bit = 0) */
#define STS_T16A	0x1	    /* Available 16-bit TSS */
#define STS_LDT		0x2	    /* Local Descriptor Table */
#define STS_T16B	0x3	    /* Busy 16-bit TSS */
#define STS_CG16	0x4	    /* 16-bit Call Gate */
#define STS_TG		0x5	    /* Task Gate / Coum Transmitions */
#define STS_IG16	0x6	    /* 16-bit Interrupt Gate */
#define STS_TG16	0x7	    /* 16-bit Trap Gate */
#define STS_T32A	0x9	    /* Available 32-bit TSS */
#define STS_T32B	0xB	    /* Busy 32-bit TSS */
#define STS_CG32	0xC	    /* 32-bit Call Gate */
#define STS_IG32	0xE	    /* 32-bit Interrupt Gate */
#define STS_TG32	0xF	    /* 32-bit Trap Gate */

/* Pseudo-descriptors for GDT, LDT and IDT */
struct PseudoDesc {
	uint16  pd_lim;     /* limit */
	uint32  pd_base;    /* base */
} gcc_packed;

/* Segment Descriptors */
struct SegDesc {
	unsigned sd_lim_15_0 : 16;  /* Low bits of segment limit */
	unsigned sd_base_15_0 : 16; /* Low bits of segment base address */
	unsigned sd_base_23_16 : 8; /* Middle bits of segment base address */
	unsigned sd_type : 4;       /* Segment type (see STS_ constants) */
	unsigned sd_s : 1;          /* 0 = system, 1 = application */
	unsigned sd_dpl : 2;        /* Descriptor Privilege Level */
	unsigned sd_p : 1;          /* Present */
	unsigned sd_lim_19_16 : 4;  /* High bits of segment limit */
	unsigned sd_avl : 1;        /* Unused (available for software use) */
	unsigned sd_rsv1 : 1;       /* Reserved */
	unsigned sd_db : 1;         /* 0 = 16-bit segment, 1 = 32-bit segment */
	unsigned sd_g : 1;          /* Granularity: limit scaled by 4K when set */
	unsigned sd_base_31_24 : 8; /* High bits of segment base address */
};

/* Null segment */
#define SEGDESC_NULL							\
	(struct SegDesc){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
/* Segment that is loadable but faults when used */
#define SEGDESC_FAULT							\
	(struct SegDesc){ 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 }
/* Normal segment */
#define SEGDESC32(type, base, lim, dpl) (struct SegDesc)		\
{ ((lim) >> 12) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,	\
    type, 1, dpl, 1, (unsigned) (lim) >> 28, 0, 0, 1, 1,		\
    (unsigned) (base) >> 24 }
#define SEGDESC16(type, base, lim, dpl) (struct SegDesc)		\
{ (lim) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,		\
    type, 1, dpl, 1, (unsigned) (lim) >> 16, 0, 0, 1, 0,		\
    (unsigned) (base) >> 24 }

struct TSS {
	uint32 ts_link;
	uint32 ts_esp0;
	uint16 ts_ss0;  uint16 ts_padding1;
	uint32 ts_esp1;
	uint16 ts_ss1;  uint16 ts_padding2;
	uint32 ts_esp2;
	uint16 ts_ss2;  uint16 ts_padding3;
	uint32 ts_cr3;
	uint32 ts_eip;
	uint32 ts_eflags;
	uint32 ts_eax;
	uint32 ts_ecx;
	uint32 ts_edx;
	uint32 ts_ebx;
	uint32 ts_esp;
	uint32 ts_ebp;
	uint32 ts_esi;
	uint32 ts_edi;
	uint16 ts_es;   uint16 ts_padding4;
	uint16 ts_cs;   uint16 ts_padding5;
	uint16 ts_ss;   uint16 ts_padding6;
	uint16 ts_ds;   uint16 ts_padding7;
	uint16 ts_fs;   uint16 ts_padding8;
	uint16 ts_gs;   uint16 ts_padding9;
	uint16 ts_ldt;  uint16 ts_padding10;
	uint16 ts_trap;
	uint16 ts_iomb;
	uint8  ts_iopm[129];
};

struct GateDesc {
	unsigned gd_off_15_0 : 16;   /* low 16 bits of offset in segment */
	unsigned gd_ss : 16;         /* segment selector */
	unsigned gd_args : 5;        /* # args, 0 for interrupt/trap gates */
	unsigned gd_rsv1 : 3;        /* reserved(should be zero I guess) */
	unsigned gd_type : 4;        /* type(STS_{TG,IG32,TG32}) */
	unsigned gd_s : 1;           /* must be 0 (system) */
	unsigned gd_dpl : 2;         /* descriptor(meaning new) privilege level */
	unsigned gd_p : 1;           /* Present */
	unsigned gd_off_31_16 : 16;  /* high bits of offset in segment */
};

/*
 * Set up a normal interrupt/trap gate descriptor.
 * - isTrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
 * - sel: Code segment selector for interrupt/trap handler
 * - off: Offset in code segment for interrupt/trap handler
 * - dpl: Descriptor Privilege Level -
 *	  the privilege level required for software to invoke
 *	  this interrupt/trap gate explicitly using an int instruction.
*/
#define SETGATE(gate, isTrap, sel, off, dpl)			\
{								\
	(gate).gd_off_15_0 = (uint32) (off) & 0xffff;		\
	(gate).gd_ss = (sel);					\
	(gate).gd_args = 0;					\
	(gate).gd_rsv1 = 0;					\
	(gate).gd_type = (isTrap) ? STS_TG32 : STS_IG32;	\
	(gate).gd_s = 0;					\
	(gate).gd_dpl = (dpl);					\
	(gate).gd_p = 1;					\
	(gate).gd_off_31_16 = (uint32) (off) >> 16;		\
}

#define offsetof(type, member)	__builtin_offsetof(type, member)

static volatile int intr_inited = 0;

/* Entries of interrupt handlers, defined in trap_main.S by TRAPHANDLE* */
extern char Xdivide, Xdebug, Xnmi, Xbrkpt, Xoflow, Xbound, Xillop, Xdevice,
	Xdblflt, Xtss, Xsegnp, Xstack, Xgpflt, Xpgflt, Xfperr, Xalign, Xmchk;
extern char Xirq_timer, Xirq_kbd, Xirq_slave, Xirq_serial2, Xirq_serial1,
	Xirq_lpt, Xirq_floppy, Xirq_spurious, Xirq_rtc, Xirq9, Xirq10, Xirq11,
	Xirq_mouse, Xirq_coproc, Xirq_ide1, Xirq_ide2;
extern char Xsyscall;
extern char Xdefault;

static struct TSS tss0;
uint8 kernelStack[PAGESIZE] gcc_aligned(PAGESIZE);
extern uint8 procStacks[MAX_PROCS][PAGESIZE] gcc_aligned(PAGESIZE);

struct SegDesc  gdtLoc[CPU_GDT_NDESC];
struct TSS      tssLoc[MAX_PROCS];

/* Interrupt Descriptors Table */
struct GateDesc idt[256];
struct PseudoDesc idtDesc = {
    .pd_lim = sizeof(idt) - 1,
    .pd_base = (uint32)idt
};

extern struct TrapFrame userContexts[MAX_PROCS];

void TrapReturn(struct TrapFrame* tf);

static void InterruptInitIDT()
{
	/* check that T_IRQ0 is a multiple of 8 */
	//KERN_ASSERT((T_IRQ0 & 7) == 0);

	/* install a default handler */
	for (uint32 i = 0; i < sizeof(idt) / sizeof(idt[0]); i++) {
		SETGATE(idt[i], 0, CPU_GDT_KCODE, &Xdefault, 0);
    }

	SETGATE(idt[T_DIVIDE],            0, CPU_GDT_KCODE, &Xdivide,       0);
	SETGATE(idt[T_DEBUG],             0, CPU_GDT_KCODE, &Xdebug,        0);
	SETGATE(idt[T_NMI],               0, CPU_GDT_KCODE, &Xnmi,          0);
	SETGATE(idt[T_BRKPT],             0, CPU_GDT_KCODE, &Xbrkpt,        3);
	SETGATE(idt[T_OFLOW],             0, CPU_GDT_KCODE, &Xoflow,        3);
	SETGATE(idt[T_BOUND],             0, CPU_GDT_KCODE, &Xbound,        0);
	SETGATE(idt[T_ILLOP],             0, CPU_GDT_KCODE, &Xillop,        0);
	SETGATE(idt[T_DEVICE],            0, CPU_GDT_KCODE, &Xdevice,       0);
	SETGATE(idt[T_DBLFLT],            0, CPU_GDT_KCODE, &Xdblflt,       0);
	SETGATE(idt[T_TSS],               0, CPU_GDT_KCODE, &Xtss,          0);
	SETGATE(idt[T_SEGNP],             0, CPU_GDT_KCODE, &Xsegnp,        0);
	SETGATE(idt[T_STACK],             0, CPU_GDT_KCODE, &Xstack,        0);
	SETGATE(idt[T_GPFLT],             0, CPU_GDT_KCODE, &Xgpflt,        0);
	SETGATE(idt[T_PGFLT],             0, CPU_GDT_KCODE, &Xpgflt,        0);
	SETGATE(idt[T_FPERR],             0, CPU_GDT_KCODE, &Xfperr,        0);
	SETGATE(idt[T_ALIGN],             0, CPU_GDT_KCODE, &Xalign,        0);
	SETGATE(idt[T_MCHK],              0, CPU_GDT_KCODE, &Xmchk,         0);

	SETGATE(idt[T_IRQ0+IRQ_TIMER],    0, CPU_GDT_KCODE, &Xirq_timer,    0);
	SETGATE(idt[T_IRQ0+IRQ_KBD],      0, CPU_GDT_KCODE, &Xirq_kbd,      0);
	SETGATE(idt[T_IRQ0+IRQ_SLAVE],    0, CPU_GDT_KCODE, &Xirq_slave,    0);
	SETGATE(idt[T_IRQ0+IRQ_SERIAL24], 0, CPU_GDT_KCODE, &Xirq_serial2,  0);
	SETGATE(idt[T_IRQ0+IRQ_SERIAL13], 0, CPU_GDT_KCODE, &Xirq_serial1,  0);
	SETGATE(idt[T_IRQ0+IRQ_LPT2],     0, CPU_GDT_KCODE, &Xirq_lpt,      0);
	SETGATE(idt[T_IRQ0+IRQ_FLOPPY],   0, CPU_GDT_KCODE, &Xirq_floppy,   0);
	SETGATE(idt[T_IRQ0+IRQ_SPURIOUS], 0, CPU_GDT_KCODE, &Xirq_spurious, 0);
	SETGATE(idt[T_IRQ0+IRQ_RTC],      0, CPU_GDT_KCODE, &Xirq_rtc,      0);
	SETGATE(idt[T_IRQ0+9],            0, CPU_GDT_KCODE, &Xirq9,         0);
	SETGATE(idt[T_IRQ0+10],           0, CPU_GDT_KCODE, &Xirq10,        0);
	SETGATE(idt[T_IRQ0+11],           0, CPU_GDT_KCODE, &Xirq11,        0);
	SETGATE(idt[T_IRQ0+IRQ_MOUSE],    0, CPU_GDT_KCODE, &Xirq_mouse,    0);
	SETGATE(idt[T_IRQ0+IRQ_COPROCESSOR], 0, CPU_GDT_KCODE, &Xirq_coproc, 0);
	SETGATE(idt[T_IRQ0+IRQ_IDE1],     0, CPU_GDT_KCODE, &Xirq_ide1,     0);
	SETGATE(idt[T_IRQ0+IRQ_IDE2],     0, CPU_GDT_KCODE, &Xirq_ide2,     0);

	// Use DPL=3 here because system calls are explicitly invoked
	// by the user process (with "int $T_SYSCALL").
	SETGATE(idt[T_SYSCALL],           0, CPU_GDT_KCODE, &Xsyscall,      3);

	/* default */
	SETGATE(idt[T_DEFAULT],           0, CPU_GDT_KCODE, &Xdefault,      0);

	asm volatile("lidt %0" : : "m" (idtDesc));
    Printf("Set interrupt descriptor table\n");
}

static void InterruptEnable(uint8 irq)
{
	if (irq >= 16) {
        return;
    }
	pic_enable(irq);
}

static void PageFaultHandler()
{
    Printf("page fault\n");
}

static void ExceptionHandler()
{
    uint32 type = userContexts[GetCurrentID()].trapno;

    switch (type) {
        case T_PGFLT: {
            PageFaultHandler();
            break;
        }
        default: {
            break;
        }
    }
}

static void InterruptHandler()
{
    uint32 type = userContexts[GetCurrentID()].trapno;

    switch (type) {
        case T_IRQ0 + IRQ_TIMER: {
            pic_eoi();
            break;
        }
        case T_IRQ0 + IRQ_SPURIOUS: {
            break;
        }
        default: {
            pic_eoi();
            break;
        }
    }
}

static void SyscallHandler()
{
    Printf("system call\n");
}

void InterruptInit()
{
    if (intr_inited == 1) {
        return;
    }

	// setup GDT
	gdtLoc[0] = SEGDESC_NULL;
	// 0x08: kernel code
	gdtLoc[CPU_GDT_KCODE >> 3] = SEGDESC32(STA_X | STA_R, 0x0, 0xffffffff, 0);
	// 0x10: kernel data
	gdtLoc[CPU_GDT_KDATA >> 3] = SEGDESC32(STA_W, 0x0, 0xffffffff, 0);
	// 0x18: user code
	gdtLoc[CPU_GDT_UCODE >> 3] = SEGDESC32(STA_X | STA_R, 0x00000000,
        0xffffffff, 3);
	// 0x20: user data
	gdtLoc[CPU_GDT_UDATA >> 3] = SEGDESC32(STA_W, 0x00000000, 0xffffffff, 3);

    // setup TSS
    tss0.ts_esp0 = (uint32)kernelStack + PAGESIZE;
    tss0.ts_ss0 = CPU_GDT_KDATA;
    gdtLoc[CPU_GDT_TSS >> 3] = SEGDESC16(STS_T32A, (uint32)(&tss0),
        sizeof(struct TSS) - 1, 0);
    gdtLoc[CPU_GDT_TSS >> 3].sd_s = 0;

    struct PseudoDesc gdtDesc = {
        .pd_lim = sizeof(gdtLoc) - 1,
        .pd_base = (uint32)gdtLoc
    };
	asm volatile("lgdt %0" :: "m" (gdtDesc));
	asm volatile("movw %%ax,%%gs" :: "a" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%fs" :: "a" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%es" :: "a" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%ds" :: "a" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%ss" :: "a" (CPU_GDT_KDATA));
	/* reload %cs */
	asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (CPU_GDT_KCODE));

    // Load bootstrap TSS
    LTR(CPU_GDT_TSS);

	// Initialize all TSS structures for processes.
	MemZero(tssLoc, sizeof(struct TSS) * MAX_PROCS);
	MemZero(procStacks, sizeof(char) * MAX_PROCS * PAGESIZE);
	for (uint32 pid = 0; pid < MAX_PROCS; pid++)
	{
		tssLoc[pid].ts_esp0 = (uint32)procStacks[pid] + PAGESIZE;
		tssLoc[pid].ts_ss0 = CPU_GDT_KDATA;
		tssLoc[pid].ts_iomb = offsetof(struct TSS, ts_iopm);
		MemZero(tssLoc[pid].ts_iopm, sizeof(uint8) * 128);
		tssLoc[pid].ts_iopm[128] = 0xff;
	}

    PICInit();
    InterruptInitIDT();
    intr_inited = 1;

    InterruptEnable(IRQ_TIMER);
    InterruptEnable(IRQ_KBD);
    InterruptEnable(IRQ_SERIAL13);

    STI();
    Printf("Enabled interrupts\n");

    Printf("Initialized interrupts\n\n");
}

void SwitchTSS(uint32 pid)
{
    gdtLoc[CPU_GDT_TSS >> 3] =
        SEGDESC16(STS_T32A, (uint32)(&tssLoc[pid]), sizeof(struct TSS) - 1, 0);
    gdtLoc[CPU_GDT_TSS >> 3].sd_s = 0;
    LTR(CPU_GDT_TSS);
}

void HandleTrap(struct TrapFrame* tf)
{
    uint32 currentID = GetCurrentID();
    userContexts[currentID] = *tf;
    SetPageDirectory(0);

    Printf("Trap %d from process %d\n", tf->trapno, currentID);

    if (T_DIVIDE <= tf->trapno && tf->trapno <= T_SECEV) {
        ExceptionHandler();
    }
    else if (T_IRQ0 + IRQ_TIMER <= tf->trapno
    && tf->trapno <= T_IRQ0 + IRQ_IDE2) {
        InterruptHandler();
    }
    else if (tf->trapno == T_SYSCALL) {
        SyscallHandler();
    }
    else {
        // unhandled?
    }

    //ProcessStartUser();
    TrapReturn(tf);
}