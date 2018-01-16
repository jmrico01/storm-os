.globl start
start:

.code32

# Received SMAP through %edx

# Prepare kernel stack
movl $0x0, %ebp
movl $(kernelStack + 4096), %esp

pushl $dataEndLD
pushl $dataStartLD
pushl $textEndLD
pushl $textStartLD
pushl %edx
call KernelMain

jmp .
