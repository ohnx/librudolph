; compile with nasm main.s -felf64 then ld main.o -o main
section	.text
    global _start

_start:
    mov     eax, 0x01   ; write
    mov     edi, 0x01   ; to stdout
    mov     rsi, msg    ; the message
    mov     edx, 0xe    ; 14 bytes
    syscall

    mov     eax, 0x3c   ; exit
    mov     edi, 0x01   ; with return code 1
    syscall

msg db `Hello, World!\n`

