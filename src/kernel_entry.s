.globl start
start:

.code32

# Prepare kernel stack
movl $0x0, %ebp
movl $(kernelStack + 4096), %esp

call KernelMain

jmp .
