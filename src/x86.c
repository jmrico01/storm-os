#include "x86.h"

void CLI()
{
	asm volatile("cli":::"memory");
}
void STI()
{
	asm volatile("sti;nop");
}

void LTR(uint16 sel)
{
	asm volatile("ltr %0" : : "r" (sel));
}

uint32 GetCR0()
{
	uint32 val;
	asm volatile("movl %%cr0,%0" : "=r" (val));
	return val;
}
void SetCR0(uint32 val)
{
	asm volatile("movl %0,%%cr0" : : "r" (val));
}

uint32 GetCR2()
{
	uint32 val;
	asm volatile("movl %%cr2,%0" : "=r" (val));
	return val;
}

void SetCR3(uint32 val)
{
	asm volatile("movl %0,%%cr3" : : "r" (val));
}

uint32 GetCR4()
{
	uint32 val;
	asm volatile("movl %%cr4,%0" : "=r" (val));
	return val;
}
void SetCR4(uint32 val)
{
	asm volatile("movl %0,%%cr4" : : "r" (val));
}