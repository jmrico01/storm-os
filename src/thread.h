#ifndef THREAD_H
#define THREAD_H

#include "types.h"

#define MAX_PROCS 64

void ThreadInit();

uint32 GetCurrentID();

void ProcessStartUser();
uint32 CreateProcess(void* elfAddr, uint32 quota);

#endif