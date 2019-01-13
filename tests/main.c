#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rudolph/buffer.h>
#include <rudolph/elf.h>
#include <rudolph/elf_link.h>

/* this test will produce an elf file */
int main() {
    FILE *fp;
    rd_buf_t *buffer, *code, *data;
    struct rd_elf_link_relocation relocs[3];
    int n;

    unsigned char echo_str_code[] = {
        0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 0x1 */
        0xBF, 0x01, 0x00, 0x00, 0x00, /* mov edi, 0x1 */
        0x48, 0xBE, 0xA7, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, /* movabs rsi, 0x4000a7*/
        0xBA, 0x0E, 0x00, 0x00, 0x00, /* mov edx, 0xe (15) */
        0x0F, 0x05, /* syscall */
        0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 0x3c (60) */
        0xBF, 0x00, 0x00, 0x00, 0x00, /* mov edi, 0x0 */
        0x0F, 0x05, /* syscall */
    };
    unsigned char echo_str_data[] = "Hello, world...\n";

    /* setup */
    fp = fopen("a.out", "wb");
    if (!fp) { fprintf(stderr, "oops, die.\n"); exit(-1); }

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
    rd_buffer_free(data);

    /* done */
    return 0;
}
