#ifndef SYSCALL_H
#define SYSCALL_H

#include "interrupt.h"

void SyscallDispatch(struct TrapFrame* tf);

#endif