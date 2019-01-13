#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rudolph/buffer.h>
#include <rudolph/elf.h>
#include <rudolph/elf_link.h>

/* this test will produce an elf file */
int main() {
    FILE *fp, *fp2;
    rd_buf_t *buffer, *code, *data;
    struct rd_elf_link_relocation relocs[3];
    int n;

    unsigned char echo_str_code[] = {
        0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 0x1 (write) */
        0xBF, 0x01, 0x00, 0x00, 0x00, /* mov edi, 0x1 (stdout) */
        0x48, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* movabs rsi, 0x00 (will be relocated) */
        0xBA, 0x09, 0x00, 0x00, 0x00, /* mov edx, 0x00 (will be relocated) */
        0x0F, 0x05, /* syscall */
        0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 0x3c (60 = exit) */
        0xBF, 0x00, 0x00, 0x00, 0x00, /* mov edi, 0x0 (exit coe 0) */
        0x0F, 0x05, /* syscall */
    };
    unsigned char echo_str_code_arm[] = {
        0x01, 0x00, 0xA0, 0xE3, /* mov r0, #1 (stdout) */
        0x14, 0x10, 0x9F, 0xE5, /* ldr r1, [pc, #20] (aka first variable storage) */
        0x14, 0x20, 0x9F, 0xE5, /* ldr r2, [pc, #20] (aka the second variable, length) */
        0x04, 0x70, 0xA0, 0xE3, /* mov r7, #4 (aka write) */
        0x00, 0x00, 0x00, 0xEF, /* svc 0x0 (aka swi 0x0) - syscall */
        0x00, 0x00, 0xA0, 0xE3, /* mov r0, #0 */
        0x01, 0x70, 0xA0, 0xE3, /* mov r7, #1 (aka exit) */
        0x00, 0x00, 0x00, 0xEF, /* svc 0x0 (aka swi 0x0) - syscall */
        0x00, 0x00, 0x00, 0x00, /* used for relocation of string location */
        0x00, 0x00, 0x00, 0x00, /* used for relocation of string length */
    };
    unsigned char echo_str_data[] = "Hello, world!!!\n";

    /* setup */
    fp = fopen("a.out", "wb");
    fp2 = fopen("a.arm.out", "wb");
    if (!fp || !fp2) { fprintf(stderr, "oops, die.\n"); exit(-1); }

    /* push the code to a buffer */
    code = rd_buffer_init();
    rd_buffer_push(&code, echo_str_code, sizeof(echo_str_code));

    /* push the data to another buffer */
    data = rd_buffer_init();
    rd_buffer_push(&data, echo_str_data, sizeof(echo_str_data));

    /* set up relocations */
    relocs[0].type = RELOC_DATA64;
    relocs[0].target = 12; /* offset 12 is exactly where the movabs instruction is */
    relocs[0].src = 0;

    relocs[1].type = RELOC_CUSTOM32;
    relocs[1].target = 21; /* offset 21 is exactly where the length is */
    relocs[1].src = sizeof(echo_str_data);

    relocs[2].type = RELOC_NULL;

    /* link the code */
    buffer = NULL;
    n = rd_elf_link64(RD_ELFHDR_MACHINE_X86_64, code, data, relocs, &buffer);

    /* check errors */
    if (n) {
        fprintf(stderr, "Error occurred :( %d\n", n);
    }

    /* write file */
    fwrite(rd_buffer_data(buffer), buffer->len, 1, fp);
    fclose(fp);
    rd_buffer_free(buffer);
    rd_buffer_free(code);

    /* ... now for the arm code ... */

    /* push the code to a buffer */
    code = rd_buffer_init();
    rd_buffer_push(&code, echo_str_code_arm, sizeof(echo_str_code_arm));

    /* data buffer already set up */

    /* set up relocations */
    relocs[0].type = RELOC_DATA32;
    relocs[0].target = 0x20; /* offset 0x20 is where the location storage is */
    relocs[0].src = 0;

    relocs[1].type = RELOC_CUSTOM32;
    relocs[1].target = 0x24; /* offset 0x24 is where the length value is */
    relocs[1].src = sizeof(echo_str_data);

    relocs[2].type = RELOC_NULL;

    /* link the code */
    n = rd_elf_link32(RD_ELFHDR_MACHINE_ARM, code, data, relocs, &buffer);

    /* check errors */
    if (n) {
        fprintf(stderr, "Error occurred :( %d\n", n);
    }

    /* write file */
    fwrite(rd_buffer_data(buffer), buffer->len, 1, fp2);
    fclose(fp2);
    rd_buffer_free(buffer);
    rd_buffer_free(data);

    /* done */
    return 0;
}
