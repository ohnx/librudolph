.section    .text
.global     _start

_start:
    mov r0, #1 /* stdout */
    ldr r1, vs1
    ldr r2, vs2
    mov r7, #4 /* syscall for write */
    swi #0

    mov r0, #0 /* output 0 */
    mov r7, #1 /* syscall for exit */
    swi #0

vs1:
    andeq	r0, r0, r0

vs2:
    andeq	r0, r0, r0
