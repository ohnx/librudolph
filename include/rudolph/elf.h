#ifndef __RD_ELF_INC_H
#define __RD_ELF_INC_H

#ifdef RUDOLF_USE_STDLIB
/* for uint types */
#include <stdint.h>
#else
#include <rudolph/own_stdlib.h>
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=[ main elf header ]=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#define __RD_ELF_HEADER_1                                                      \
    /* 0x7F ELF */                                                             \
    uint8_t magic[4];                                                          \
                                                                               \
    /* 32 or 64 bits */                                                        \
    uint8_t bclass;                                                            \
                                                                               \
    /* big or little endian */                                                 \
    uint8_t endianness;                                                        \
                                                                               \
    /* version = 1 */                                                          \
    uint8_t version;                                                           \
                                                                               \
    /* ABI (aka OS) */                                                         \
    uint8_t abi;                                                               \
                                                                               \
    /* ABI version */                                                          \
    uint8_t abi_version;                                                       \
                                                                               \
    /* padding */                                                              \
    uint8_t pad[7];                                                            \
                                                                               \
    /* object file type */                                                     \
    uint16_t type;                                                             \
                                                                               \
    /* target ISA */                                                           \
    uint16_t machine;                                                          \
                                                                               \
    /* version = 1 again (this time as a long) */                              \
    uint32_t lversion;

#define __RD_ELF_HEADER_2                                                      \
    /* flags */                                                                \
    uint32_t flags;                                                            \
                                                                               \
    /* header size */                                                          \
    uint16_t hdrsz;                                                            \
                                                                               \
    /* program header size */                                                  \
    uint16_t prghdrsz;                                                         \
                                                                               \
    /* number of program headers */                                            \
    uint16_t prghdrnbr;                                                        \
                                                                               \
    /* section header size */                                                  \
    uint16_t shdrsz;                                                           \
                                                                               \
    /* number of section headers */                                            \
    uint16_t shdrnbr;                                                          \
                                                                               \
    /* index of the section header that contains section names */              \
    uint16_t snshdridx;

struct rd_elfhdr32 {
    __RD_ELF_HEADER_1

    /* entry point */
    uint32_t entry;

    /* program header */
    uint32_t proghdr;

    /* section header */
    uint32_t sectionhdr;

    __RD_ELF_HEADER_2
};

struct rd_elfhdr64 {
    __RD_ELF_HEADER_1

    /* entry point */
    uint64_t entry;

    /* program header */
    uint64_t proghdr;

    /* section header */
    uint64_t sectionhdr;

    __RD_ELF_HEADER_2
};

#undef __RD_ELF_HEADER_1
#undef __RD_ELF_HEADER_2

#define RD_ELFHDR_BCLASS_32     0x01
#define RD_ELFHDR_BCLASS_64     0x02

#define RD_ELFHDR_ENDIANNESS_LITTLE     0x01
#define RD_ELFHDR_ENDIANNESS_BIG        0x02

#define RD_ELFHDR_ABI_SYSV          0x00
#define RD_ELFHDR_ABI_HPUX          0x01
#define RD_ELFHDR_ABI_NETBSD        0x02
#define RD_ELFHDR_ABI_LINUX         0x03
#define RD_ELFHDR_ABI_HURD          0x04
#define RD_ELFHDR_ABI_SOLARIS       0x06
#define RD_ELFHDR_ABI_AIX           0x07
#define RD_ELFHDR_ABI_IRIX          0x08
#define RD_ELFHDR_ABI_FREEBSD       0x09
#define RD_ELFHDR_ABI_TRU64         0x0a
#define RD_ELFHDR_ABI_MODESTO       0x0b
#define RD_ELFHDR_ABI_OPENBSD       0x0c
#define RD_ELFHDR_ABI_OPENVMS       0x0d
#define RD_ELFHDR_ABI_NONSTOP       0x0e
#define RD_ELFHDR_ABI_AROS          0x0f
#define RD_ELFHDR_ABI_FENIX         0x10
#define RD_ELFHDR_ABI_CLOUDABI      0x11

#define RD_ELFHDR_TYPE_NONE         0x0000
#define RD_ELFHDR_TYPE_REL          0x0001
#define RD_ELFHDR_TYPE_EXEC         0x0002
#define RD_ELFHDR_TYPE_DYN          0x0003
#define RD_ELFHDR_TYPE_CORE         0x0004
#define RD_ELFHDR_TYPE_LOOS         0xfe00
#define RD_ELFHDR_TYPE_HIOS         0xfeff
#define RD_ELFHDR_TYPE_LOPROC       0xff00
#define RD_ELFHDR_TYPE_HIPROC       0xffff

#define RD_ELFHDR_MACHINE_NONE      0x0000
#define RD_ELFHDR_MACHINE_SPARC     0x0002
#define RD_ELFHDR_MACHINE_386       0x0003
#define RD_ELFHDR_MACHINE_MIPS      0x0008
#define RD_ELFHDR_MACHINE_POWERPC   0x0014
#define RD_ELFHDR_MACHINE_S390      0x0016
#define RD_ELFHDR_MACHINE_ARM       0x0028
#define RD_ELFHDR_MACHINE_SUPERH    0x002a
#define RD_ELFHDR_MACHINE_IA64      0x0032
#define RD_ELFHDR_MACHINE_X86_64    0x003e
#define RD_ELFHDR_MACHINE_AARCH64   0x00b7
#define RD_ELFHDR_MACHINE_RISCV     0x00f3

#define RD_ELFHDR_HDSRZ_32          0x0034
#define RD_ELFHDR_HDRSZ_64          0x0040

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=[ program header ]=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
struct rd_elf_prghdr32 {
    /* type of segment */
    uint32_t type;

    /* offset in the file */
    uint32_t offset;

    /* virtual address */
    uint32_t vaddr;

    /* physical address */
    uint32_t paddr;

    /* size of segment in file */
    uint32_t size_file;

    /* size of segment in memory */
    uint32_t size_mem;

    /* flags */
    uint32_t flags;

    /* alignment */
    uint32_t alignment;
};

struct rd_elf_prghdr64 {
    /* type of segment */
    uint32_t type;

    /* flags */
    uint32_t flags;

    /* offset in the file */
    uint64_t offset;

    /* virtual address */
    uint64_t vaddr;

    /* physical address */
    uint64_t paddr;

    /* size of segment in file */
    uint64_t size_file;

    /* size of segment in memory */
    uint64_t size_mem;

    /* alignment */
    uint64_t alignment;
};

#define RD_ELF_PRGHDR_TYPE_NULL         0x00000000
#define RD_ELF_PRGHDR_TYPE_LOAD         0x00000001
#define RD_ELF_PRGHDR_TYPE_DYNAMIC      0x00000002
#define RD_ELF_PRGHDR_TYPE_INTERP       0x00000003
#define RD_ELF_PRGHDR_TYPE_NOTE         0x00000004
#define RD_ELF_PRGHDR_TYPE_SHLIB        0x00000005
#define RD_ELF_PRGHDR_TYPE_PHDR         0x00000006
#define RD_ELF_PRGHDR_TYPE_LOOS         0x60000000
#define RD_ELF_PRGHDR_TYPE_HIOS         0x6fffffff
#define RD_ELF_PRGHDR_TYPE_LOPROC       0x70000000
#define RD_ELF_PRGHDR_TYPE_HIPROC       0x7fffffff

#define RD_ELF_PRGHDR_FLAGS_X           0x00000001
#define RD_ELF_PRGHDR_FLAGS_W           0x00000002
#define RD_ELF_PRGHDR_FLAGS_R           0x00000004
#define RD_ELF_PRGHDR_FLAGS_MASKOS      0x0ff00000
#define RD_ELF_PRGHDR_FLAGS_MASKPROC    0xf0000000

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=[ section header ]=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
struct rd_elf_sechdr32 {
    /* offset to the name */
    uint32_t name;

    /* type of this section */
    uint32_t type;

    /* flags for the section */
    uint32_t flags;

    /* address of the section in memory */
    uint32_t addr;

    /* offset of the section in a file */
    uint32_t offset;

    /* size of the section on disk */
    uint32_t size;

    /* section index */
    uint32_t link;

    /* extra info */
    uint32_t info;

    /* alignment of the section */
    uint32_t align;

    /* size of each entry (fixed size only) */
    uint32_t entsz;
};

struct rd_elf_sechdr64 {
    /* offset to the name */
    uint32_t name;

    /* type of this section */
    uint32_t type;

    /* flags for the section */
    uint64_t flags;

    /* address of the section in memory */
    uint64_t addr;

    /* offset of the section in a file */
    uint64_t offset;

    /* size of the section on disk */
    uint64_t size;

    /* associated section index */
    uint32_t link;

    /* extra info */
    uint32_t info;

    /* alignment of the section */
    uint64_t align;

    /* size of each entry (fixed size only) */
    uint64_t entsz;
};

#define RD_ELF_SECHDR_TYPE_NULL             0x00000000
#define RD_ELF_SECHDR_TYPE_PROGBITS         0x00000001
#define RD_ELF_SECHDR_TYPE_SYMTAB           0x00000002
#define RD_ELF_SECHDR_TYPE_STRTAB           0x00000003
#define RD_ELF_SECHDR_TYPE_RELA             0x00000004
#define RD_ELF_SECHDR_TYPE_HASH             0x00000005
#define RD_ELF_SECHDR_TYPE_DYNAMIC          0x00000006
#define RD_ELF_SECHDR_TYPE_NOTE             0x00000007
#define RD_ELF_SECHDR_TYPE_NOBITS           0x00000008
#define RD_ELF_SECHDR_TYPE_REL              0x00000009
#define RD_ELF_SECHDR_TYPE_SHLIB            0x0000000a
#define RD_ELF_SECHDR_TYPE_DYNSYM           0x0000000b
#define RD_ELF_SECHDR_TYPE_INITARRAY        0x0000000e
#define RD_ELF_SECHDR_TYPE_FINIARRAY        0x0000000f
#define RD_ELF_SECHDR_TYPE_PREINITARRAY     0x00000010
#define RD_ELF_SECHDR_TYPE_GROUP            0x00000011
#define RD_ELF_SECHDR_TYPE_SYMTAB_SHDIDX    0x00000012
#define RD_ELF_SECHDR_TYPE_NUM              0x00000013
#define RD_ELF_SECHDR_TYPE_LOOS             0x60000000

#define RD_ELF_SECHDR_FLAGS_WRITE           0x00000001
#define RD_ELF_SECHDR_FLAGS_ALLOC           0x00000002
#define RD_ELF_SECHDR_FLAGS_EXECINSTR       0x00000004
#define RD_ELF_SECHDR_FLAGS_MERGE           0x00000010
#define RD_ELF_SECHDR_FLAGS_STRINGS         0x00000020
#define RD_ELF_SECHDR_FLAGS_INFO_LINK       0x00000040
#define RD_ELF_SECHDR_FLAGS_LINK_ORDER      0x00000080
#define RD_ELF_SECHDR_FLAGS_OS_NONCONFRM    0x00000100
#define RD_ELF_SECHDR_FLAGS_GROUP           0x00000200
#define RD_ELF_SECHDR_FLAGS_TLS             0x00000400
#define RD_ELF_SECHDR_FLAGS_ORDERED         0x04000000
#define RD_ELF_SECHDR_FLAGS_EXCLUDE         0x08000000
#define RD_ELF_SECHDR_FLAGS_MASKOS          0x0ff00000
#define RD_ELF_SECHDR_FLAGS_MASKPROC        0xf0000000

#endif /* __RD_ELF_INC_H */
