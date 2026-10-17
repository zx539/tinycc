#ifdef TARGET_DEFS_ONLY
#define EM_TCC_TARGET EM_LOONGARCH
#define R_DATA_32 R_LARCH_32
#define R_DATA_PTR R_LARCH_32
#define R_JMP_SLOT R_LARCH_JUMP_SLOT
#define R_GLOB_DAT R_LARCH_32
#define R_COPY R_LARCH_COPY
#define R_RELATIVE R_LARCH_RELATIVE
#define R_NUM R_LARCH_NUM
#define ELF_START_ADDR 0x00010000
#define ELF_PAGE_SIZE 0x1000
#define PCRELATIVE_DLLPLT 1
#define RELOCATE_DLLPLT 1
#else
#include "tcc.h"
ST_FUNC int code_reloc(int type)
{
    switch (type) {
    case R_LARCH_B16:
    case R_LARCH_B21:
    case R_LARCH_B26:
        return 1;
    case R_LARCH_NONE:
    case R_LARCH_32:
    case R_LARCH_RELATIVE:
    case R_LARCH_JUMP_SLOT:
    case R_LARCH_ABS_HI20:
    case R_LARCH_ABS_LO12:
    case R_LARCH_PCALA_HI20:
    case R_LARCH_PCALA_LO12:
        return 0;
    default:
        return -1;
    }
}

ST_FUNC int gotplt_entry_type(int type)
{
    switch (type) {
    case R_LARCH_B26:
    case R_LARCH_32:
        return AUTO_GOTPLT_ENTRY;
    case R_LARCH_NONE:
    case R_LARCH_B16:
    case R_LARCH_B21:
    case R_LARCH_RELATIVE:
    case R_LARCH_JUMP_SLOT:
    case R_LARCH_ABS_HI20:
    case R_LARCH_ABS_LO12:
    case R_LARCH_PCALA_HI20:
    case R_LARCH_PCALA_LO12:
        return NO_GOTPLT_ENTRY;
    default:
        return -1;
    }
}
ST_FUNC unsigned create_plt_entry(TCCState *s, unsigned off, struct sym_attr *a) { unsigned p = s->plt->data_offset; uint8_t *q = section_ptr_add(s->plt, 16); write32le(q, 0x14000000); return p; }
ST_FUNC void relocate_plt(TCCState *s) { }
ST_FUNC void relocate(TCCState *s1, ElfW_Rel *rel, int type, unsigned char *p,
                      addr_t addr, addr_t val)
{
    uint32_t x, imm;
    int32_t off;
    int bits;
    switch (type) {
    case R_LARCH_NONE: return;
    /* The generic ELF linker has already added the RELA addend to val. */
    case R_LARCH_32: write32le(p, val); return;
    case R_LARCH_RELATIVE: return;
    case R_LARCH_B16:
    case R_LARCH_B21:
    case R_LARCH_B26:
        bits = type == R_LARCH_B16 ? 18 : type == R_LARCH_B21 ? 23 : 28;
        off = val - addr;
        if ((off & 3) || off < -(1 << (bits - 1))
            || off >= (1 << (bits - 1))) {
            tcc_error_noabort("R_LARCH_B%d relocation out of range or unaligned",
                              bits - 2);
            return;
        }
        imm = (uint32_t)off >> 2;
        x = read32le(p);
        if (type == R_LARCH_B16)
            x = (x & 0xfc0003ff) | ((imm & 0xffff) << 10);
        else if (type == R_LARCH_B21)
            x = (x & 0xfc0003e0) | ((imm & 0xffff) << 10)
                | ((imm >> 16) & 0x1f);
        else
            x = (x & 0xfc000000) | ((imm & 0xffff) << 10)
                | ((imm >> 16) & 0x3ff);
        write32le(p, x);
        return;
    case R_LARCH_ABS_HI20:
        /* lu12i.w + ori: the low part is unsigned, without rounding. */
        x = read32le(p);
        write32le(p, (x & 0xfe00001f) | ((val >> 12) << 5));
        return;
    case R_LARCH_PCALA_HI20:
        /* pcalau12i uses page-relative addressing; addi/ld/st sign-extends
           the low 12 bits.  Round the target page, not the PC delta. */
        imm = ((val + 0x800u) & ~0xfffu) - (addr & ~0xfffu);
        x = read32le(p);
        write32le(p, (x & 0xfe00001f) | ((imm >> 12) << 5));
        return;
    case R_LARCH_ABS_LO12:
    case R_LARCH_PCALA_LO12:
        x = read32le(p);
        write32le(p, (x & 0xffc003ff) | ((val & 0xfff) << 10));
        return;
    case R_LARCH_JUMP_SLOT: write32le(p, val); return;
    default: tcc_error_noabort("unsupported LA32R relocation %d", type); return;
    }
}
#endif
