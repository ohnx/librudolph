#ifndef __LINK_H_INC
#define __LINK_H_INC
#ifdef RUDOLF_USE_STDLIB
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif
#include <rudolph/elf.h>

/* relocation management */
enum rd_link_relocation_type {
    RELOC_CODE, /* a code relocation will fill in a jump instruction */
    RELOC_DATA /* a data relocation will fill in an address to the data */
};

struct rd_link_relocation {
    /* the type of the relocation - code or data */
    enum rd_link_relocation_type type;

    /* the offset into the binary to perform this relocation at */
    size_t offset
}

#endif /* __LINK_H_INC */
