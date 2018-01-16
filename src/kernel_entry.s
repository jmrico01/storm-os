.globl start
start:

.code32

# Received SMAP through %edx

# Prepare kernel stack
movl $0x0, %ebp
movl $(kernelStack + 4096), %esp

pushl %edx
call KernelMain

jmp .
