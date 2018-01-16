#include "x86.h"

uint32 GetCR0()
{
	uint32 val;
	__asm __volatile("movl %%cr0,%0" : "=r" (val));
	return val;
}
void SetCR0(uint32 val)
{
	__asm __volatile("movl %0,%%cr0" : : "r" (val));
}

void SetCR3(uint32 val)
{
	__asm __volatile("movl %0,%%cr3" : : "r" (val));
}

uint32 GetCR4()
{
	uint32 cr4;
	__asm __volatile("movl %%cr4,%0" : "=r" (cr4));
	return cr4;
}
void SetCR4(uint32 val)
{
	__asm __volatile("movl %0,%%cr4" : : "r" (val));
}