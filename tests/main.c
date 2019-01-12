#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rudolph/buffer.h>
#include <rudolph/elf.h>

/* this test will produce an elf file */
int main() {
    FILE *fp;
    rd_buf_t *buffer, *a, *b, *c, *d, *e, *f, *g;
    struct rd_elfhdr64 *hdr;
    struct rd_prghdr64 *prg;
    struct rd_sechdr64 *sec;
    const char *textstr = ".text";
    const char *shstrtabstr = ".shstrtab";

    /* this code calls syscall exit with return code #1 */
    /* unsigned char exit1_code[] = { 0xB8, 0x3C, 0x00, 0x00, 0x00, 0xBF, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05 }; */
    unsigned char echo_hello[] = {
        0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 0x1 */
        0xBF, 0x01, 0x00, 0x00, 0x00, /* mov edi, 0x1 */
        0x48, 0xBE, 0xA7, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, /* movabs rsi, 0x4000a7*/
        0xBA, 0x0E, 0x00, 0x00, 0x00, /* mov edx, 0xe (15) */
        0x0F, 0x05, /* syscall */
        0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 0x3c (60) */
        0xBF, 0x01, 0x00, 0x00, 0x00, /* mov edi, 0x1 */
        0x0F, 0x05 /* syscall */
    };
    unsigned char echo_msg[] = {
        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\n' /* null terminator technically not neeeded*/
    };

    /* setup */
    fp = fopen("a.out", "wb");
    buffer = rd_buffer_init();
    if (!fp || !buffer) { fprintf(stderr, "oops, die.\n"); exit(-1); }

    /* first thing is to is to make the header */
    a = rd_buffer_initsz(sizeof(struct rd_elfhdr64));
    hdr = (struct rd_elfhdr64 *)rd_buffer_data(a);
    hdr->magic[0] = 0x7F;
    hdr->magic[1] = 'E';
    hdr->magic[2] = 'L';
    hdr->magic[3] = 'F';
    hdr->bclass = RD_ELFHDR_BCLASS_64; /* 64 bit */
    hdr->endianness = RD_ELFHDR_ENDIANNESS_LITTLE; /* little endian */
    hdr->version = 1;
    hdr->abi = RD_ELFHDR_ABI_SYSV; /* linux uses sysv a lot of the time */
    hdr->abi_version = 0;
    hdr->type = RD_ELFHDR_TYPE_EXEC; /* static executable */
    hdr->machine = RD_ELFHDR_MACHINE_X86_64; /* x86_64 machine code */
    hdr->lversion = 1;
    hdr->entry = 0; /* tbd; aka where the actual executable code begins */
    hdr->proghdr = 0; /* tbd; where the program headers are (offset) */
    hdr->sectionhdr = 0; /* tbd; where the section headers are (offset) */
    hdr->flags = 0;
    hdr->hdrsz = sizeof(struct rd_elfhdr64);
    hdr->prghdrsz = sizeof(struct rd_prghdr64);
    hdr->prghdrnbr = 1;
    hdr->shdrsz = sizeof(struct rd_sechdr64);
    hdr->shdrnbr = 3; /* 1 for null, 1 for text, 1 for strings */
    hdr->snshdridx = 2; /* 0-based indexing */
    a->len = a->alloc;

    /* now we make a program header to tell the system to load our executable */
    b = rd_buffer_initsz(sizeof(struct rd_prghdr64));
    prg = (struct rd_prghdr64 *)rd_buffer_data(b);
    prg->type = RD_PRGHDR_TYPE_LOAD; /* we want the entire elf to be loaded */
    prg->flags = RD_PRGHDR_FLAGS_X | RD_PRGHDR_FLAGS_R; /* read and execute */
    prg->offset = 0x0; /* offset in the file - we can just map the entire file */
    prg->vaddr = 0x0000000000400000; /* mapping to 0x0000000000400000 prevents segfaults, i think */
    prg->paddr = prg->vaddr; /* physical address is not used */
    prg->size_file = 0; /* tbd; size of the entire file */
    prg->size_mem = 0; /* tbd ; size of the block in memory (in this case equal to size_file if we map the entire file) */
    prg->alignment = 0x200000; /* map to the nearest 2MiB block */
    b->len = b->alloc;

    /* TODO: it appears as if, to optimize memory space, only the elf header and program header are before the code. */
    /* this would decrease the program size to a+b+<code> */
    /* do the same thing here */

    /* now come the section headers... first up, a null sentinel */
    c = rd_buffer_initsz(sizeof(struct rd_sechdr64));
    sec = (struct rd_sechdr64 *)rd_buffer_data(c);
    sec->name = 0; /* tbd; will be pointer to a null */
    sec->type = RD_SECHDR_TYPE_NULL; /* null sentinel */
    sec->flags = 0; /* all values are 0 */
    sec->addr = 0;
    sec->offset = 0;
    sec->size = 0;
    sec->link = 0;
    sec->info = 0;
    sec->align = 0;
    sec->entsz = 0;
    c->len = c->alloc;

    /* now the code itself */
    d = rd_buffer_initsz(sizeof(struct rd_sechdr64));
    sec = (struct rd_sechdr64 *)rd_buffer_data(d);
    sec->name = 0; /* tbd; will point to string ".text" */
    sec->type = RD_SECHDR_TYPE_PROGBITS; /* this section contains program code */
    sec->flags = RD_SECHDR_FLAGS_ALLOC | RD_SECHDR_FLAGS_EXECINSTR; /* allocate space for this section in memory and execute it */
    sec->addr = 0; /* tbd; the entire file will be mmap'd into 0x0000000000400000 and this is the offset where code begins */
    sec->offset = 0; /* tbd; this is the offset where code begins in the file */
    sec->size = 0; /* tbd; need the length of the program code */
    sec->link = 0; /* no related sections */
    sec->info = 0; /* no extra info */
    sec->align = 1; /* align to the nearest byte */
    sec->entsz = 0; /* ?? */
    d->len = d->alloc;

    /* finally, the text strings */
    e = rd_buffer_initsz(sizeof(struct rd_sechdr64));
    sec = (struct rd_sechdr64 *)rd_buffer_data(e);
    sec->name = 0; /* tbd; will point to string ".shstr" */
    sec->type = RD_SECHDR_TYPE_STRTAB; /* this section contains strings for the elf reader */
    sec->flags = 0; /* this section will stay on the disk and will not be mapped into memory */
    sec->addr = 0; /* section is not loaded, so no address needed */
    sec->offset = 0; /* tbd; offset where the strings begin in the file */
    sec->size = 0; /* tbd; length of the strings part */
    sec->link = 0; /* not linked to any specific sections */
    sec->info = 0; /* no extra info */
    sec->align = 1; /* align to the nearest byte (not really used since it's not mapped to memory) */
    sec->entsz = 0; /* ?? */
    e->len = e->alloc;

    f = rd_buffer_init();
    rd_buffer_push(&f, (const unsigned char *)textstr, strlen(textstr)+1); /* .text first */
    rd_buffer_push(&f, (const unsigned char *)shstrtabstr, strlen(shstrtabstr)+1); /* then .shstrtab */

    g = rd_buffer_init();
    rd_buffer_push(&g, echo_hello, sizeof(echo_hello)); /* push the code to the buffer */
    /* this is like a lowkey relocation */
    *((uint16_t *)(((unsigned char *)rd_buffer_data(g))+12)) = a->len + b->len + c->len + d->len + e->len + f->len + g->len;
    rd_buffer_push(&g, echo_msg, sizeof(echo_msg)); /* push the data to the buffer */

    /* at this point, a+b+c+d+e is where all the null-terminated strings are */
    sec = (struct rd_sechdr64 *)rd_buffer_data(c);
    sec->name = strlen(textstr); /* this will point to exactly null */

    sec = (struct rd_sechdr64 *)rd_buffer_data(e);
    sec->name = strlen(textstr)+1; /* .text appears first, so skip past it */
    sec->offset = a->len + b->len + c->len + d->len + e->len;
    sec->size = f->len;

    /* at this point, a+b+c+d+e is where the executable code begins */
    sec = (struct rd_sechdr64 *)rd_buffer_data(d);
    sec->name = 0; /* .text appears first */
    sec->offset = a->len + b->len + c->len + d->len + e->len + f->len;
    sec->addr = prg->vaddr + a->len + b->len + c->len + d->len + e->len + f->len;
    sec->size = g->len;

    hdr->entry = sec->addr;
    hdr->proghdr = a->len;
    hdr->sectionhdr = a->len + b->len;

    prg->size_file = a->len + b->len + c->len + d->len + e->len + f->len + g->len;
    prg->size_mem = prg->size_file;

    rd_buffer_merge(&buffer, 7, a, b, c, d, e, f, g);

    rd_buffer_free(a);
    rd_buffer_free(b);
    rd_buffer_free(c);
    rd_buffer_free(d);
    rd_buffer_free(e);
    rd_buffer_free(f);
    rd_buffer_free(g);

    /* write file */
    fwrite(rd_buffer_data(buffer), buffer->len, 1, fp);
    fclose(fp);
    rd_buffer_free(buffer);

    /* done */
    return 0;
}