#include <rudolph/elf_link.h>
#include <rudolph/elf.h>
#include <rudolph/buffer.h>
#include <rudolph/error.h>

/* TODO - allow custom base_addr's other than 4MiB?? */
int rd_elf_link64(uint16_t machine, rd_buf_t *text, rd_buf_t *data, struct rd_elf_link_relocation *relocs, rd_buf_t **res) {
    int n;
    rd_buf_t *a, *b, *c, *d, *e, *f, *g, *h, *i;
    struct rd_elfhdr64 *hdr;
    struct rd_elf_prghdr64 *prg;
    struct rd_elf_sechdr64 *sec_null, *sec_text, /* todo nyi *sec_data, */*sec_shst;
    static const unsigned char shstrtab_strtext[] = ".text";
    static const unsigned char shstrtab_strdata[] = ".data";
    static const unsigned char shstrtab_strshst[] = ".shstrtab";
    struct rd_elf_link_relocation *reloc;

    if (!res) { return RD_E_NONSENSE; }

    /* TODO - add support for data section */
    if (data) { return RD_E_NONSENSE; }

    /* initialize variables */
    a = NULL; b = NULL; c = NULL; d = NULL; e = NULL; f = NULL; g = NULL; h = NULL; i = NULL;

    /* first, the header */
    n = rd_elf_link_genhdr64(machine, &a);
    if (n != RD_E_OK) goto die;
    hdr = (struct rd_elfhdr64 *)rd_buffer_data(a);

    /* now we make a program header to tell the system to load our executable and make it read/executable */
    n = rd_elf_link_genelfprg64(RD_ELF_PRGHDR_TYPE_LOAD, RD_ELF_PRGHDR_FLAGS_X | RD_ELF_PRGHDR_FLAGS_R, &b);
    if (n != RD_E_OK) goto die;
    prg = (struct rd_elf_prghdr64 *)rd_buffer_data(b);

    /* now we have the code section, but make a copy since we will modify it for relocations */
    c = rd_buffer_initsz(text->len);
    if (!c) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&c, rd_buffer_data(text), text->len);
    if (n != RD_E_OK) goto die;

    /* todo nyi - data */
    d = rd_buffer_init();
    if (!d) { n = RD_E_OOM; goto die; }

    /* now the null section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_NULL, &e);
    if (n != RD_E_OK) goto die;
    sec_null = (struct rd_elf_sechdr64 *)rd_buffer_data(e);

    /* text section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_PROGBITS, &f);
    if (n != RD_E_OK) goto die;
    sec_text = (struct rd_elf_sechdr64 *)rd_buffer_data(f);

    /* todo nyi - data section is g */
    g = rd_buffer_init();
    if (!g) { n = RD_E_OOM; goto die; }

    /* section header string section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_STRTAB, &h);
    if (n != RD_E_OK) goto die;
    sec_shst = (struct rd_elf_sechdr64 *)rd_buffer_data(h);

    /* section header strings themselves */
    i = rd_buffer_init();
    if (!i) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&i, shstrtab_strtext, sizeof(shstrtab_strtext));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strdata, sizeof(shstrtab_strdata));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strshst, sizeof(shstrtab_strshst));
    if (n != RD_E_OK) goto die;

    /* now... the fun begins! relocations! */
    for (reloc = relocs; reloc && reloc->type != RELOC_NULL; reloc++) {
        switch (reloc->type) {
#define rd_reloc(size, target, source) *((size *)(((unsigned char *)rd_buffer_data(c))+target)) = source;
        /* 1st type: we are relocating from the text section into the text section. */
        /* src needs to have a->len + b->len added to it and stored at target */
        case RELOC_TEXT8:
            rd_reloc(uint8_t, reloc->target, a->len + b->len + reloc->src);
            break;
        case RELOC_TEXT16:
            rd_reloc(uint16_t, reloc->target, a->len + b->len + reloc->src);
            break;
        case RELOC_TEXT32:
            rd_reloc(uint32_t, reloc->target, a->len + b->len + reloc->src);
            break;
        case RELOC_TEXT64:
            rd_reloc(uint64_t, reloc->target, a->len + b->len + reloc->src);
            break;
        /* 2nd type: we are relocating from the data section into the text section. */
        /* src needs to have a->len + b->len + c->len added to it and stored at target */
        case RELOC_DATA8:
            rd_reloc(uint8_t, reloc->target, a->len + b->len + c->len + reloc->src);
            break;
        case RELOC_DATA16:
            rd_reloc(uint16_t, reloc->target, a->len + b->len + c->len + reloc->src);
            break;
        case RELOC_DATA32:
            rd_reloc(uint32_t, reloc->target, a->len + b->len + c->len + reloc->src);
            break;
        case RELOC_DATA64:
            rd_reloc(uint64_t, reloc->target, a->len + b->len + c->len + reloc->src);
            break;
        default:
            n = RD_E_UNKNOWN_RELOC;
            goto die;
#undef rd_reloc
        }
    }

    /* done relocating! time to write in all the values now that we know them */
    /* start with string offsets for section header names */

    /* null section */
    /* null section name will point to the null after .text */
    sec_null->name = sizeof(shstrtab_strtext)-1;

    /* section header string section */
    /* section header string table name will point to the right addr */
    sec_shst->name = sizeof(shstrtab_strtext) + sizeof(shstrtab_strdata);
    /* offset for the data for this section is basically the entire rest of file */
    sec_shst->offset = a->len + b->len + c->len + d->len + e->len + f->len + g->len + h->len;
    /* and it is i long */
    sec_shst->size = i->len;

    /* text section */
    /* name is just 0 since .text appears first */
    sec_text->name = 0;
    /* offset is after the main header and program header */
    sec_text->offset = a->len + b->len;
    /* this section will be mapped into memory at the same offset from the base memory */
    sec_text->addr = prg->vaddr + sec_text->offset;
    /* code length is as expected */
    sec_text->size = c->len;

    /* program header */
    /* only bother loading the interesting parts (ignore section headers) */
    prg->size_file = a->len + b->len + c->len + d->len;
    /* size in memory is the same as size on the file unless we have bss (we don't) */
    prg->size_mem = prg->size_file;

    /* main elf header */
    /* entrypoint is at sec_text address */
    hdr->entry = sec_text->addr;
    /* program headers are located after this header */
    hdr->proghdr = a->len;
    /* section headers are located after a,b,c,d */
    hdr->sectionhdr = a->len + b->len + c->len + d->len;

    /* ... and finally, join all the buffers together! */
    if (*res) *res = rd_buffer_init();
    n = rd_buffer_merge(res, 9, a, b, c, d, e, f, g, h, i);

die:
    if (a) rd_buffer_free(a);
    if (b) rd_buffer_free(b);
    if (c) rd_buffer_free(c);
    if (d) rd_buffer_free(d);
    if (e) rd_buffer_free(e);
    if (f) rd_buffer_free(f);
    if (g) rd_buffer_free(g);
    if (h) rd_buffer_free(h);
    if (i) rd_buffer_free(i);

    return n;
}

#define rd_allocbuf(x, r) do {                                                 \
    /* initial allocation/sanity checks */                                     \
    if (!r) return RD_E_NONSENSE;                                              \
    *r = rd_buffer_initsz(x);                                                  \
    if (!(*r)) return RD_E_OOM;                                                \
    (*r)->len = (*r)->alloc;                                                   \
} while (0);

/* generate a header that can be further customized if necessary */
__inline int rd_elf_link_genhdr64(uint16_t machine, rd_buf_t **ret) {
    struct rd_elfhdr64 *hdr;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elfhdr64), ret);

    /* cast to pointer to header */
    hdr = (struct rd_elfhdr64 *)rd_buffer_data(*ret);

    /* magic words */
    hdr->magic[0] = 0x7F;
    hdr->magic[1] = 'E';
    hdr->magic[2] = 'L';
    hdr->magic[3] = 'F';

    /* 64 bit */
    hdr->bclass = RD_ELFHDR_BCLASS_64;

    /* little endian */
    hdr->endianness = RD_ELFHDR_ENDIANNESS_LITTLE;

    /* version */
    hdr->version = 1;

    /* linux uses sysv a lot of the time */
    hdr->abi = RD_ELFHDR_ABI_SYSV;
    hdr->abi_version = 0;

    /* static executable */
    hdr->type = RD_ELFHDR_TYPE_EXEC;

    /* type of machine code */
    hdr->machine = machine;

    /* version, again */
    hdr->lversion = 1;

    /* address of executable code (tbd) */
    hdr->entry = 0;

    /* address of program headers (tbd) */
    hdr->proghdr = 0;

    /* address of section headers (tbd) */
    hdr->sectionhdr = 0;

    /* no special flags */
    hdr->flags = 0;

    /* sizes of structures */
    hdr->hdrsz = sizeof(struct rd_elfhdr64);
    hdr->prghdrsz = sizeof(struct rd_elf_prghdr64);
    hdr->shdrsz = sizeof(struct rd_elf_sechdr64);

    /* normally there is 1 program header */
    hdr->prghdrnbr = 1;

    /* normally there are 3 headers - 1 null, 1 for text (code), and 1 for section header names */
    hdr->shdrnbr = 3;

    /* section headers are at the 2nd index (0 based) */
    hdr->snshdridx = 2;

    return RD_E_OK;
}

/* mapping to 0x0000000000400000 (4MiB) prevents segfaults, i think */
#define __RD_CONST_BASEADDR 0x0000000000400000
__inline int rd_elf_link_genelfprg64(uint32_t type, uint32_t flags, rd_buf_t **ret) {
    struct rd_elf_prghdr64 *prg;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elf_prghdr64), ret);

    /* cast to pointer to header */
    prg = (struct rd_elf_prghdr64 *)rd_buffer_data(*ret);

    /* set the type */
    prg->type = type;

    /* flags for the section */
    prg->flags = flags;

    /* offset in the file - we can just map starting from the base of the file */
    prg->offset = 0x0;

    /* base address for the program in memory */
    prg->vaddr = __RD_CONST_BASEADDR;

    /* physical address is not used so set it to same as virtual */
    prg->paddr = prg->vaddr;

    /* tbd; size of the entire file */
    prg->size_file = 0;

    /* tbd; size of the block in memory */
    prg->size_mem = 0;

    /* align to the nearest 2MiB block - TODO isn't this just the page boundary? */
    prg->alignment = 0x200000;

    return RD_E_OK;
}

__inline int rd_elf_link_genelfsec64(uint32_t type, rd_buf_t **ret) {
    struct rd_elf_sechdr64 *sec;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elf_sechdr64), ret);

    /* cast to pointer to header */
    sec = (struct rd_elf_sechdr64 *)rd_buffer_data(*ret);

    /* pointer to name is unknown right now */
    sec->name = 0;

    /* this section contains program code */
    sec->type = type;

    /* be helpful and add some default flags if we can */
    switch (type) {
    case RD_ELF_SECHDR_TYPE_PROGBITS:
        sec->flags = RD_ELF_SECHDR_FLAGS_ALLOC | RD_ELF_SECHDR_FLAGS_EXECINSTR;
        break;
    default:
        sec->flags = 0;
        break;
    }

    /* where the section begins when mapped into memory */
    sec->addr = 0;

    /* offset of the section in the file */
    sec->offset = 0;

    /* size of the section in the file */
    sec->size = 0;

    /* no related sections */
    sec->link = 0;

    /* no extra info */
    sec->info = 0;

    /* align to the nearest byte most of the time */
    sec->align = 1;

    /* ?? */
    sec->entsz = 0;

    return RD_E_OK;
}

#undef rd_allocbuf
