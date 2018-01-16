#ifndef X86_H
#define X86_H

#include "types.h"
#include "gcc.h"

// CR0
#define CR0_PE		0x00000001	/* Protection Enable */
#define CR0_MP		0x00000002	/* Monitor coProcessor */
#define CR0_EM		0x00000004	/* Emulation */
#define CR0_TS		0x00000008	/* Task Switched */
#define CR0_NE		0x00000020	/* Numeric Errror */
#define CR0_WP		0x00010000	/* Write Protect */
#define CR0_AM		0x00040000	/* Alignment Mask */
#define CR0_PG		0x80000000	/* Paging */

// CR4
#define CR4_PGE 0x00000080  /* Page Global Enable */

void CLI();
void STI();

void LTR(uint16 sel);

uint32 GetCR0();
void SetCR0(uint32 val);

uint32 GetCR2();

void SetCR3(uint32 val);

uint32 GetCR4();
void SetCR4(uint32 val);

#endif