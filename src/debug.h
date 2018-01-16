#ifndef DEBUG_H
#define DEBUG_H

#define KERNEL_PANIC(...) do {					\
		DebugPanic(__FILE__, __LINE__, __VA_ARGS__);	\
	} while (0)

#define KERNEL_ASSERT(x) do {						\
		if (!(x))						\
			KERNEL_PANIC("Kernel assertion failed: %s\n", #x); \
	} while(0)

#endif

void DebugPanic(const char*, int, const char*, ...);