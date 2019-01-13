#ifndef __RD_ELF_LINK_H_INC
#define __RD_ELF_LINK_H_INC
#include <rudolph/buffer.h>

#ifdef RUDOLF_USE_STDLIB
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif
#include <rudolph/elf.h>

/* relocation management */
enum rd_elf_link_relocation_type {
    RELOC_NULL = 0,     /* used to signify end of list */
    RELOC_TEXT32,       /* an absolute text relocation will fill in an address to the text (code) */
    RELOC_TEXT64,       /* an absolute text relocation will fill in an address to the text (code) */
    RELOC_DATA8,        /* a data relocation will fill in an address to the data */
    RELOC_DATA16,       /* a data relocation will fill in an address to the data */
    RELOC_DATA32,       /* a data relocation will fill in an address to the data */
    RELOC_DATA64,       /* a data relocation will fill in an address to the data */
    RELOC_CUSTOM8,      /* a custom relocation will write src to target */
    RELOC_CUSTOM16,     /* a custom relocation will write src to target */
    RELOC_CUSTOM32,     /* a custom relocation will write src to target */
    RELOC_CUSTOM64      /* a custom relocation will write src to target */
};

struct rd_elf_link_relocation {
    /* the type of the relocation - code or data */
    enum rd_elf_link_relocation_type type;

    /* the offset into the binary to perform this relocation at */
    size_t target;

    /* the offset into the text/data that the target *should* point to after relocation */
    size_t src;
};


int rd_elf_link64(uint16_t machine, rd_buf_t *text, rd_buf_t *data, struct rd_elf_link_relocation *relocs, rd_buf_t **res);
__inline int rd_elf_link_genhdr64(uint16_t machine, rd_buf_t **ret);
__inline int rd_elf_link_genelfprg64(uint32_t type, uint32_t flags, rd_buf_t **ret);
__inline int rd_elf_link_genelfsec64(uint32_t type, rd_buf_t **ret);

int rd_elf_link32(uint16_t machine, rd_buf_t *text, rd_buf_t *data, struct rd_elf_link_relocation *relocs, rd_buf_t **res);
__inline int rd_elf_link_genhdr32(uint16_t machine, rd_buf_t **ret);
__inline int rd_elf_link_genelfprg32(uint32_t type, uint32_t flags, rd_buf_t **ret);
__inline int rd_elf_link_genelfsec32(uint32_t type, rd_buf_t **ret);

#endif /* __RD_ELF_LINK_H_INC */
