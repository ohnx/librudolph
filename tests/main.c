#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rudolph/buffer.h>
#include <rudolph/elf.h>
#include <rudolph/elf_link.h>

/* this test will produce an elf file */
int main() {
    FILE *fp;
    rd_buf_t *buffer, *code;
    struct rd_elf_link_relocation relocs[2];
    int n;

    unsigned char echo_hello_code[] = {
        0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 0x1 */
        0xBF, 0x01, 0x00, 0x00, 0x00, /* mov edi, 0x1 */
        0x48, 0xBE, 0xA7, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, /* movabs rsi, 0x4000a7*/
        0xBA, 0x0E, 0x00, 0x00, 0x00, /* mov edx, 0xe (15) */
        0x0F, 0x05, /* syscall */
        0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 0x3c (60) */
        0xBF, 0x00, 0x00, 0x00, 0x00, /* mov edi, 0x0 */
        0x0F, 0x05, /* syscall */
    };
    unsigned char echo_hello_data[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\n' /* null terminator technically not neeeded*/
    };
    /* setup */
    fp = fopen("a.out", "wb");
    if (!fp) { fprintf(stderr, "oops, die.\n"); exit(-1); }

    /* push the code to a buffer */
    code = rd_buffer_init();
    rd_buffer_push(&code, echo_hello_code, sizeof(echo_hello_code));
    rd_buffer_push(&code, echo_hello_data, sizeof(echo_hello_data));

    /* set up relocations */
    relocs[0].type = RELOC_TEXT16;
    relocs[0].target = 12; /* offset 12 is exactly where the movabs instruction is */
    relocs[0].src = sizeof(echo_hello_code);

    relocs[1].type = RELOC_NULL;

    /* link the code */
    n = rd_elf_link64(RD_ELFHDR_MACHINE_X86_64, code, NULL, relocs, &buffer);

    /* check errors */
    if (n) {
        fprintf(stderr, "Error occurred :( %d\n", n);
    }

    /* write file */
    fwrite(rd_buffer_data(buffer), buffer->len, 1, fp);
    fclose(fp);
    rd_buffer_free(buffer);

    /* done */
    return 0;
}
