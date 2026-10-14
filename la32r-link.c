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
ST_FUNC int code_reloc(int type) { return type == R_LARCH_B16 || type == R_LARCH_B26 ? 1 : type == R_LARCH_32 || type == R_LARCH_RELATIVE || type == R_LARCH_ABS_HI20 || type == R_LARCH_ABS_LO12 ? 0 : -1; }
ST_FUNC int gotplt_entry_type(int type) { return type == R_LARCH_JUMP_SLOT ? NO_GOTPLT_ENTRY : type == R_LARCH_B26 ? AUTO_GOTPLT_ENTRY : type == R_LARCH_32 ? AUTO_GOTPLT_ENTRY : -1; }
ST_FUNC unsigned create_plt_entry(TCCState *s, unsigned off, struct sym_attr *a) { unsigned p = s->plt->data_offset; uint8_t *q = section_ptr_add(s->plt, 16); write32le(q, 0x14000000); return p; }
ST_FUNC void relocate_plt(TCCState *s) { }
ST_FUNC void relocate(TCCState *s1, ElfW_Rel *rel, int type, unsigned char *p, addr_t addr, addr_t val) {
    uint32_t x;
    switch (type) {
    case R_LARCH_NONE: return;
    case R_LARCH_32: write32le(p, read32le(p) + val); return;
    case R_LARCH_RELATIVE: return;
    case R_LARCH_B16: x = read32le(p); write32le(p, (x & 0xfc0003ff) | ((((val - addr) >> 2) & 0xffff) << 10)); return;
    case R_LARCH_B26: x = read32le(p); write32le(p, (x & 0xfc000000) | (((val - addr) >> 2) & 0x03ffffff)); return;
    case R_LARCH_ABS_HI20: x = read32le(p); write32le(p, (x & 0xfe00001f) | (((val + 0x800) >> 12) & 0xfffff) << 5); return;
    case R_LARCH_ABS_LO12: x = read32le(p); write32le(p, (x & 0xffc003ff) | ((val & 0xfff) << 10)); return;
    case R_LARCH_JUMP_SLOT: write32le(p, val); return;
    default: tcc_error_noabort("unsupported LA32R relocation %d", type); return;
    }
}
#endif
