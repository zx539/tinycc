/* LoongArch32 Release 1 code generator.
 * The instruction encodings used here are the base LA32R integer subset
 * described by docs/LoongArch32精简版指令表.pdf. */
#ifdef TARGET_DEFS_ONLY
#define NB_REGS 10                 /* a0-a7, ra, sp */
#define CONFIG_TCC_ASM
#define NB_ASM_REGS 32
#define TREG_R(x) (x)
#define RC_INT (1 << 0)
#define RC_FLOAT (1 << 1)
#define RC_R(x) (1 << (1 + (x)))
#define RC_IRET RC_R(0)
#define RC_IRE2 RC_R(1)
#define RC_FRET 0
#define REG_IRET TREG_R(0)
#define REG_IRE2 TREG_R(1)
#define REG_FRET 0
#define PTR_SIZE 4
#define LDOUBLE_SIZE 8
#define LDOUBLE_ALIGN 4
#define MAX_ALIGN 4
#define CHAR_IS_UNSIGNED
#else
#define USING_GLOBALS
#include "tcc.h"
#include <assert.h>

#define XLEN 4
#define TREG_RA 8
#define TREG_SP 9
#define LA_FP 22
#define IREG(r) ((r) == TREG_RA ? 1 : (r) == TREG_SP ? 3 : (r) + 4)
#define IS_IREG(r) ((unsigned)(r) < 8 || (r) == TREG_RA || (r) == TREG_SP)
#define OP_R(op, rd, rj, rk) (((op) << 15) | ((rk) << 10) | ((rj) << 5) | (rd))
#define OP_I(op, rd, rj, imm) (((op) << 22) | (((imm) & 0xfff) << 10) | ((rj) << 5) | (rd))
#define OP_B(op, rj, rd, imm) (((op) << 26) | (((imm) >> 2) & 0xffff) << 10 | ((rj) << 5) | (rd))

ST_DATA const char * const target_machine_defs = "__loongarch__\0__loongarch32\0__loongarch_grlen 32\0";
ST_DATA const int reg_classes[NB_REGS] = {
    RC_INT|RC_R(0), RC_INT|RC_R(1), RC_INT|RC_R(2), RC_INT|RC_R(3),
    RC_INT|RC_R(4), RC_INT|RC_R(5), RC_INT|RC_R(6), RC_INT|RC_R(7),
    1 << TREG_RA, 1 << TREG_SP
};

ST_FUNC void o(unsigned int c) {
    if (!nocode_wanted) { if (ind + 4 > cur_text_section->data_allocated) section_realloc(cur_text_section, ind + 4); write32le(cur_text_section->data + ind, c); ind += 4; }
}

ST_FUNC void g(int c) { if (!nocode_wanted) { if (ind + 1 > cur_text_section->data_allocated) section_realloc(cur_text_section, ind + 1); cur_text_section->data[ind++] = c; } }
ST_FUNC void gen_le16(int c) { g(c); g(c >> 8); }
ST_FUNC void gen_le32(int c) { o(c); }
ST_FUNC void gen_expr32(ExprValue *e) { gen_le32(e->v); }
ST_FUNC int asm_parse_regvar(int t) { return -1; }
ST_FUNC void asm_opcode(TCCState *s, int t) { expect("LA32R inline assembly"); }
ST_FUNC void asm_compute_constraints(ASMOperand *a, int n, int o, const uint8_t *c, int *r) { }
ST_FUNC void subst_asm_operand(CString *a, SValue *s, int m) { }
ST_FUNC void asm_gen_code(ASMOperand *a, int n, int o, int i, uint8_t *c, int r) { }
ST_FUNC void asm_clobber(uint8_t *c, const char *s) { }

ST_FUNC void gsym_addr(int t, int a) {
    while (t) { unsigned char *p = cur_text_section->data + t; uint32_t n = read32le(p); int32_t d = a - t; if ((d & 3) || d < -(1 << 17) || d >= (1 << 17)) tcc_error("branch out of range"); write32le(p, (read32le(p) & 0xfc00001f) | (((d >> 2) & 0xffff) << 10)); t = n; }
}

static void emit_i(int op, int d, int j, int imm) { o(OP_I(op, d, j, imm)); }
static void emit_r(int op, int d, int j, int k) { o(OP_R(op, d, j, k)); }

ST_FUNC void load(int r, SValue *sv) {
    int d = IREG(r), v = sv->r & VT_VALMASK, size, al, base, off = sv->c.i;
    if (sv->r & VT_LVAL) {
        size = type_size(&sv->type, &al); base = v < VT_CONST ? IREG(v) : 3;
        if (v == VT_LOCAL) base = LA_FP;
        else if (v >= VT_CONST) { emit_i(0x0a, 5, 0, off >> 12); emit_i(0x0a, 5, 5, off); base = 5; off = 0; }
        if (sv->r & VT_SYM) { greloca(cur_text_section, sv->sym, ind, R_LARCH_ABS_LO12, off); base = 0; off = 0; }
        emit_i(size == 1 ? ((sv->type.t & VT_UNSIGNED) ? 0xA8 : 0xA0) : size == 2 ? ((sv->type.t & VT_UNSIGNED) ? 0xA9 : 0xA1) : 0xA2, d, base, off);
    } else if (v == VT_CONST) {
        emit_i(0x0a, d, 0, off >> 12); emit_i(0x0a, d, d, off);
    } else if (v < VT_CONST) {
        emit_i(0x0a, d, IREG(v), 0);
    } else if (v == VT_CMP) {
        int a = IREG(vtop->cmp_r & 255), b = IREG((vtop->cmp_r >> 8) & 255);
        emit_r((vtop->cmp_op >= TOK_ULT && vtop->cmp_op <= TOK_UGT) ? 0x25 : 0x24, d, a, b);
    } else tcc_error("la32r: unsupported load");
}

ST_FUNC void store(int r, SValue *sv) {
    int s = IREG(r), v = sv->r & VT_VALMASK, size, al, base = v < VT_CONST ? IREG(v) : 3, off = sv->c.i;
    size = type_size(&sv->type, &al); if (v == VT_LOCAL) base = LA_FP; else if (v >= VT_CONST) { emit_i(0x0a, 5, 0, off >> 12); emit_i(0x0a, 5, 5, off); base = 5; off = 0; }
    o((size == 1 ? 0xA4 : size == 2 ? 0xA5 : 0xA6) << 22 | ((off & 0xfff) << 10) | (base << 5) | s);
}

static void load_const(int r, int32_t v) { emit_i(0x0a, IREG(r), 0, v >> 12); emit_i(0x0a, IREG(r), IREG(r), v); }

ST_FUNC void gfunc_call(int n) {
    int i; for (i = n - 1; i >= 0; --i) { gv(RC_R(i < 8 ? i : 0)); if (i < 8 && vtop->r != i) emit_i(0x0a, IREG(i), IREG(vtop->r), 0); vtop--; }
    if ((vtop->r & VT_VALMASK) == VT_CONST && (vtop->r & VT_SYM)) { greloca(cur_text_section, vtop->sym, ind, R_LARCH_B26, 0); o(0x15 << 26); } else { gv(RC_INT); emit_i(0x13, 1, IREG(vtop->r), 0); }
    vtop--;
}

static int la_prolog;
ST_FUNC void gfunc_prolog(Sym *f) {
    Sym *a;
    int i = 0;
    loc = -4;
    la_prolog = ind;
    o(0);                 /* patched: allocate frame */
    o(0);                 /* patched: establish frame pointer */
    emit_i(0xA6, 1, LA_FP, -4); /* save ra */
    for (a = f->type.ref->next; a; a = a->next, ++i) {
        loc -= 4;
        gfunc_set_param(a, loc, 0);
        if (i < 8)
            o((0xA6 << 22) | (((loc & 0xfff) << 10)) | (LA_FP << 5) | (4 + i));
    }
}
ST_FUNC int gfunc_sret(CType *v, int va, CType *r, int *a, int *rs) { int z = type_size(v, a); *rs = 4; r->t = VT_INT; return z > 8 ? 0 : (z + 3) / 4; }
ST_FUNC void gfunc_epilog(void) {
    int size = (-loc + 15) & -16;
    if (size < 16) size = 16;
    write32le(cur_text_section->data + la_prolog, OP_I(0x0a, 3, 3, -size));
    write32le(cur_text_section->data + la_prolog + 4, OP_I(0x0a, LA_FP, 3, size));
    emit_i(0xA2, 1, LA_FP, -4); /* restore ra */
    emit_i(0x0a, 3, LA_FP, 0);  /* discard frame */
    o((0x13 << 26) | (1 << 10) | (1 << 5)); /* jirl zero, ra, 0 */
}
ST_FUNC void arch_transfer_ret_regs(int x) { }
ST_FUNC void gen_va_start(void) { vtop--; vset(&char_pointer_type, VT_LOCAL, 0); }

ST_FUNC int gjmp(int t) { o(t); return ind - 4; }
ST_FUNC void gjmp_addr(int a) { int d = a - ind; o(0x14 << 26 | ((d >> 2) & 0x03ffffff)); }
ST_FUNC int gjmp_cond(int op, int t) { int a = IREG(vtop->cmp_r & 255), b = IREG((vtop->cmp_r >> 8) & 255); o(OP_B(op == TOK_EQ ? 0x16 : op == TOK_NE ? 0x17 : op == TOK_LT ? 0x18 : 0x19, a, b, 8)); return gjmp(t); }
ST_FUNC int gjmp_append(int n, int t) { if (n) { int q = n; while (read32le(cur_text_section->data + q)) q = read32le(cur_text_section->data + q); write32le(cur_text_section->data + q, t); } return n ? n : t; }

ST_FUNC void gen_opi(int op) {
    int a, b, d, rop = 0x20;
    if (op >= TOK_ULT && op <= TOK_GT) { gv2(RC_INT, RC_INT); a = vtop[-1].r; b = vtop->r; vtop--; vset_VT_CMP(op); vtop->cmp_r = a | (b << 8); return; }
    if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) { int c = vtop->c.i; vswap(); a = gv(RC_INT); d = a; emit_i(op == '+' ? 0x0a : 0x0d, IREG(d), IREG(a), c); vtop--; vtop->r = d; return; }
    gv2(RC_INT, RC_INT); a = IREG(vtop[-1].r); b = IREG(vtop->r); vtop--; d = vtop->r;
    switch (op) { case '+': rop=0x20; break; case '-': rop=0x22; break; case '*': rop=0x38; break; case '/': rop=0x40; break; case '%': rop=0x41; break; case '&': rop=0x29; break; case '|': rop=0x2a; break; case '^': rop=0x2b; break; case TOK_SHL: rop=0x2e; break; case TOK_SHR: rop=0x2f; break; case TOK_SAR: rop=0x30; break; default: tcc_error("la32r: unsupported operator"); }
    emit_r(rop, IREG(d), a, b);
}
ST_FUNC void gen_opf(int op) { tcc_error("la32r: floating point is not implemented"); }
ST_FUNC void gen_cvt_csti(int t) { }
ST_FUNC void gen_cvt_sxtw(void) { }
ST_FUNC void gen_cvt_itof(int t) { tcc_error("la32r: floating point is not implemented"); }
ST_FUNC void gen_cvt_ftoi(int t) { tcc_error("la32r: floating point is not implemented"); }
ST_FUNC void gen_cvt_ftof(int t) { tcc_error("la32r: floating point is not implemented"); }
ST_FUNC void gen_clear_cache(void) { }
ST_FUNC void gen_fill_nops(int n) { while (n >= 4) { o(0); n -= 4; } }
ST_FUNC void ggoto(void) { load(TREG_RA, vtop); vtop--; }
ST_FUNC void gen_vla_sp_save(int a) { }
ST_FUNC void gen_vla_sp_restore(int a) { }
ST_FUNC void gen_vla_alloc(CType *t, int a) { }
#endif
