#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rudolph/buffer.h>
#include <rudolph/elf.h>

/* this test will produce an elf file */
int main() {
    FILE *fp;
    buf_t *buffer, *a, *b, *c, *d, *e, *f;
    struct rd_elfhdr64 *hdr;
    struct rd_prghdr64 *prg;
    struct rd_sechdr64 *sec;
    const char *textstr = ".text";
    const char *shstrtabstr = ".shstrtab";

    /* this code calls syscall exit with return code #1 */
    unsigned char exit1_code[] = { 0xB8, 0x3C, 0x00, 0x00, 0x00, 0xBF, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x05 };

    /* setup */
    fp = fopen("a.out", "wb");
    buffer = buffer_init();
    if (!fp || !buffer) { fprintf(stderr, "oops, die.\n"); exit(-1); }

    /* first thing is to is to make the header */
    a = buffer_initsz(sizeof(struct rd_elfhdr64));
    hdr = (struct rd_elfhdr64 *)buffer_data(a);
    hdr->magic[0] = 0x7F;
    hdr->magic[1] = 'E';
    hdr->magic[2] = 'L';
    hdr->magic[3] = 'F';
    hdr->bclass = RD_ELFHDR_BCLASS_64;
    hdr->endianness = RD_ELFHDR_ENDIANNESS_LITTLE;
    hdr->version = 1;
    hdr->abi = RD_ELFHDR_ABI_SYSV;
    hdr->abi_version = 0;
    hdr->type = RD_ELFHDR_TYPE_EXEC;
    hdr->machine = RD_ELFHDR_MACHINE_X86_64;
    hdr->lversion = 1;
    hdr->entry = 0; /* tbd; aka where the actual executable code begins */
    hdr->proghdr = 0; /* tbd */
    hdr->sectionhdr = 0; /* tbd */
    hdr->flags = 0;
    hdr->hdrsz = 64;
    hdr->prghdrsz = sizeof(struct rd_prghdr64);
    hdr->prghdrnbr = 1;
    hdr->shdrsz = sizeof(struct rd_sechdr64);
    hdr->shdrnbr = 2; /* 1 for text, 1 for strings */
    hdr->snshdridx = 1; /* 0-based indexing */
    a->len = a->alloc;

    /* now we make a program header to tell the system to load our executable */
    b = buffer_initsz(sizeof(struct rd_prghdr64));
    prg = (struct rd_prghdr64 *)buffer_data(b);
    prg->type = RD_PRGHDR_TYPE_LOAD;
    prg->flags = RD_PRGHDR_FLAGS_X | RD_PRGHDR_FLAGS_R; /* read and execute */
    prg->offset = 0x0;
    prg->vaddr = 0x0000000000000000; /* it's easiest to map to 0x0 */
    prg->paddr = 0x0000000000000000;
    prg->size_file = 0; /* tbd */
    prg->size_mem = 0; /* tbd */
    prg->alignment = 0x200000;
    b->len = b->alloc;    

    /* now come the section headers... first up, the code itself */
    c = buffer_initsz(sizeof(struct rd_sechdr64));
    sec = (struct rd_sechdr64 *)buffer_data(c);
    sec->name = 0; /* tbd */
    sec->type = RD_SECHDR_TYPE_PROGBITS;
    sec->flags = RD_SECHDR_FLAGS_ALLOC | RD_SECHDR_FLAGS_EXECINSTR; /* allocate space for this section in memory and execute it */
    sec->addr = 0; /* tbd; the entire file will be mmap'd into 0x0 and this is the offset where code begins */
    sec->offset = 0; /* tbd; this is the same as addr in our simplified case with vaddr = 0x0 */
    sec->size = 0; /* tbd */
    sec->link = 0;
    sec->info = 0;
    sec->align = 1;
    sec->entsz = 0;
    c->len = c->alloc;

    d = buffer_initsz(sizeof(struct rd_sechdr64));
    sec = (struct rd_sechdr64 *)buffer_data(d);
    sec->name = 0; /* tbd */
    sec->type = RD_SECHDR_TYPE_STRTAB;
    sec->flags = 0;
    sec->addr = 0; /* section is not loaded */
    sec->offset = 0; /* tbd */
    sec->size = 0; /* tbd */
    sec->link = 0;
    sec->info = 0;
    sec->align = 1;
    sec->entsz = 0;
    d->len = d->alloc;

    e = buffer_init();
    buffer_push(&e, (const unsigned char *)textstr, strlen(textstr)+1);
    buffer_push(&e, (const unsigned char *)shstrtabstr, strlen(shstrtabstr)+1);

    f = buffer_init();
    buffer_push(&f, exit1_code, sizeof(exit1_code));

    /* at this point, a+b+c+d is where all the null-terminated strings are */
    sec = (struct rd_sechdr64 *)buffer_data(d);
    sec->name = strlen(textstr)+1; /* .text appears first, so skip past it */
    sec->offset = a->len + b->len + c->len + d->len;
    sec->size = e->len;

    /* at this point, a+b+c+d+e is where the executable code begins */
    sec = (struct rd_sechdr64 *)buffer_data(c);
    sec->name = 0; /* .text appears first */
    sec->addr = a->len + b->len + c->len + d->len + e->len;
    sec->offset = sec->addr;
    sec->size = f->len;

    hdr->entry = a->len + b->len + c->len + d->len + e->len;
    hdr->proghdr = a->len;
    hdr->sectionhdr = a->len + b->len;

    prg->size_file = a->len + b->len + c->len + d->len + e->len + f->len;
    prg->size_mem = prg->size_file;

    buffer_merge(&buffer, 6, a, b, c, d, e, f);

    buffer_free(a);
    buffer_free(b);
    buffer_free(c);
    buffer_free(d);
    buffer_free(e);
    buffer_free(f);

    /* write file */
    fwrite(buffer_data(buffer), buffer->len, 1, fp);
    buffer_free(buffer);
    fclose(fp);

    /* done */
    return 0;
}