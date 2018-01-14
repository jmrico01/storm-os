#ifndef INTERRUPT_H
#define INTERRUPT_H

struct IDTEntry {
    uint16  jmpAddrLo;  // Lower 16 bits of interrupt handler address.
    uint16  jmpSeg;     // Kernel segment of interrupt handler.
    uint8   zero;       // Must always be set to zero.
    uint8   flags;      // Flags:
                        // 7    present
                        // 6-5  privilege level called from
                        // 4-1  always set to 0b00110
    uint16  jmpAddrHi;  // Higher 16 bits of interrupt handler address.
} __attribute__((packed));

struct IDTDescriptor {
    uint16 size;
    uint32 start;
} __attribute__((packed));

void InitIDT();

#endif