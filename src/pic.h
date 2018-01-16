/*
 * Hardware definitions for the 8259A Programmable Interrupt Controller (PIC).
 *
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Derived from the MIT Exokernel and JOS.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#ifndef PIC_H
#define PIC_H

#include "types.h"

void PICInit();
void pic_setmask(uint16 mask);
void pic_enable(int irq);
void pic_eoi();
void pic_reset();

#endif
