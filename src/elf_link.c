#include <rudolph/elf_link.h>
#include <rudolph/elf.h>
#include <rudolph/buffer.h>
#include <rudolph/error.h>
#include <string.h>

/* TODO - allow custom base_addr's other than 4MiB?? */
/* TODO: proper memory alignment of 4 bytes instead of 1? */
int rd_elf_link64(uint16_t machine, rd_buf_t *text, rd_buf_t *data, size_t bss_len, struct rd_elf_link_relocation *relocs, rd_buf_t **res) {
    int n;
    size_t fdsec;
    rd_buf_t *a, *b, *c, *d, *e, *f, *g, *h, *i, *j, *k, *l;
    struct rd_elfhdr64 *hdr;
    struct rd_elf_prghdr64 *prg, *prg_rw;
    struct rd_elf_sechdr64 *sec_null, *sec_text, *sec_data, *sec_bss, *sec_shst;
    static const unsigned char shstrtab_strnull[] = "";
    static const unsigned char shstrtab_strtext[] = ".text";
    static const unsigned char shstrtab_strdata[] = ".data";
    static const unsigned char shstrtab_strbss[] = ".bss";
    static const unsigned char shstrtab_strshst[] = ".shstrtab";
    struct rd_elf_link_relocation *reloc;

    if (!res) { return RD_E_NONSENSE; }

    /* initialize variables */
    a = NULL; b = NULL; c = NULL; d = NULL; e = NULL; f = NULL; g = NULL;
    h = NULL; i = NULL; j = NULL; k = NULL; l = NULL;

    /* first, the header */
    n = rd_elf_link_genhdr64(machine, &a);
    if (n != RD_E_OK) goto die;
    hdr = (struct rd_elfhdr64 *)rd_buffer_data(a);

    /* now we make a program header to tell the system to load our executable and make it read/executable */
    n = rd_elf_link_genelfprg64(RD_ELF_PRGHDR_TYPE_LOAD, RD_ELF_PRGHDR_FLAGS_X | RD_ELF_PRGHDR_FLAGS_R, &b);
    if (n != RD_E_OK) goto die;
    prg = (struct rd_elf_prghdr64 *)rd_buffer_data(b);

    /* now we have the text section, but make a copy since we will modify it for relocations */
    c = rd_buffer_initsz(text->len);
    if (!c) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&c, rd_buffer_data(text), text->len);
    if (n != RD_E_OK) goto die;

    /* now we have the data section */
    d = data;

    /* now the null section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_NULL, &e);
    if (n != RD_E_OK) goto die;
    sec_null = (struct rd_elf_sechdr64 *)rd_buffer_data(e);

    /* text section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_PROGBITS, &f);
    if (n != RD_E_OK) goto die;
    sec_text = (struct rd_elf_sechdr64 *)rd_buffer_data(f);
    sec_text->flags |= RD_ELF_SECHDR_FLAGS_EXECINSTR;

    hdr->shdrnbr--;
    hdr->snshdridx--;

    /* data section, if applicable */
    if (d->len) {
        /* data section */
        n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_PROGBITS, &g);
        if (n != RD_E_OK) goto die;
        sec_data = (struct rd_elf_sechdr64 *)rd_buffer_data(g);
        sec_data->flags |= RD_ELF_SECHDR_FLAGS_WRITE;

        /* modify the number of section headers in the program */
        hdr->shdrnbr++;
        hdr->snshdridx++;
    } else {
        g = rd_buffer_init();
        sec_data = NULL;
    }

    /* bss section, if applicable */
    if (bss_len > 0) {
        /* bss section */
        n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_NOBITS, &j);
        if (n != RD_E_OK) goto die;
        sec_bss = (struct rd_elf_sechdr64 *)rd_buffer_data(j);
        sec_bss->flags |= RD_ELF_SECHDR_FLAGS_ALLOC | RD_ELF_SECHDR_FLAGS_WRITE;
        sec_bss->size = bss_len;

        /* modify the number of section headers in the program */
        hdr->shdrnbr++;
        hdr->snshdridx++;
    } else {
        j = rd_buffer_init();
        sec_bss = NULL;
    }

    /* also update the program headers if necessary */
    if (sec_data || sec_bss) {
        /* we either have a data section or a bss section so we need a RW program header */
        n = rd_elf_link_genelfprg64(RD_ELF_PRGHDR_TYPE_LOAD, RD_ELF_PRGHDR_FLAGS_W | RD_ELF_PRGHDR_FLAGS_R, &k);
        if (n != RD_E_OK) goto die;
        prg_rw = (struct rd_elf_prghdr64 *)rd_buffer_data(k);

        /* modify the number of program headers in the program */
        hdr->prghdrnbr++;
    } else {
        k = rd_buffer_init();
        prg_rw = NULL;
    }

    /* section header string section */
    n = rd_elf_link_genelfsec64(RD_ELF_SECHDR_TYPE_STRTAB, &h);
    if (n != RD_E_OK) goto die;
    sec_shst = (struct rd_elf_sechdr64 *)rd_buffer_data(h);

    /* section header strings themselves */
    i = rd_buffer_init();
    if (!i) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&i, shstrtab_strnull, sizeof(shstrtab_strnull));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strtext, sizeof(shstrtab_strtext));
    if (n != RD_E_OK) goto die;
    if (sec_data) {
        n = rd_buffer_push(&i, shstrtab_strdata, sizeof(shstrtab_strdata));
        if (n != RD_E_OK) goto die;
    }
    if (sec_bss) {
        n = rd_buffer_push(&i, shstrtab_strbss, sizeof(shstrtab_strbss));
        if (n != RD_E_OK) goto die;
    }
    n = rd_buffer_push(&i, shstrtab_strshst, sizeof(shstrtab_strshst));
    if (n != RD_E_OK) goto die;

    /* done relocating! time to write in all the values now that we know them */
    /* start with string offsets for section header names */

    /* null section */
    /* null section name will point to the first null */
    sec_null->name = 0;

    /* section header string section */
    /* section header string table name will point to the right addr */
    sec_shst->name = sizeof(shstrtab_strnull) + sizeof(shstrtab_strtext);
    /* offset for the data for this section is basically the entire rest of file */
    sec_shst->offset = a->len + b->len + k->len + c->len + d->len + e->len + f->len + g->len + j->len + h->len;
    /* and it is i long */
    sec_shst->size = i->len;

    /* bss section */
    if (sec_bss) {
        /* adjust shst name location */
        sec_shst->name += sizeof(shstrtab_strbss);
        /* name is after .text or .data */
        sec_bss->name = sizeof(shstrtab_strnull) + sizeof(shstrtab_strtext);
        /* offset is after the main header, program header, code (text), and data */
        sec_bss->offset = a->len + b->len + k->len + c->len + d->len;
        /* this section will be mapped into memory at the same offset from the base memory */
        sec_bss->addr = prg->vaddr + sec_bss->offset;
    }

    /* data section */
    if (sec_data) {
        /* adjust shst name location */
        sec_shst->name += sizeof(shstrtab_strdata);
        /* adjust bss name location */
        if (sec_bss) sec_bss->name += sizeof(shstrtab_strdata);
        /* name is after .text */
        sec_data->name = sizeof(shstrtab_strnull) + sizeof(shstrtab_strtext);
        /* offset is after the main header, program header, and code (text) */
        sec_data->offset = a->len + b->len + k->len + c->len;
        /* this section will be mapped into memory at the same offset from the base memory */
        sec_data->addr = prg->vaddr + sec_data->offset;
        /* data length is as expected */
        sec_data->size = d->len;
    }

    /* text section */
    /* name is after the null character */
    sec_text->name = sizeof(shstrtab_strnull);
    /* offset is after the main header and program headers */
    sec_text->offset = a->len + b->len + k->len;
    /* this section will be mapped into memory at the same offset from the base memory */
    sec_text->addr = prg->vaddr + sec_text->offset;
    /* code length is as expected */
    sec_text->size = c->len;

    /* program header */
    /* only bother loading the interesting parts (ignore section headers) */
    prg->size_file = a->len + b->len + c->len + k->len;
    /* size in memory is the same as size on the file */
    prg->size_mem = prg->size_file;

    /* calculate where the prg_rw will be in memory */
    fdsec = prg->vaddr + prg->size_mem;
    if (prg_rw) {
        /* adjust fdsec to be aligned on the nearest alignment */
        fdsec = ((fdsec + prg_rw->alignment) & (~(prg_rw->alignment - 1)));

        /* set the values for the read/write area program header */
        prg_rw->offset = (prg->offset + prg->size_file + prg_rw->alignment) & (~(prg_rw->alignment - 1));
        prg_rw->vaddr = prg_rw->paddr = fdsec;
        prg_rw->size_file = 0;
        prg_rw->size_mem = 0;

        /* set the correct starting address for data/bss sections */
        if (sec_data) {
            /* build a buffer to pad the space between .text and .data in the file */
            l = rd_buffer_initsz(prg_rw->offset - (prg->offset + prg->size_file));
            l->len = l->alloc;
            memset(rd_buffer_data(l), 0, l->alloc);
            sec_data->addr = fdsec;
            prg_rw->size_file += sec_data->size;
            prg_rw->size_mem += sec_data->size;

            if (sec_bss) {
                sec_bss->addr = sec_data->addr + sec_data->size;
                prg_rw->size_mem += sec_bss->size;
            }

            /* adjust offsets of everything after padding l, too */
            /* d, e, f, g, j, h, i */
            sec_shst->offset += l->len;
            sec_data->offset += l->len;
            if (sec_bss) sec_bss->offset += l->len;
        } else {
            /* only bss */
            sec_bss->addr = fdsec;
            prg_rw->size_mem += sec_bss->size;
            l = rd_buffer_init();
        }
    }

    /* now... the fun begins! relocations! */
    for (reloc = relocs; reloc && reloc->type != RELOC_NULL; reloc++) {
        switch (reloc->type) {
#define rd_reloc(size, target, source) *((size *)(((unsigned char *)rd_buffer_data(c))+target)) = (size)(source);
        /* 1st type: we are relocating from the text section into the text section. */
        /* src needs to have sec_text->addr added to it and stored at target */
        case RELOC_TEXT32:
            rd_reloc(uint32_t, reloc->target, sec_text->addr + reloc->src);
            break;
        case RELOC_TEXT64:
            rd_reloc(uint64_t, reloc->target, sec_text->addr + reloc->src);
            break;
        /* 2nd type: we are relocating from the data section into the text section. */
        /* src needs to have sec_data->addr added to it and stored at target */
        case RELOC_DATA32:
            if (!sec_data) { n = RD_E_UNKNOWN_RELOC; goto die; }
            rd_reloc(uint32_t, reloc->target, sec_data->addr + reloc->src);
            break;
        case RELOC_DATA64:
            if (!sec_data) { n = RD_E_UNKNOWN_RELOC; goto die; }
            rd_reloc(uint64_t, reloc->target, sec_data->addr + reloc->src);
            break;
        /* 3rd type: we are relocating from the bss section into the text section. */
        /* src needs to have sec_bss->addr added to it and stored at target */
        case RELOC_BSS32:
            if (!sec_bss) { n = RD_E_UNKNOWN_RELOC; goto die; }
            rd_reloc(uint32_t, reloc->target, sec_bss->addr + reloc->src);
            break;
        case RELOC_BSS64:
            if (!sec_bss) { n = RD_E_UNKNOWN_RELOC; goto die; }
            rd_reloc(uint64_t, reloc->target, sec_bss->addr + reloc->src);
            break;
        /* 4th type: custom relocation: just write whatever value we are given */
        case RELOC_CUSTOM8:
            rd_reloc(uint8_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM16:
            rd_reloc(uint16_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM32:
            rd_reloc(uint32_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM64:
            rd_reloc(uint64_t, reloc->target, reloc->src);
            break;
        /* unknown relocation type */
        default:
            n = RD_E_UNKNOWN_RELOC;
            goto die;
#undef rd_reloc
        }
    }

    /* main elf header */
    /* entrypoint is at sec_text address */
    hdr->entry = sec_text->addr;
    /* program headers are located after this header */
    hdr->proghdr = a->len;
    /* section headers are located after a,b,k,c,d (program headers and code) */
    hdr->sectionhdr = a->len + b->len + k->len + c->len + l->len + d->len;

    /* ... and finally, join all the buffers together! */
    if (*res) *res = rd_buffer_init();
    n = rd_buffer_merge(res, 12, a, b, k, c, l, d, e, f, g, j, h, i);

    /* TODO: make bss work.
    
    Will need to:

    * Think over again how stuff works... where exactly is stuff in memory?!?
    * Fix section header strings
    * Add a new program header for bss that is R/W, 0 size in file, and blah size in memory
    
    */

die:
    if (a) rd_buffer_free(a);
    if (b) rd_buffer_free(b);
    if (c) rd_buffer_free(c);
    /* we don't touch d since it is user-supplied */
    if (e) rd_buffer_free(e);
    if (f) rd_buffer_free(f);
    if (g) rd_buffer_free(g);
    if (h) rd_buffer_free(h);
    if (i) rd_buffer_free(i);
    if (j) rd_buffer_free(j);
    if (k) rd_buffer_free(k);
    if (l) rd_buffer_free(l);

    return n;
}

/* ... and now basically the same exact thing but for 32 bit ... */
int rd_elf_link32(uint16_t machine, rd_buf_t *text, rd_buf_t *data, struct rd_elf_link_relocation *relocs, rd_buf_t **res) {
    int n;
    rd_buf_t *a, *b, *c, *d, *e, *f, *g, *h, *i;
    struct rd_elfhdr32 *hdr;
    struct rd_elf_prghdr32 *prg;
    struct rd_elf_sechdr32 *sec_null, *sec_text, *sec_data, *sec_shst;
    static const unsigned char shstrtab_strnull[] = "";
    static const unsigned char shstrtab_strtext[] = ".text";
    static const unsigned char shstrtab_strdata[] = ".data";
    static const unsigned char shstrtab_strshst[] = ".shstrtab";
    struct rd_elf_link_relocation *reloc;

    if (!res) { return RD_E_NONSENSE; }

    /* initialize variables */
    a = NULL; b = NULL; c = NULL; d = NULL; e = NULL; f = NULL; g = NULL; h = NULL; i = NULL;

    /* first, the header */
    n = rd_elf_link_genhdr32(machine, &a);
    if (n != RD_E_OK) goto die;
    hdr = (struct rd_elfhdr32 *)rd_buffer_data(a);

    /* now we make a program header to tell the system to load our executable and make it read/executable */
    n = rd_elf_link_genelfprg32(RD_ELF_PRGHDR_TYPE_LOAD, RD_ELF_PRGHDR_FLAGS_X | RD_ELF_PRGHDR_FLAGS_R, &b);
    if (n != RD_E_OK) goto die;
    prg = (struct rd_elf_prghdr32 *)rd_buffer_data(b);

    /* now we have the text section, but make a copy since we will modify it for relocations */
    c = rd_buffer_initsz(text->len);
    if (!c) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&c, rd_buffer_data(text), text->len);
    if (n != RD_E_OK) goto die;

    /* now we have the data section */
    d = data;

    /* now the null section */
    n = rd_elf_link_genelfsec32(RD_ELF_SECHDR_TYPE_NULL, &e);
    if (n != RD_E_OK) goto die;
    sec_null = (struct rd_elf_sechdr32 *)rd_buffer_data(e);

    /* text section */
    n = rd_elf_link_genelfsec32(RD_ELF_SECHDR_TYPE_PROGBITS, &f);
    if (n != RD_E_OK) goto die;
    sec_text = (struct rd_elf_sechdr32 *)rd_buffer_data(f);
    sec_text->flags |= RD_ELF_SECHDR_FLAGS_EXECINSTR;

    /* data section */
    n = rd_elf_link_genelfsec32(RD_ELF_SECHDR_TYPE_PROGBITS, &g);
    if (n != RD_E_OK) goto die;
    sec_data = (struct rd_elf_sechdr32 *)rd_buffer_data(g);
    sec_data->flags |= RD_ELF_SECHDR_FLAGS_WRITE;

    /* section header string section */
    n = rd_elf_link_genelfsec32(RD_ELF_SECHDR_TYPE_STRTAB, &h);
    if (n != RD_E_OK) goto die;
    sec_shst = (struct rd_elf_sechdr32 *)rd_buffer_data(h);

    /* section header strings themselves */
    i = rd_buffer_init();
    if (!i) { n = RD_E_OOM; goto die; }
    n = rd_buffer_push(&i, shstrtab_strnull, sizeof(shstrtab_strnull));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strtext, sizeof(shstrtab_strtext));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strdata, sizeof(shstrtab_strdata));
    if (n != RD_E_OK) goto die;
    n = rd_buffer_push(&i, shstrtab_strshst, sizeof(shstrtab_strshst));
    if (n != RD_E_OK) goto die;

    /* now... the fun begins! relocations! */
    for (reloc = relocs; reloc && reloc->type != RELOC_NULL; reloc++) {
        switch (reloc->type) {
#define rd_reloc(size, target, source) *((size *)(((unsigned char *)rd_buffer_data(c))+target)) = (size)source;
        /* 1st type: we are relocating from the text section into the text section. */
        /* src needs to have prg->vaddr + a->len + b->len added to it and stored at target */
        case RELOC_TEXT32:
            rd_reloc(uint32_t, reloc->target, prg->vaddr + a->len + b->len + reloc->src);
            break;
        case RELOC_TEXT64:
            rd_reloc(uint64_t, reloc->target, prg->vaddr + a->len + b->len + reloc->src);
            break;
        /* 2nd type: we are relocating from the data section into the text section. */
        /* src needs to have prg->vaddr + a->len + b->len + c->len added to it and stored at target */
        case RELOC_DATA32:
            rd_reloc(uint32_t, reloc->target, prg->vaddr + a->len + b->len + c->len + reloc->src);
            break;
        case RELOC_DATA64:
            rd_reloc(uint64_t, reloc->target, prg->vaddr + a->len + b->len + c->len + reloc->src);
            break;
        /* 3rd type: custom relocation: just write whatever value we are given */
        case RELOC_CUSTOM8:
            rd_reloc(uint8_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM16:
            rd_reloc(uint16_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM32:
            rd_reloc(uint32_t, reloc->target, reloc->src);
            break;
        case RELOC_CUSTOM64:
            rd_reloc(uint64_t, reloc->target, reloc->src);
            break;
        /* unknown relocation type */
        default:
            n = RD_E_UNKNOWN_RELOC;
            goto die;
#undef rd_reloc
        }
    }

    /* done relocating! time to write in all the values now that we know them */
    /* start with string offsets for section header names */

    /* null section */
    /* null section name will point to the first null */
    sec_null->name = 0;

    /* section header string section */
    /* section header string table name will point to the right addr */
    sec_shst->name = sizeof(shstrtab_strnull) + sizeof(shstrtab_strtext) + sizeof(shstrtab_strdata);
    /* offset for the data for this section is basically the entire rest of file */
    sec_shst->offset = a->len + b->len + c->len + d->len + e->len + f->len + g->len + h->len;
    /* and it is i long */
    sec_shst->size = i->len;

    /* data section */
    /* name is after .text */
    sec_data->name = sizeof(shstrtab_strnull) + sizeof(shstrtab_strtext);
    /* offset is after the main header, program header, and code (text) */
    sec_data->offset = a->len + b->len + c->len;
    /* this section will be mapped into memory at the same offset from the base memory */
    sec_data->addr = prg->vaddr + sec_data->offset;
    /* data length is as expected */
    sec_data->size = d->len;

    /* text section */
    /* name is after the null character */
    sec_text->name = sizeof(shstrtab_strnull);
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
    /* we don't touch d since it is user-supplied */
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

    /* padding */
    hdr->pad[0] = hdr->pad[1] = hdr->pad[2] = hdr->pad[3] = hdr->pad[4] = hdr->pad[5] = hdr->pad[6] = 0;

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

    /* normally there are 4 headers - 1 null, 1 for text (code), 1 for data, and 1 for section header names */
    hdr->shdrnbr = 4;

    /* section headers are at the 3rd index (0 based) */
    hdr->snshdridx = 3;

    return RD_E_OK;
}

__inline int rd_elf_link_genhdr32(uint16_t machine, rd_buf_t **ret) {
    struct rd_elfhdr32 *hdr;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elfhdr64), ret);

    /* cast to pointer to header */
    hdr = (struct rd_elfhdr32 *)rd_buffer_data(*ret);

    /* magic words */
    hdr->magic[0] = 0x7F;
    hdr->magic[1] = 'E';
    hdr->magic[2] = 'L';
    hdr->magic[3] = 'F';

    /* 32 bit */
    hdr->bclass = RD_ELFHDR_BCLASS_32;

    /* little endian */
    hdr->endianness = RD_ELFHDR_ENDIANNESS_LITTLE;

    /* version */
    hdr->version = 1;

    /* linux uses sysv a lot of the time */
    hdr->abi = RD_ELFHDR_ABI_SYSV;
    hdr->abi_version = 0;

    /* padding */
    hdr->pad[0] = hdr->pad[1] = hdr->pad[2] = hdr->pad[3] = hdr->pad[4] = hdr->pad[5] = hdr->pad[6] = 0;

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

    /* no special flags unless we are compiling for arm */
    switch (hdr->machine) {
    case RD_ELFHDR_MACHINE_ARM:
        hdr->flags = 0x5000200; /* Version5 EABI, soft-float ABI */
        /* could use 0x5000400 for hard floats */
        break;
    default:
        hdr->flags = 0;
        break;
    }

    /* sizes of structures */
    hdr->hdrsz = sizeof(struct rd_elfhdr32);
    hdr->prghdrsz = sizeof(struct rd_elf_prghdr32);
    hdr->shdrsz = sizeof(struct rd_elf_sechdr32);

    /* normally there is 1 program header */
    hdr->prghdrnbr = 1;

    /* normally there are 4 headers - 1 null, 1 for text (code), 1 for data, and 1 for section header names */
    hdr->shdrnbr = 4;

    /* section headers are at the 3rd index (0 based) */
    hdr->snshdridx = 3;

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

    /* align to the nearest 4KiB block (page boundary) */
    prg->alignment = 0x1000;

    return RD_E_OK;
}

__inline int rd_elf_link_genelfprg32(uint32_t type, uint32_t flags, rd_buf_t **ret) {
    struct rd_elf_prghdr32 *prg;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elf_prghdr32), ret);

    /* cast to pointer to header */
    prg = (struct rd_elf_prghdr32 *)rd_buffer_data(*ret);

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

    /* align to the nearest 4KiB block (page boundary) */
    prg->alignment = 0x1000;

    return RD_E_OK;
}

/* sections */
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
        sec->flags = RD_ELF_SECHDR_FLAGS_ALLOC;
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

__inline int rd_elf_link_genelfsec32(uint32_t type, rd_buf_t **ret) {
    struct rd_elf_sechdr32 *sec;

    /* initial allocation/sanity checks */
    rd_allocbuf(sizeof(struct rd_elf_sechdr32), ret);

    /* cast to pointer to header */
    sec = (struct rd_elf_sechdr32 *)rd_buffer_data(*ret);

    /* pointer to name is unknown right now */
    sec->name = 0;

    /* this section contains program code */
    sec->type = type;

    /* be helpful and add some default flags if we can */
    switch (type) {
    case RD_ELF_SECHDR_TYPE_PROGBITS:
        sec->flags = RD_ELF_SECHDR_FLAGS_ALLOC;
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

