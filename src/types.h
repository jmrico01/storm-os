#ifndef TYPES_H
#define TYPES_H

typedef char        int8;
typedef short       int16;
typedef int         int32;
typedef long long   int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 *
 * Based on former do_div() implementation from asm-parisc/div64.h:
 *	Copyright (C) 1999 Hewlett-Packard Co
 *	Copyright (C) 1999 David Mosberger-Tang <davidm@hpl.hp.com>
 *
 *
 * Generic C version of 64bit/32bit division and modulo, with
 * 64bit result and 32bit remainder.
 *
 * The fast case for (n>>32 == 0) is handled inline by do_div(). 
 *
 * Code generated for this function might be very inefficient
 * for some CPUs. __div64_32() can be overridden by linking arch-specific
 * assembly versions such as arch/ppc/lib/div64.S and arch/sh/lib/div64.S
 * or by defining a preprocessor macro in arch/include/asm/div64.h.
 */
uint32 DivideAndMod64by32(uint64* n, uint32 base)
{
	uint64 rem = *n;
	uint64 b = base;
	uint64 res, d = 1;
	uint32 high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64)high << 32;
		rem -= (uint64)(high * base) << 32;
	}

	while ((int64)b > 0 && b < rem) {
		b = b + b;
		d = d + d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

    *n = res;
	return rem;
}

uint64 Divide64by32(uint64 a, uint32 b)
{
    DivideAndMod64by32(&a, b);
    return a;
}

uint32 Modulo64by32(uint64 a, uint32 b)
{
    return DivideAndMod64by32(&a, b);
}

uint32 RoundDown(uint32 a, uint32 b)
{
	return a - a % b;
}

uint32 RoundUp(uint32 a, uint32 b)
{
	return RoundDown(a + b - 1, b);
}

#endif