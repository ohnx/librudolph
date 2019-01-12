.globl _start
_start:
	mov    $0x3c,%eax
	mov    $0x1,%edi
	syscall
