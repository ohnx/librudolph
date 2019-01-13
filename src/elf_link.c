#include <rudolph/elf_link.h>
#include <rudolph/elf.h>
#include <rudolph/buffer.h>

#ifdef RUDOLF_USE_STDLIB
/* for malloc(), free(), realloc() */
#include <stdlib.h>
/* for memcpy() */
#include <string.h>
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif


rd_buf_t *elf_link64(struct rd_elfhdr64 *hdr, uint32_t base_addr, rd_buf_t *code, rd_buf_t *data, struct rd_elf_link_relocation *relocs) {
    return NULL;
}

/* generate a header that can be further customized if necessary */
__inline struct rd_elfhdr64 *elf_link_genhdr(uint8_t bclass, uint16_t machine) {
    struct rd_elfhdr64 *hdr;

    hdr = (struct rd_elfhdr64 *)malloc(sizeof(struct rd_elfhdr64));
    if (!hdr) return hdr;

    /* magic words */
    hdr->magic[0] = 0x7F;
    hdr->magic[1] = 'E';
    hdr->magic[2] = 'L';
    hdr->magic[3] = 'F';

    /* 64 bit */
    hdr->bclass = bclass;

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
    hdr->prghdrsz = sizeof(struct rd_prghdr64);
    hdr->shdrsz = sizeof(struct rd_sechdr64);

    /* normally there is 1 program header */
    hdr->prghdrnbr = 1;

    /* normally there are 3 headers - 1 null, 1 for text (code), and 1 for section header names */
    hdr->shdrnbr = 3;

    /* section headers are at the 2nd index (0 based) */
    hdr->snshdridx = 2;

    return hdr;
}
