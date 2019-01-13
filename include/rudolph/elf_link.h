#ifndef __ELF_LINK_H_INC
#define __ELF_LINK_H_INC
#ifdef RUDOLF_USE_STDLIB
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif
#include <rudolph/elf.h>

/* relocation management */
enum rd_elf_link_relocation_type {
    RELOC_NULL = 0, /* used to signify end of list */
    RELOC_CODE32, /* a code relocation will fill in a jump instruction */
    RELOC_CODE64, /* a code relocation will fill in a jump instruction */
    RELOC_DATA8,  /* a data relocation will fill in an address to the data */
    RELOC_DATA16, /* a data relocation will fill in an address to the data */
    RELOC_DATA32, /* a data relocation will fill in an address to the data */
    RELOC_DATA64  /* a data relocation will fill in an address to the data */
};

struct rd_elf_link_relocation {
    /* the type of the relocation - code or data */
    enum rd_elf_link_relocation_type type;

    /* the offset into the binary to perform this relocation at */
    size_t target;

    /* the offset into the binary to fetch the data from */
    size_t data;
};


__inline struct rd_elfhdr64 *elf_link_genhdr(uint8_t bclass, uint16_t machine);

#endif /* __ELF_LINK_H_INC */
