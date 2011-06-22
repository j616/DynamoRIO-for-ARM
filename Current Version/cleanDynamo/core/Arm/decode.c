#include "../globals.h"
#include "arch.h"
#include "instr.h"
#include "decode.h"
#include "decode_fast.h"
#include <string.h> /* for memcpy */

static ptr_int_t
get_immed(decode_info_t *di, opnd_size_t opsize)
{
	// COMPLETEDD get_immed
	printf("Starting get_immed\n");
    ptr_int_t val = 0;
#ifndef ARM
    if (di->size_immed == OPSZ_NA) {
        /* ok b/c only instr_info_t fields passed */
        CLIENT_ASSERT(di->size_immed2 != OPSZ_NA, "decode immediate size error");
        val = di->immed2;
        di->size_immed2 = OPSZ_NA; /* mark as used up */
    } else {
        /* ok b/c only instr_info_t fields passed */
        CLIENT_ASSERT(di->size_immed != OPSZ_NA, "decode immediate size error");
        val = di->immed;
        di->size_immed = OPSZ_NA; /* mark as used up */
    }
#else
    val = di->immed;
#endif
    return val;
}

#ifndef ARM

opnd_size_t
resolve_var_reg_size(opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_1_reg4: return (is_reg ? OPSZ_4 : OPSZ_1);
    case OPSZ_2_reg4: return (is_reg ? OPSZ_4 : OPSZ_2);
    case OPSZ_4_reg16: return (is_reg ? OPSZ_16 : OPSZ_4);
    }
    return sz;
}
/* Like all our code, we assume cs specifies default data and address sizes.
 * This routine assumes the size varies by data, NOT by address!
 */
opnd_size_t
resolve_variable_size(decode_info_t *di/*IN: x86_mode, prefixes*/,
                      opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_2_short1: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_1 : OPSZ_2);
    case OPSZ_4_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4);
    case OPSZ_4x8: return (X64_MODE(di) ? OPSZ_8 : OPSZ_4);
    case OPSZ_4x8_short2:
        return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 :
                (X64_MODE(di) ? OPSZ_8 : OPSZ_4));
    case OPSZ_4x8_short2xi8:
        return (X64_MODE(di) ? (proc_get_vendor() == VENDOR_INTEL ? OPSZ_8 :
                              (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8)) :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_short2xi4:
        return ((X64_MODE(di) && proc_get_vendor() == VENDOR_INTEL) ? OPSZ_4 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8_short2: /* rex.w trumps data prefix */
        return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8: return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 : OPSZ_4);
    case OPSZ_6_irex10_short4: /* rex.w trumps data prefix, but is ignored on AMD */
        DODEBUG({
            /* less annoying than a CURIOSITY assert when testing */
            if (TEST(PREFIX_REX_W, di->prefixes))
                SYSLOG_INTERNAL_INFO_ONCE("curiosity: rex.w on OPSZ_6_irex10_short4!");
        });
        return ((TEST(PREFIX_REX_W, di->prefixes) && proc_get_vendor() != VENDOR_AMD) ?
                OPSZ_10 : (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_6));
    case OPSZ_6x10: return (X64_MODE(di) ? OPSZ_10 : OPSZ_6);
    case OPSZ_8_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8);
    case OPSZ_8_short4: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_8);
    case OPSZ_28_short14:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_14 : OPSZ_28);
    case OPSZ_108_short94:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_94 : OPSZ_108);
    case OPSZ_1_reg4:
    case OPSZ_2_reg4:
    case OPSZ_4_reg16:
        return resolve_var_reg_size(sz, is_reg);
    }
    return sz;
}
/* Also takes in reg8 for TYPE_REG_EX mov_imm */
reg_id_t
resolve_var_reg(decode_info_t *di/*IN: x86_mode, prefixes*/,
                reg_id_t reg32, bool addr, bool can_shrink
                _IF_X64(bool default_64) _IF_X64(bool can_grow)
                _IF_X64(bool extendable))
{
#ifdef X64
    if (extendable && X64_MODE(di) && di->prefixes != 0/*optimization*/) {
        /* Note that Intel's table 3-1 on +r possibilities is incorrect:
         * it lists rex.r, while Table 2-4 lists rex.b which is correct.
         */
        if (TEST(PREFIX_REX_B, di->prefixes))
            reg32 = reg32 + 8;
        else
            reg32 = reg8_alternative(di, reg32, di->prefixes);
    }
#endif

    if (addr) {
#ifdef X64
        if (X64_MODE(di)) {
            CLIENT_ASSERT(default_64, "addr-based size must be default 64");
            if (!can_shrink || !TEST(PREFIX_ADDR, di->prefixes))
                return reg_32_to_64(reg32);
            /* else leave 32 (it's addr32 not addr16) */
        } else
#endif
            if (can_shrink && TEST(PREFIX_ADDR, di->prefixes))
                return reg_32_to_16(reg32);
    } else {
#ifdef X64
        /* rex.w trumps data prefix */
        if (X64_MODE(di) &&
            ((can_grow && TEST(PREFIX_REX_W, di->prefixes)) ||
             (default_64 && (!can_shrink || !TEST(PREFIX_DATA, di->prefixes)))))
            return reg_32_to_64(reg32);
        else
#endif
            if (can_shrink && TEST(PREFIX_DATA, di->prefixes))
                return reg_32_to_16(reg32);
    }
    return reg32;
}

/* which register within modrm we're decoding */
typedef enum {
    DECODE_REG_REG,
    DECODE_REG_BASE,
    DECODE_REG_INDEX,
    DECODE_REG_RM,
} decode_reg_t;
/* Pass in the raw opsize, NOT a size passed through resolve_variable_size(),
 * to avoid allowing OPSZ_6_irex10_short4 w/ data16
 */
static reg_id_t
decode_reg(decode_reg_t which_reg, decode_info_t *di, byte optype, opnd_size_t opsize)
{
    bool extend = false;
    byte reg = 0;
    switch (which_reg) {
    case DECODE_REG_REG:
        reg = di->reg;   extend = X64_MODE(di) && TEST(PREFIX_REX_R, di->prefixes); break;
    case DECODE_REG_BASE:
        reg = di->base;  extend = X64_MODE(di) && TEST(PREFIX_REX_B, di->prefixes); break;
    case DECODE_REG_INDEX:
        reg = di->index; extend = X64_MODE(di) && TEST(PREFIX_REX_X, di->prefixes); break;
    case DECODE_REG_RM:
        reg = di->rm;    extend = X64_MODE(di) && TEST(PREFIX_REX_B, di->prefixes); break;
    default:
        CLIENT_ASSERT(false, "internal unknown reg error");
    }

    switch (optype) {
    case TYPE_P:
    case TYPE_Q:
    case TYPE_P_MODRM:
        return (REG_START_MMX + reg); /* no x64 extensions */
    case TYPE_V:
    case TYPE_W:
    case TYPE_V_MODRM:
        return (extend? (REG_START_XMM + 8 + reg) : (REG_START_XMM + reg));
    case TYPE_S:
        if (reg >= 6)
            return REG_NULL;
        return (REG_START_SEGMENT + reg);
    case TYPE_C:
        return (extend? (REG_START_CR + 8 + reg) : (REG_START_CR + reg));
    case TYPE_D:
        return (extend? (REG_START_DR + 8 + reg) : (REG_START_DR + reg));
    case TYPE_E:
    case TYPE_G:
    case TYPE_R:
    case TYPE_M:
    case TYPE_INDIR_E:
    case TYPE_FLOATMEM:
        /* GPR: fall-through since variable subset of full register */
        break;
    default:
        CLIENT_ASSERT(false, "internal unknown reg error");
    }

    /* Do not allow a register for 'p' or 'a' types.  FIXME: maybe *_far_ind_* should
     * use TYPE_INDIR_M instead of TYPE_INDIR_E?  What other things are going to turn
     * into asserts or crashes instead of invalid instrs based on events as fragile
     * as these decode routines moving sizes around?
     */
    if (opsize != OPSZ_6_irex10_short4 && opsize != OPSZ_8_short4)
        opsize = resolve_variable_size(di, opsize, true);

    switch (opsize) {
    case OPSZ_1:
        if (extend)
            return (REG_START_8 + 8 + reg);
        else
            return reg8_alternative(di, REG_START_8 + reg, di->prefixes);
    case OPSZ_2:
        return (extend? (REG_START_16 + 8 + reg) : (REG_START_16 + reg));
    case OPSZ_4:
        return (extend? (REG_START_32 + 8 + reg) : (REG_START_32 + reg));
    case OPSZ_8:
        return (extend? (REG_START_64 + 8 + reg) : (REG_START_64 + reg));
    case OPSZ_6:
    case OPSZ_6_irex10_short4:
    case OPSZ_8_short4:
        /* invalid: no register of size p */
        return REG_NULL;
    default:
        /* ok to assert since params controlled by us */
        CLIENT_ASSERT(false, "decode error: unknown register size");
        return REG_NULL;
    }
}
/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */

static bool
decode_modrm(decode_info_t *di, byte optype, opnd_size_t opsize,
             opnd_t *reg_opnd, opnd_t *rm_opnd)
{
    /* for x64, addr prefix affects only base/index and truncates final addr:
     * modrm + sib table is the same
     */
    bool addr16 = !X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes);

    if (reg_opnd != NULL) {
        reg_id_t reg = decode_reg(DECODE_REG_REG, di, optype, opsize);
        if (reg == REG_NULL)
            return false;
        *reg_opnd = opnd_create_reg(reg);
    }

    if (rm_opnd != NULL) {
        reg_id_t base_reg = REG_NULL;
        int disp = 0;
        reg_id_t index_reg = REG_NULL;
        int scale = 0;
        char memtype = TYPE_M;
        opnd_size_t memsize = resolve_addr_size(di);
        bool encode_zero_disp, force_full_disp;
        if (di->has_disp)
            disp = di->disp;
        else
            disp = 0;
        if (di->has_sib) {
            CLIENT_ASSERT(!addr16,
                          "decode error: x86 addr16 cannot have a SIB byte");
            if (di->index == 4 &&
                /* rex.x enables r12 as index */
                (!X64_MODE(di) || !TEST(PREFIX_REX_X, di->prefixes))) {
                /* no scale/index */
                index_reg = REG_NULL;
            } else {
                index_reg = decode_reg(DECODE_REG_INDEX, di, memtype, memsize);
                if (index_reg == REG_NULL) {
                    CLIENT_ASSERT(false, "decode error: internal modrm decode error");
                    return false;
                }
                if (di->scale == 0)
                    scale = 1;
                else if (di->scale == 1)
                    scale = 2;
                else if (di->scale == 2)
                    scale = 4;
                else if (di->scale == 3)
                    scale = 8;
            }
            if (di->base == 5 && di->mod == 0) {
                /* no base */
                base_reg = REG_NULL;
            } else {
                base_reg = decode_reg(DECODE_REG_BASE, di, memtype, memsize);
                if (base_reg == REG_NULL) {
                    CLIENT_ASSERT(false, "decode error: internal modrm decode error");
                    return false;
                }
            }
        } else {
            if ((!addr16 && di->mod == 0 && di->rm == 5) ||
                (addr16 && di->mod == 0 && di->rm == 6)) {
                /* just absolute displacement, or rip-relative for x64 */
#ifdef X64
                if (X64_MODE(di)) {
                    /* rip-relative: convert from relative offset to absolute target pc */
                    byte *addr;
                    CLIENT_ASSERT(di->start_pc != NULL,
                                  "internal decode error: start pc not set");
                    if (di->orig_pc != di->start_pc)
                        addr = di->orig_pc + di->len + di->disp;
                    else
                        addr = di->start_pc + di->len + di->disp;
                    if (TEST(PREFIX_ADDR, di->prefixes)) {
                        /* Need to clear upper 32 bits.
                         * Debuggers do not display this truncation, though
                         * both Intel and AMD manuals describe it.
                         * I did verify it w/ actual execution.
                         */
                        ASSERT_NOT_TESTED();
                        addr = (byte *) ((ptr_uint_t)addr & 0xffffffff);
                    }
                    *rm_opnd = opnd_create_far_rel_addr
                        (di->seg_override, (void *) addr,
                         resolve_variable_size(di, opsize, false));
                    return true;
                } else
#endif
                    base_reg = REG_NULL;
                index_reg = REG_NULL;
            } else if (di->mod == 3) {
                /* register */
                reg_id_t rm_reg = decode_reg(DECODE_REG_RM, di, optype, opsize);
                if (rm_reg == REG_NULL) /* no assert since happens, e.g., ff d9 */
                    return false;
                else {
                    *rm_opnd = opnd_create_reg(rm_reg);
                    return true;
                }
            } else {
                /* non-sib reg-based memory address */
                if (addr16) {
                    /* funny order requiring custom decode */
                    switch (di->rm) {
                    case 0: base_reg = REG_BX; index_reg = REG_SI; scale = 1; break;
                    case 1: base_reg = REG_BX; index_reg = REG_DI; scale = 1; break;
                    case 2: base_reg = REG_BP; index_reg = REG_SI; scale = 1; break;
                    case 3: base_reg = REG_BP; index_reg = REG_DI; scale = 1; break;
                    case 4: base_reg = REG_SI; break;
                    case 5: base_reg = REG_DI; break;
                    case 6: base_reg = REG_BP;
                        CLIENT_ASSERT(di->mod != 0,
                                      "decode error: %bp cannot have mod 0");
                        break;
                    case 7: base_reg = REG_BX; break;
                    default: CLIENT_ASSERT(false, "decode error: unknown modrm rm");
                        break;
                    }
                } else {
                    /* single base reg */
                    base_reg = decode_reg(DECODE_REG_RM, di, memtype, memsize);
                    if (base_reg == REG_NULL) {
                        CLIENT_ASSERT(false,
                                      "decode error: internal modrm decode error");
                        return false;
                    }
                }
            }

        }
        /* We go ahead and preserve the force bools if the original really had a 0
         * disp; up to user to unset bools when changing disp value (FIXME: should
         * we auto-unset on first mod?)
         */
        encode_zero_disp = di->has_disp && disp == 0 &&
            /* there is no bp base without a disp */
            (!addr16 || base_reg != REG_BP);
        force_full_disp = di->has_disp && disp >= INT8_MIN && disp <= INT8_MAX &&
            di->mod == 2;
        if (di->seg_override != REG_NULL) {
            *rm_opnd = opnd_create_far_base_disp_ex
                (di->seg_override, base_reg, index_reg, scale, disp,
                 resolve_variable_size(di, opsize, false),
                 encode_zero_disp, force_full_disp,
                 TEST(PREFIX_ADDR, di->prefixes));
        } else {
            /* Note that OP_{jmp,call}_far_ind does NOT have a far base disp
             * operand: it is a regular base disp containing 6 bytes that
             * specify a segment selector and address.  The opcode must be
             * examined to know how to interpret those 6 bytes.
             */
            *rm_opnd = opnd_create_base_disp_ex
                (base_reg, index_reg, scale, disp,
                 resolve_variable_size(di, opsize, false),
                 encode_zero_disp, force_full_disp,
                 TEST(PREFIX_ADDR, di->prefixes));
        }
    }
    return true;
}

bool decode_operand(decode_info_t *di, byte optype, opnd_size_t opsize, opnd_t *opnd)
{
    /* resolving here, for non-reg, makes for simpler code: though the
     * most common types don't need this.
     */
    opnd_size_t ressize = resolve_variable_size(di, opsize, false);
    switch (optype) {
    case TYPE_NONE:
        *opnd = opnd_create_null();
        return true;
    case TYPE_REG:
        *opnd = opnd_create_reg(opsize);
        return true;
    case TYPE_VAR_REG:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, true/*shrinkable*/
                                 _IF_X64(false/*d32*/) _IF_X64(true/*growable*/)
                                 _IF_X64(false/*!extendable*/)));
        return true;
    case TYPE_VARZ_REG:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, true/*shrinkable*/
                                 _IF_X64(false/*d32*/) _IF_X64(false/*!growable*/)
                                 _IF_X64(false/*!extendable*/)));
        return true;
    case TYPE_VAR_XREG:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, true/*shrinkable*/
                                 _IF_X64(true/*d64*/) _IF_X64(false/*!growable*/)
                                 _IF_X64(false/*!extendable*/)));
        return true;
    case TYPE_VAR_ADDR_XREG:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, true/*addr*/, true/*shrinkable*/
                                 _IF_X64(true/*d64*/) _IF_X64(false/*!growable*/)
                                 _IF_X64(false/*!extendable*/)));
        return true;
    case TYPE_REG_EX:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, false/*!shrink*/
                                 _IF_X64(false/*d32*/) _IF_X64(false/*!growable*/)
                                 _IF_X64(true/*extendable*/)));
        return true;
    case TYPE_VAR_REG_EX:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, true/*shrinkable*/
                                 _IF_X64(false/*d32*/) _IF_X64(true/*growable*/)
                                 _IF_X64(true/*extendable*/)));
        return true;
    case TYPE_VAR_XREG_EX:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, true/*shrinkable*/
                                 _IF_X64(true/*d64*/) _IF_X64(false/*!growable*/)
                                 _IF_X64(true/*extendable*/)));
        return true;
    case TYPE_VAR_REGX_EX:
        *opnd = opnd_create_reg(resolve_var_reg
                                (di, opsize, false/*!addr*/, false/*!shrink*/
                                 _IF_X64(false/*d64*/) _IF_X64(true/*growable*/)
                                 _IF_X64(true/*extendable*/)));
        return true;
    case TYPE_FLOATMEM:
    case TYPE_M:
        /* ensure referencing memory */
        if (di->mod >= 3)
            return false;
        /* fall through */
    case TYPE_E:
    case TYPE_Q:
    case TYPE_W:
        return decode_modrm(di, optype, opsize, NULL, opnd);
    case TYPE_R:
    case TYPE_P_MODRM:
    case TYPE_V_MODRM:
        /* ensure referencing a register */
        if (di->mod != 3)
            return false;
        return decode_modrm(di, optype, opsize, NULL, opnd);
    case TYPE_G:
    case TYPE_P:
    case TYPE_V:
    case TYPE_S:
    case TYPE_C:
    case TYPE_D:
        return decode_modrm(di, optype, opsize, opnd, NULL);
    case TYPE_I:
        *opnd = opnd_create_immed_int(get_immed(di, opsize), ressize);
        return true;
    case TYPE_1:
        CLIENT_ASSERT(opsize == OPSZ_0, "internal decode inconsistency");
        *opnd = opnd_create_immed_int(1, ressize);
        return true;
    case TYPE_FLOATCONST:
        CLIENT_ASSERT(opsize == OPSZ_0, "internal decode inconsistency");
        /* FIXME: we don't modify any floating point state, but we
         * could cleanup the binary representation of 0.0f is 0x0
         */
        *opnd = opnd_create_immed_float(0.);
        return true;
    case TYPE_J:
        if (di->seg_override == SEG_JCC_NOT_TAKEN ||
            di->seg_override == SEG_JCC_TAKEN) {
            /* SEG_DS - taken,     pt */
            /* SEG_CS - not taken, pn */
            /* starting from RH9 I see code using this */
            LOG(THREAD_GET, LOG_EMIT, 5, "disassemble: branch hint %s:\n",
                di->seg_override == SEG_JCC_TAKEN ? "pt" : "pn");
            if (di->seg_override == SEG_JCC_NOT_TAKEN)
                di->prefixes |= PREFIX_JCC_NOT_TAKEN;
            else
                di->prefixes |= PREFIX_JCC_TAKEN;
            di->seg_override = REG_NULL;
            STATS_INC(num_branch_hints);
        }
        /* just ignore other segment prefixes -- don't assert */
        *opnd = opnd_create_pc((app_pc)get_immed(di, opsize));
        return true;
    case TYPE_A:
        {
#ifdef IA32_ON_IA64
            if (opsize == OPSZ_4_short2) {
                if (di->seg_override == SEG_CS || di->seg_override == SEG_DS) {
                    /* branch hint! just delete it? FIXME! */
                    di->seg_override = REG_NULL;
                    ASSERT_CURIOSITY(false); /* see if this ever happens */
                }
                /* just ignore other segment prefixes -- don't assert */
                *opnd = opnd_create_pc((app_pc)get_immed(di, opsize));
                return true;
            }
#endif
            /* ok since instr_info_t fields */
            CLIENT_ASSERT(!X64_MODE(di), "x64 has no type A instructions");
            CLIENT_ASSERT(opsize == OPSZ_6_irex10_short4, "decode A operand error");
            /* just ignore segment prefixes -- don't assert */
            if (TEST(PREFIX_DATA, di->prefixes)) {
                /* 4-byte immed */
                ptr_int_t val = get_immed(di, opsize);
                *opnd = opnd_create_far_pc
                    ((ushort) (((ptr_int_t)val & 0xffff0000) >> 16),
                     (app_pc)((ptr_int_t)val & 0x0000ffff));
            } else {
                /* 6-byte immed */
                /* ok since instr_info_t fields */
                CLIENT_ASSERT(di->size_immed == OPSZ_6 &&
                              di->size_immed2 == OPSZ_6,
                              "decode A operand 6-byte immed error");
                ASSERT(CHECK_TRUNCATE_TYPE_short(di->immed));
                *opnd = opnd_create_far_pc((ushort)(short)di->immed, (app_pc) di->immed2);
                di->size_immed = OPSZ_NA;
                di->size_immed2 = OPSZ_NA;
            }
            return true;
        }
    case TYPE_O:
        {
            /* no modrm byte, offset follows directly */
            ptr_int_t immed = get_immed(di, resolve_addr_size(di));
            *opnd = opnd_create_far_abs_addr(di->seg_override, (void *) immed, ressize);
            return true;
        }
    case TYPE_X:
        /* this means the memory address DS:(E)SI */
        if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_SI, REG_NULL,0,0,ressize);
        else if (!X64_MODE(di) || TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_ESI, REG_NULL,0,0,ressize);
        else
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_RSI, REG_NULL,0,0,ressize);
        return true;
    case TYPE_Y:
        /* this means the memory address ES:(E)DI */
        if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(SEG_ES, REG_DI, REG_NULL, 0, 0, ressize);
        else if (!X64_MODE(di) || TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(SEG_ES, REG_EDI, REG_NULL, 0, 0, ressize);
        else
            *opnd = opnd_create_far_base_disp(SEG_ES, REG_RDI, REG_NULL, 0, 0, ressize);
        return true;
    case TYPE_XLAT:
        /* this means the memory address DS:(E)BX+AL */
        if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_BX, REG_AL,1,0,ressize);
        else if (!X64_MODE(di) || TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_EBX, REG_AL,1,0,ressize);
        else
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_RBX, REG_AL,1,0,ressize);
        return true;
    case TYPE_MASKMOVQ:
        /* this means the memory address DS:(E)DI */
        if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_DI, REG_NULL,0,0,ressize);
        else if (!X64_MODE(di) || TEST(PREFIX_ADDR, di->prefixes))
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_EDI, REG_NULL,0,0,ressize);
        else
            *opnd = opnd_create_far_base_disp(ds_seg(di), REG_RDI, REG_NULL,0,0,ressize);
        return true;
    case TYPE_INDIR_REG:
        /* FIXME: how know data size?  for now just use reg size: our only use
         * of this does not have a varying hardcoded reg, fortunately. */
        *opnd = opnd_create_base_disp(opsize, REG_NULL, 0, 0, reg_get_size(opsize));
        return true;
    case TYPE_INDIR_VAR_XREG: /* indirect reg varies by addr16 not data16, base is 4x8,
                               * opsize varies by data16 */
    case TYPE_INDIR_VAR_REG: /* indirect reg varies by addr16 not data16, base is 4x8,
                              * opsize varies by rex and data16 */
    case TYPE_INDIR_VAR_XIREG: /* indirect reg varies by addr16 not data16, base is 4x8,
                                * opsize varies by data16 except on 64-bit Intel */
        {
            reg_id_t reg =
                resolve_var_reg(di, opsize, true/*addr*/, true/*shrinkable*/
                                _IF_X64(true/*d64*/) _IF_X64(false/*!growable*/)
                                _IF_X64(false/*!extendable*/));
            /* FIXME - xref case 10541/PR 214976, this type is overused and could
             * have different sizes. Right now TYPE_INDIR_VAR_XREG is used for
             * push/pop/pushf/popf/call (for which
             * OPSZ_VARSTACK == OPSZ_4x8_short2 is right), and
             * TYPE_INDIR_VAR_XIREG is used for ret/ret_imm, which are
             * correct.  However, TYPE_INDIR_VAR_XREG is also used
             * for pusha/popa/call_far/enter/leave/ret_far/int which have
             * different sizes, may access memory down instead of up from the address
             * or at an offset to the address.  Currently iret, far ret, and
             * far call use TPYE_INDIR_VAR_REG, and they do multiple pushes/pops
             * as well (iret does 3 (or 5) pops of this size
             * depending on the operand size (3 for 16 or 32-bit, 5 for 64-bit)).
             * NOTE - needs to match size in opnd_type_ok() and instr_create.h */
            *opnd = opnd_create_base_disp (reg, REG_NULL, 0, 0,
                                           resolve_variable_size
                                           (di, indir_var_reg_size(optype),
                                            false/*not reg*/));
            return true;
        }
    case TYPE_INDIR_E:
        /* FIXME: how mark as indirect?
         * in current usage decode_modrm will be treated as indirect, becoming
         * a base_disp operand, vs. an immed, which becomes a pc operand
         * besides, Ap is just as indirect as i_Ep!
         */
        return decode_operand(di, TYPE_E, opsize, opnd);
    default:
        /* ok to assert, types coming only from instr_info_t */
        CLIENT_ASSERT(false, "decode error: unknown operand type");
    }
    return false;
}
#else
static bool
decode_operand(decode_info_t *di, byte optype, opnd_size_t opsize, opnd_t *opnd)
{
	// INPROCESSS decode_operand
    /* resolving here, for non-reg, makes for simpler code: though the
     * most common types don't need this.
     */
	printf("Starting Decode_operand\n");
    switch (optype) {
    case TYPE_NONE:
        *opnd = opnd_create_null();
        return true;
    case Reg:
        *opnd = opnd_create_reg(opsize);
        return true;
    case Immediate:
    	// ADDME Warning Setting the default size of immed to 10
    	*opnd = opnd_create_immed_int(get_immed(di, opsize), 10);
    	return true;
    }
}
#endif




/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */
/* Disassembles the instruction at pc into the data structures ret_info
 * and di.  Does NOT set or read di->len.
 * Returns a pointer to the pc of the next instruction.
 * If just_opcode is true, does not decode the immeds and returns NULL
 * (you must call decode_next_pc to get the next pc, but that's faster
 * than decoding the immeds)
 * Returns NULL on an invalid instruction
 */
#ifdef ARM

// This function just works out what type of Data Processing Instruction we have
int decodeDataProcessing(unsigned int instruction)
{
  int i = 0;
  for(i = 0; i < DP_NUMBER; i++)
  {
  //  printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction,
   //   DataProcCats[i].concernedBits, DataProcCats[i].maskedBits);
    if((instruction & DataProcCats[i].maskedBits) == DataProcCats[i].concernedBits)
      return DataProcCats[i].catergoryNumber;
  }
  return DP_UNDEFINED;
}

// This function just works out which of the top layer the instruction is
int decodeCommon(unsigned int instruction)
{
  int i = 0;
  for(i = 0; i < CAT_NUMBER; i++)
  {
  //  printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction, Catergories[i].maskedBits, Catergories[i].concernedBits);
    if((instruction & Catergories[i].concernedBits) == Catergories[i].maskedBits)
      return Catergories[i].catergoryNumber;
  }
  return CAT_UNDEFINED;
}

// Read the operands of the instruction
void read_operands(unsigned char * PC, decode_info_t *di, const instr_info_t  * info)
{
  // First of all get the full instruction again
  unsigned int instruction = *PC;
  int bitShifter = 0;
  int temp;
  int i = 0;
  instruction = instruction << 8 | *(PC+1);
  instruction = instruction << 8 | *(PC+2);
  instruction = instruction << 8 | *(PC+3);

  printf("Instruction %.8x\n", instruction);

  // Go through the availible operands one by one and write the values into DI.
  if (info->dst1_type != TYPE_NONE)
  {
 //   printf("Destination 1 Exists!!\n");
    di->regDst1 = (instruction) & (info->dst1_mask);
  //  printf("Destion1 Hex: %x\n", info->dst1_mask);
    //printf("Dest1 Hex %x\n", di->regDst1);
    temp = info->dst1_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }

    di->regDst1 = di->regDst1 >> bitShifter;
    bitShifter = 0;
  }
  if (info->dst2_type != TYPE_NONE)
  {
  //  printf("Destination 2 Exists!!\n");
    di->regDst2 = instruction & info->dst2_mask;
 //  printf("Dest2 Hex %x\n", di->regDst2);
    temp = info->dst2_mask;
    for(i = 0 ; i < 32; i++)
      if(temp & 0x1 == 1)
        break;
      else
      {
        temp = temp >> 1;
        bitShifter ++;
      }

    di->regDst2 = di->regDst2 >> bitShifter;
    bitShifter = 0;
  }
  if (info->src1_type != TYPE_NONE)
  {
    printf("Source 1 Exists!!\n");
    if(info->src1_type != Reg)
    {
      di->immed = instruction & info->src1_mask;
      temp = info->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }

      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc1 = instruction & info->src1_mask;
     // printf("Src1 Hex %x\n", di->regSrc1);
      temp = info->src1_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }

      di->regSrc1 = di->regSrc1 >> bitShifter;
      bitShifter = 0;
    }
  }
  if (info->src2_type != TYPE_NONE)
  {
 //   printf("Source 2 Exists!!\n");
    if(info->src2_type != Reg)
    {
      di->immed = instruction & info->src2_mask;
      temp = info->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }

      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc2 = instruction & info->src2_mask;
  //           printf("Src2 Hex %x\n", di->regSrc2);
      temp = info->src2_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->regSrc2 = di->regSrc2 >> bitShifter;
    }
  }
  if (info->src3_type != TYPE_NONE)
  {
 //   printf("Source 3 Exists!!\n");
    if(info->src3_type != Reg)
    {
      di->immed = instruction & info->src3_mask;
      temp = info->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->immed = di->immed >> bitShifter;
      bitShifter = 0;
    }
    else
    {
      di->regSrc3 = instruction & info->src3_mask;
      temp = info->src3_mask;
      for(i = 0 ; i < 32; i++)
        if(temp & 0x1 == 1)
          break;
        else
        {
          temp = temp >> 1;
          bitShifter ++;
        }
      di->regSrc3 = di->regSrc3 >> bitShifter;
    }
  }
  return;
}

static byte *
read_instruction(byte *pc, byte *orig_pc,
                 const instr_info_t **ret_info, decode_info_t *di,
                 bool just_opcode _IF_DEBUG(bool report_invalid))
{
	// COMPLETEDD #558 read_instruction Has been gotten to but not ready to do yet
	unsigned int instruction = *pc;
	const instr_info_t *decodedInstruction = NULL;
	di->start_pc = pc;
	di->size_immed = OPSZ_NA;
  di->regDst1 = TYPE_NONE;
  di->regDst2 = TYPE_NONE;
  di->regSrc1 = TYPE_NONE;
  di->regSrc2 = TYPE_NONE;
  di->regSrc3 = TYPE_NONE;
  di->immed = 0;
  di->len = 4;

  // First of all we need to deal with the instruction as a whole not just a byte
  instruction = instruction << 8 | *(pc+1);
  instruction = instruction << 8 | *(pc+2);
  instruction = instruction << 8 | *(pc+3);

  // First we need to check what type of instruction it is
  int instrCat = decodeCommon(instruction);

  // With this information get the appropriate array to decode the instruction
  switch (instrCat)
    {
      case CAT_UNCONDITIONAL:
      {
        printf("Instruction is an Unconditional Instruction\n");
        decodedInstruction = NullInstruction;
        break;
      }
      case CAT_DATAPROC:
      {
        printf("Instruction is a Data Processing Instruction\n");
        instrCat = decodeDataProcessing(instruction);
        switch (instrCat)
        {
          case DP_REGISTER:
          {
            printf("Instruction is a Register Processing Instruction\n");
            decodedInstruction = DataProcessingInstructionsR;
            break;
          }
          case DP_REGISTERSR:
          {
            printf("Instruction is a Register Shifted Register Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_MISC:
          {
            printf("Instruction is a Misc Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_HWMMA:
          {
            printf("Instruction is a Halfword Multiply Accumumlate Instructions\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_MULTIPLYMA:
          {
            printf("Instruction is a Multiply and Accumulate Instruction\n");
            decodedInstruction = MultipyAndMultiplyAcc;
            break;
          }
          case DP_SP:
          {
            printf("Instruction is a Synchronization Primitive\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_EXTRALS:
          {
            printf("Instruction is an Extra Load/ Store Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_EXTRALSUP:
          {
            printf("Instruction is an Extra Load Store Unprivlaged Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_IMM:
          {
            printf("Instruction is a Data Processing IMM Instruction\n");
            decodedInstruction = DataProcessingInstructionsIMM;
            break;
          }
          case DP_IMMLD:
          {
            printf("Instruction is a 16 Bit Immediate load Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_IMMHILD:
          {
            printf("Instruction is a High Halfword Immediate Load Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_MSRIMM:
          {
            printf("Instruction is a MSR Immedate Instruction\n");
            decodedInstruction = NullInstruction;
            break;
          }
          case DP_UNDEFINED:
          {
            printf("Instruction is Undefined\n");
            decodedInstruction = NullInstruction;
            break;
          }
        }
        break;
      }
      case CAT_LOADSTORE1:
      {
        printf("Instruction is a Load Store Type 1 Instruction\n");
      }
      case CAT_LOADSTORE2:
      {
        printf("Instruction is a Load Store Type 2 Instruction\n");
        decodedInstruction = loadStore;
        break;
      }
      case CAT_BRANCH:
      {
        printf("Instruction is a Branch Insturction\n");
        decodedInstruction = BranchInstructions;
        break;
      }
      case CAT_SVC:
      {
        printf("Instruction is a Service Call Instruction\n");
        decodedInstruction = serviceCalls;
        break;
      }
      case CAT_UNDEFINED:
      {
        printf("Instruction is Undefined\n");
        decodedInstruction = NullInstruction;
        break;
      }
    }

  // Now cycle through instructions to find exact one
  while (decodedInstruction->type != OP_NULL)
  {
   //printf("Comparing %.8X with %.8X with mask of %.8X\n", instruction,
     // decodedInstruction->concernedBits, decodedInstruction->maskedBits);

    if((instruction & decodedInstruction->maskedBits) == decodedInstruction->concernedBits)
    break;

    decodedInstruction++;
  }
  printf("Final Instruction Opcode is: %s\n", decodedInstruction->name);

  /* if just want opcode, stop here!  faster for caller to
   * separately call decode_next_pc than for us to decode immeds!
   */
  if (just_opcode) {
      *ret_info = decodedInstruction;
      return NULL;
  }

  read_operands(pc,di, decodedInstruction);
  printf("Destination Operand 1 is: %d\n", di->regDst1);
  printf("Destination Operand 2 is: %d\n", di->regDst2);
  printf("Source Operand 1 is: %d\n", di->regSrc1);
  printf("Source Operand 2 is: %d\n", di->regSrc2);
  printf("Source Operand 3 is: %d\n", di->regSrc3);
  printf("Immediate Operand is: %d\n", di->immed);
  /* return values */
  *ret_info = decodedInstruction;
  return pc+4;
}
#else


opnd_size_t
resolve_var_reg_size(opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_1_reg4: return (is_reg ? OPSZ_4 : OPSZ_1);
    case OPSZ_2_reg4: return (is_reg ? OPSZ_4 : OPSZ_2);
    case OPSZ_4_reg16: return (is_reg ? OPSZ_16 : OPSZ_4);
    }
    return sz;
}

/* Like all our code, we assume cs specifies default data and address sizes.
 * This routine assumes the size varies by data, NOT by address!
 */
opnd_size_t
resolve_variable_size(decode_info_t *di/*IN: x86_mode, prefixes*/,
                      opnd_size_t sz, bool is_reg)
{
    switch (sz) {
    case OPSZ_2_short1: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_1 : OPSZ_2);
    case OPSZ_4_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4);
    case OPSZ_4x8: return (X64_MODE(di) ? OPSZ_8 : OPSZ_4);
    case OPSZ_4x8_short2:
        return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 :
                (X64_MODE(di) ? OPSZ_8 : OPSZ_4));
    case OPSZ_4x8_short2xi8:
        return (X64_MODE(di) ? (proc_get_vendor() == VENDOR_INTEL ? OPSZ_8 :
                              (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8)) :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_short2xi4:
        return ((X64_MODE(di) && proc_get_vendor() == VENDOR_INTEL) ? OPSZ_4 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8_short2: /* rex.w trumps data prefix */
        return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 :
                (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_4));
    case OPSZ_4_rex8: return (TEST(PREFIX_REX_W, di->prefixes) ? OPSZ_8 : OPSZ_4);
    case OPSZ_6_irex10_short4: /* rex.w trumps data prefix, but is ignored on AMD */
        DODEBUG({
            /* less annoying than a CURIOSITY assert when testing */
            if (TEST(PREFIX_REX_W, di->prefixes))
                SYSLOG_INTERNAL_INFO_ONCE("curiosity: rex.w on OPSZ_6_irex10_short4!");
        });
        return ((TEST(PREFIX_REX_W, di->prefixes) && proc_get_vendor() != VENDOR_AMD) ?
                OPSZ_10 : (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_6));
    case OPSZ_6x10: return (X64_MODE(di) ? OPSZ_10 : OPSZ_6);
    case OPSZ_8_short2: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_2 : OPSZ_8);
    case OPSZ_8_short4: return (TEST(PREFIX_DATA, di->prefixes) ? OPSZ_4 : OPSZ_8);
    case OPSZ_28_short14:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_14 : OPSZ_28);
    case OPSZ_108_short94:
        return (TEST(PREFIX_DATA, di->prefixes) ?  OPSZ_94 : OPSZ_108);
    case OPSZ_1_reg4:
    case OPSZ_2_reg4:
    case OPSZ_4_reg16:
        return resolve_var_reg_size(sz, is_reg);
    }
    return sz;
}

opnd_size_t
resolve_variable_size_dc(dcontext_t *dcontext, uint prefixes, opnd_size_t sz, bool is_reg)
{
    decode_info_t di;
    IF_X64(di.x86_mode = get_x86_mode(dcontext));
    di.prefixes = prefixes;
    return resolve_variable_size(&di, sz, is_reg);
}

opnd_size_t
resolve_addr_size(decode_info_t *di/*IN: x86_mode, prefixes*/)
{
    if (TEST(PREFIX_ADDR, di->prefixes))
        return (X64_MODE(di) ? OPSZ_4 : OPSZ_2);
    else
        return (X64_MODE(di) ? OPSZ_8 : OPSZ_4);
}

static byte *
read_immed(byte *pc, decode_info_t *di, opnd_size_t size, ptr_int_t *result)
{
    size = resolve_variable_size(di, size, false);

    /* all data immediates are sign-extended.  we use the compiler's casts with
     * signed types to do our sign extensions for us.
     */
    switch (size) {
    case OPSZ_1:
        *result = (ptr_int_t) (char) *pc; /* sign-extend */
        pc++;
        break;
    case OPSZ_2:
        *result = (ptr_int_t) *((short*)pc); /* sign-extend */
        pc += 2;
        break;
    case OPSZ_4:
        *result = (ptr_int_t) *((int*)pc); /* sign-extend */
        pc += 4;
        break;
    case OPSZ_8:
        CLIENT_ASSERT(X64_MODE(di), "decode immediate: invalid size");
        CLIENT_ASSERT(sizeof(ptr_int_t) == 8, "decode immediate: internal size error");
        *result = *((ptr_int_t*)pc);
        pc += 8;
        break;
    default:
        /* called internally w/ instr_info_t fields or hardcoded values,
         * so ok to assert */
        CLIENT_ASSERT(false, "decode immediate: unknown size");
    }
    return pc;
}
/* reads any trailing immed bytes */
static byte *
read_operand(byte *pc, decode_info_t *di, byte optype, opnd_size_t opsize)
{
    ptr_int_t val = 0;
    opnd_size_t size = opsize;
    switch (optype) {
    case TYPE_A:
        {
            CLIENT_ASSERT(!X64_MODE(di), "x64 has no type A instructions");
#ifdef IA32_ON_IA64
            /* somewhat hacked dispatch on size */
            if (opsize == OPSZ_4_short2) {
                pc = read_immed(pc, di, opsize, &val);
                break;
            }
#endif
            /* ok b/c only instr_info_t fields passed */
            CLIENT_ASSERT(opsize == OPSZ_6_irex10_short4, "decode A operand error");
            if (TEST(PREFIX_DATA, di->prefixes)) {
                /* 4-byte immed */
                pc = read_immed(pc, di, OPSZ_4, &val);
#ifdef X64
                if (!X64_MODE(di)) {
                    /* we do not want the sign extension that read_immed() applied */
                    val &= (ptr_int_t) 0x00000000ffffffff;
                }
#endif
                /* ok b/c only instr_info_t fields passed */
                CLIENT_ASSERT(di->size_immed == OPSZ_NA &&
                              di->size_immed2 == OPSZ_NA, "decode A operand error");
                di->size_immed = resolve_variable_size(di, opsize, false);
                ASSERT(di->size_immed == OPSZ_4);
                di->immed = val;
            } else {
                /* 6-byte immed */
                ptr_int_t val2 = 0;
                /* little-endian: segment comes last */
                pc = read_immed(pc, di, OPSZ_4, &val2);
                pc = read_immed(pc, di, OPSZ_2, &val);
#ifdef X64
                if (!X64_MODE(di)) {
                    /* we do not want the sign extension that read_immed() applied */
                    val2 &= (ptr_int_t) 0x00000000ffffffff;
                }
#endif
                /* ok b/c only instr_info_t fields passed */
                CLIENT_ASSERT(di->size_immed == OPSZ_NA &&
                              di->size_immed2 == OPSZ_NA, "decode A operand error");
                di->size_immed = resolve_variable_size(di, opsize, false);
                ASSERT(di->size_immed == OPSZ_6);
                di->size_immed2 = resolve_variable_size(di, opsize, false);
                di->immed = val;
                di->immed2 = val2;
            }
            return pc;
        }
    case TYPE_I:
        {
            pc = read_immed(pc, di, opsize, &val);
            break;
        }
    case TYPE_J:
        {
            byte *end_pc;
            pc = read_immed(pc, di, opsize, &val);
            if (di->orig_pc != di->start_pc) {
                CLIENT_ASSERT(di->start_pc != NULL,
                              "internal decode error: start pc not set");
                end_pc = di->orig_pc + (pc - di->start_pc);
            } else
                end_pc = pc;
            /* convert from relative offset to absolute target pc */
            val = ((ptr_int_t)end_pc) + val;
            if ((!X64_MODE(di) || proc_get_vendor() != VENDOR_INTEL) &&
                TEST(PREFIX_DATA, di->prefixes)) {
                /* need to clear upper 16 bits */
                val &= (ptr_int_t) 0x0000ffff;
            } /* for x64 Intel, always 64-bit addr ("f64" in Intel table) */
            break;
        }
    case TYPE_O:
        {
            /* no modrm byte, offset follows directly.  this is address-sized,
             * so 64-bit for x64, and addr prefix affects it. */
            size = resolve_addr_size(di);
            pc = read_immed(pc, di, size, &val);
            if (TEST(PREFIX_ADDR, di->prefixes)) {
                /* need to clear upper bits */
                if (X64_MODE(di))
                    val &= (ptr_int_t) 0xffffffff;
                else
                    val &= (ptr_int_t) 0x0000ffff;
            }
#ifdef X64
            if (!X64_MODE(di)) {
                /* we do not want the sign extension that read_immed() applied */
                val &= (ptr_int_t) 0x00000000ffffffff;
            }
#endif
            break;
        }
    default:
        return pc;
    }
    if (di->size_immed == OPSZ_NA) {
        di->size_immed = size;
        di->immed = val;
    } else {
        /* ok b/c only instr_info_t fields passed */
        CLIENT_ASSERT(di->size_immed2 == OPSZ_NA, "decode operand error");
        di->size_immed2 = size;
        di->immed2 = val;
    }
    return pc;
}

/* reads the modrm byte and any following sib and offset bytes */
static byte *
read_modrm(byte *pc, decode_info_t *di)
{
    byte modrm = *pc;
    pc++;
    di->modrm = modrm;
    di->mod = (byte)((modrm >> 6) & 0x3); /* top 2 bits */
    di->reg = (byte)((modrm >> 3) & 0x7); /* middle 3 bits */
    di->rm  = (byte)(modrm & 0x7);        /* bottom 3 bits */

    /* addr16 displacement */
    if (!X64_MODE(di) && TEST(PREFIX_ADDR, di->prefixes)) {
        di->has_sib = false;
        if ((di->mod == 0 && di->rm == 6) || di->mod == 2) {
            /* 2-byte disp */
            di->has_disp = true;
            if (di->mod == 0 && di->rm == 6) {
                /* treat absolute addr as unsigned */
                di->disp = (int) *((ushort *)pc); /* zero-extend */
            } else {
                /* treat relative addr as signed */
                di->disp = (int) *((short *)pc); /* sign-extend */
            }
            pc += 2;
        } else if (di->mod == 1) {
            /* 1-byte disp */
            di->has_disp = true;
            di->disp = (int) (char) *pc; /* sign-extend */
            pc++;
        } else {
            di->has_disp = false;
        }
    } else {
        /* 32-bit, which sometimes has a SIB */
        if (di->rm == 4 && di->mod != 3) {
            /* need SIB */
            byte sib = *pc;
            pc++;
            di->has_sib = true;
            di->scale = (byte)((sib >> 6) & 0x3); /* top 2 bits */
            di->index = (byte)((sib >> 3) & 0x7); /* middle 3 bits */
            di->base  = (byte)(sib & 0x7);        /* bottom 3 bits */
        } else {
            di->has_sib = false;
        }

        /* displacement */
        if ((di->mod == 0 && di->rm == 5) ||
            (di->has_sib && di->mod == 0 && di->base == 5) ||
            di->mod == 2) {
            /* 4-byte disp */
            di->has_disp = true;
            di->disp = *((int *)pc);
            IF_X64(di->disp_abs = pc); /* used to set instr->rip_rel_pos */
            pc += 4;
        } else if (di->mod == 1) {
            /* 1-byte disp */
            di->has_disp = true;
            di->disp = (int) (char) *pc; /* sign-extend */
            pc++;
        } else {
            di->has_disp = false;
        }
    }
    return pc;
}


static byte *
read_instruction(byte *pc, byte *orig_pc,
                 const instr_info_t **ret_info, decode_info_t *di,
                 bool just_opcode _IF_DEBUG(bool report_invalid))
{
    DEBUG_DECLARE(byte *post_suffix_pc = NULL;)
    byte first_instr_byte;
    const instr_info_t *info;
    /* these 3 prefixes may be part of opcode: */
    bool data_prefix = false;
    bool rep_prefix = false;
    bool repne_prefix = false;

    /* initialize di */
    /* though we only need di->start_pc for full decode rip-rel (and
     * there only post-read_instruction()) and decode_from_copy(), and
     * di->orig_pc only for decode_from_copy(), we assume that
     * high-perf decoding uses decode_cti() and live w/ the extra
     * writes here for decode_opcode() and decode_eflags_usage().
     */
    di->start_pc = pc;
    di->orig_pc = orig_pc;
    di->size_immed = OPSZ_NA;
    di->size_immed2 = OPSZ_NA;
    di->seg_override = REG_NULL;
    /* FIXME: set data and addr sizes to current mode
     * for now I assume always 32-bit mode (or 64 for X64_MODE(di))!
     */
    di->prefixes = 0;

    do {
        first_instr_byte = *pc;
        pc++;
        info = &first_byte[first_instr_byte];
        if (info->type == X64_EXT) {
            /* discard old info, get new one */
            info = &x64_extensions[info->code][X64_MODE(di) ? 1 : 0];
        }
        if (info->type == PREFIX) {
            if (TESTANY(PREFIX_REX_ALL, di->prefixes)) {
                /* rex.* must come after all other prefixes (including those that are
                 * part of the opcode, xref PR 271878): so discard them if before
                 * matching the behavior of decode_sizeof().  This in effect nops
                 * improperly placed rex prefixes which (xref PR 241563 and Intel Manual
                 * 2A 2.2.1) is the correct thing to do. NOTE - windbg shows early bytes
                 * as ??, objdump as their prefix names, separate from the next instr.
                 */
                di->prefixes &= ~PREFIX_REX_ALL;
            }
            if (info->code == PREFIX_REP) {
                /* see if used as part of opcode before considering prefix */
                rep_prefix = true;
            } else if (info->code == PREFIX_REPNE) {
                /* see if used as part of opcode before considering prefix */
                repne_prefix = true;
            } else if (REG_START_SEGMENT <= info->code &&
                       info->code <= REG_STOP_SEGMENT) {
                CLIENT_ASSERT_TRUNCATE(di->seg_override, byte, info->code,
                                       "decode error: invalid segment override");
                di->seg_override = (byte) info->code;
            } else if (info->code == PREFIX_DATA) {
                /* see if used as part of opcode before considering prefix */
                data_prefix = true;
            } else if (TESTANY(PREFIX_REX_ALL | PREFIX_ADDR | PREFIX_LOCK,
                               info->code)) {
                di->prefixes |= info->code;
            }
        } else
            break;
    } while (true);

    if (info->type == ESCAPE) {
        /* discard first byte, move to second */
        first_instr_byte = *pc; /* really 2nd, just reusing var */
        pc++;
        info = &second_byte[first_instr_byte];
    }
    if (info->type == ESCAPE_3BYTE_38 ||
        info->type == ESCAPE_3BYTE_3a) {
        /* discard second byte, move to third */
        first_instr_byte = *pc; /* really 3rd, just reusing var */
        pc++;
        if (info->type == ESCAPE_3BYTE_38)
            info = &third_byte_38[third_byte_38_index[first_instr_byte]];
        else
            info = &third_byte_3a[third_byte_3a_index[first_instr_byte]];
    }

    /* all FLOAT_EXT and PREFIX_EXT (except nop & pause) and EXTENSION need modrm,
     * get it now
     */
    if ((info->flags & HAS_MODRM) != 0)
        pc = read_modrm(pc, di);

    if (info->type == FLOAT_EXT) {
        if (di->modrm <= 0xbf) {
            int offs = (first_instr_byte - 0xd8) * 8 + di->reg;
            info = &float_low_modrm[offs];
        } else {
            int offs1 = (first_instr_byte - 0xd8);
            int offs2 = di->modrm - 0xc0;
            info = &float_high_modrm[offs1][offs2];
        }
    }
    else if (info->type == PREFIX_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix?1 :(data_prefix?2 :(repne_prefix?3 :0)));
        info = &prefix_extensions[code][idx];
        if (rep_prefix)
            rep_prefix = false;
        else if (data_prefix)
            data_prefix = false;
        else if (repne_prefix)
            repne_prefix = false;
        if (info->type == REX_EXT) {
            /* discard old info, get new one */
            int code = (int) info->code;
            /* currently indexed by rex.b only */
            int idx = (TEST(PREFIX_REX_B, di->prefixes) ? 1 : 0);
            info = &rex_extensions[code][idx];
        }
    }
    else if (info->type == REP_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix ? 2 : 0);
        info = &rep_extensions[code][idx];
        if (rep_prefix)
            rep_prefix = false;
    }
    else if (info->type == REPNE_EXT) {
        /* discard old info, get new one */
        int code = (int) info->code;
        int idx = (rep_prefix? 2 : (repne_prefix? 4 :0));
        info = &repne_extensions[code][idx];
        rep_prefix = false;
        repne_prefix = false;
    }
    else if (info->type == EXTENSION) {
        /* discard old info, get new one */
        info = &extensions[info->code][di->reg];
        /* absurd cases of using prefix on top of reg opcode extension
         * (pslldq, psrldq)
         */
        if (info->type == PREFIX_EXT) {
            /* discard old info, get new one */
            int code = (int) info->code;
            int idx = (rep_prefix?1 :(data_prefix?2 :(repne_prefix?3 :0)));
            info = &prefix_extensions[code][idx];
            if (rep_prefix)
                rep_prefix = false;
            else if (data_prefix)
                data_prefix = false;
            else if (repne_prefix)
                repne_prefix = false;
        } else if (info->type == MOD_EXT) {
            info = &mod_extensions[info->code][(di->mod==3) ? 1 : 0];
            /* Yes, we have yet another layer, thanks to Intel's poor choice
             * in opcodes -- why didn't they fill out the PREFIX_EXT space?
             */
            if (info->type == RM_EXT) {
                info = &rm_extensions[info->code][di->rm];
            }
        }
    }
    else if (info->type == SUFFIX_EXT) {
        /* Discard old info, get new one for complete opcode, which includes
         * a suffix byte where an immed would be (yes, ugly!).
         * We should have already read in the modrm (+ sib).
         */
        CLIENT_ASSERT(TEST(HAS_MODRM, info->flags), "decode error on 3DNow instr");
        info = &suffix_extensions[suffix_index[*pc]];
        pc++;
        DEBUG_DECLARE(post_suffix_pc = pc;)
    }

    if (TEST(REQUIRES_PREFIX, info->flags)) {
        byte required = (byte)(info->opcode >> 24);
        bool *prefix_var = NULL;
        CLIENT_ASSERT(info->opcode > 0xffffff, "decode error in SSSE3/SSE4 instr");
        if (required == DATA_PREFIX_OPCODE)
            prefix_var = &data_prefix;
        else if (required == REPNE_PREFIX_OPCODE)
            prefix_var = &repne_prefix;
        else if (required == REP_PREFIX_OPCODE)
            prefix_var = &rep_prefix;
        else
            CLIENT_ASSERT(false, "internal required-prefix error");
        if (prefix_var == NULL || !*prefix_var) {
            /* Invalid instr.  TODO: have processor w/ SSE4, confirm that
             * an exception really is raised.
             */
            info = NULL;
        } else
            *prefix_var = false;
    }

    /* at this point should be an instruction, so type should be an OP_ constant */
    if (info == NULL || info->type < OP_FIRST || info->type > OP_LAST ||
        (X64_MODE(di) && TEST(X64_INVALID, info->flags)) ||
        (!X64_MODE(di) && TEST(X86_INVALID, info->flags))) {
        /* invalid instruction: up to caller to decide what to do with it */
        /* FIXME case 10672: provide a runtime option to specify new
         * instruction formats */
        DODEBUG({
            /* don't report when decoding DR addresses, as we sometimes try to
             * decode backward (e.g., interrupted_inlined_syscall(): PR 605161)
             * XXX: better to pass in a flag when decoding that we are
             * being speculative!
             */
            if (report_invalid && !is_dynamo_address(di->start_pc)) {
                SYSLOG_INTERNAL_WARNING_ONCE("Invalid opcode encountered");
                if (info != NULL && info->type == INVALID) {
                    LOG(THREAD_GET, LOG_ALL, 1, "Invalid opcode @"PFX": 0x%x\n",
                        di->start_pc, info->opcode);
                } else {
                    int i;
                    dcontext_t *dcontext = get_thread_private_dcontext();
                    IF_X64(bool old_mode = set_x86_mode(dcontext, di->x86_mode);)
                    int sz = decode_sizeof(dcontext, di->start_pc, NULL _IF_X64(NULL));
                    IF_X64(set_x86_mode(dcontext, old_mode));
                    LOG(THREAD_GET, LOG_ALL, 1, "Error decoding "PFX" == ", di->start_pc);
                    for (i=0; i<sz; i++) {
                        LOG(THREAD_GET, LOG_ALL, 1, "0x%x ", *(di->start_pc+i));
                    }
                    LOG(THREAD_GET, LOG_ALL, 1, "\n");
                }
            }
        });
        *ret_info = &invalid_instr;
        return NULL;
    }

#ifdef INTERNAL
    DODEBUG({ /* rep & repne should have been completely handled by now */
        /* processor will typically ignore extra prefixes, but we log this internally
         * in case it's our decode messing up instead of weird app instrs
         */
        if (report_invalid &&
            ((rep_prefix &&
              /* case 6861: AMD64 opt: "rep ret" used if br tgt or after cbr */
              (pc != di->start_pc+2 || *(di->start_pc+1) != RAW_OPCODE_ret))
             || repne_prefix)) {
            char bytes[17*3];
            int i;
            dcontext_t *dcontext = get_thread_private_dcontext();
            IF_X64(bool old_mode = set_x86_mode(dcontext, di->x86_mode);)
            int sz = decode_sizeof(dcontext, di->start_pc, NULL _IF_X64(NULL));
            IF_X64(set_x86_mode(dcontext, old_mode));
            CLIENT_ASSERT(sz <= 17, "decode rep/repne error: unsupported opcode?");
            for (i=0; i<sz; i++)
                snprintf(&bytes[i*3], 3, "%02x ", *(di->start_pc+i));
            bytes[sz*3-1] = '\0'; /* -1 to kill trailing space */
            SYSLOG_INTERNAL_WARNING_ONCE("spurious rep/repne prefix @"PFX" (%s): "
                                         "decoding error?", di->start_pc, bytes);
        }
    });
#endif

    /* if just want opcode, stop here!  faster for caller to
     * separately call decode_next_pc than for us to decode immeds!
     */
    if (just_opcode) {
        *ret_info = info;
        return NULL;
    }

    if (data_prefix) {
        /* prefix was not part of opcode, it's a real prefix */
        /* From Intel manual:
         *   "For non-byte operations: if a 66H prefix is used with
         *   prefix (REX.W = 1), 66H is ignored."
         * That means non-byte-specific operations, for which 66H is
         * ignored as well, right?
         * Xref PR 593593.
         * Note that this means we could assert or remove some of
         * the "rex.w trumps data prefix" logic elsewhere in this file.
         */
        if (TEST(PREFIX_REX_W, di->prefixes)) {
            LOG(THREAD_GET, LOG_ALL, 3,
                "Ignoring 0x66 in presence of rex.w @"PFX"\n", di->start_pc);
        } else {
            di->prefixes |= PREFIX_DATA;
        }
    }

    /* read any trailing immediate bytes */
    if (info->dst1_type != TYPE_NONE)
        pc = read_operand(pc, di, info->dst1_type, info->dst1_size);
    if (info->dst2_type != TYPE_NONE)
        pc = read_operand(pc, di, info->dst2_type, info->dst2_size);
    if (info->src1_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src1_type, info->src1_size);
    if (info->src2_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src2_type, info->src2_size);
    if (info->src3_type != TYPE_NONE)
        pc = read_operand(pc, di, info->src3_type, info->src3_size);

    if (info->type == SUFFIX_EXT) {
        /* Shouldn't be any more bytes (immed bytes) read after the modrm+suffix! */
        DODEBUG({CLIENT_ASSERT(pc == post_suffix_pc, "decode error on 3DNow instr");});
    }

    /* return values */
    *ret_info = info;
    return pc;
}
#endif

/* Decodes the opcode and eflags usage of instruction at address pc
 * into instr.
 * This corresponds to a Level 2 decoding.
 * Assumes that instr is already initialized, but uses the x86/x64 mode
 * for the current thread rather than that set in instr.
 * If caller is re-using same instr struct over multiple decodings,
 * should call instr_reset or instr_reuse.
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instruction.
 */
byte *
decode_opcode(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	// COMPLETEDD #559 decode_opcode
	printf("Starting decode.c decode_opcode\n");
    const instr_info_t *info;
    decode_info_t di;
    int sz;
    /* when pass true to read_instruction it doesn't decode immeds,
     * so have to call decode_next_pc, but that ends up being faster
     * than decoding immeds!
     */
    read_instruction(pc, pc, &info, &di, true /* just opcode */
                     _IF_DEBUG(!TEST(INSTR_IGNORE_INVALID, instr->flags)));
#ifndef ARM
    sz = decode_sizeof(dcontext, pc, NULL _IF_X64(&rip_rel_pos));
#endif
    instr_set_opcode(instr, info->type);
    /* read_instruction sets opcode to OP_INVALID for illegal instr.
     * decode_sizeof will return 0 for _some_ illegal instrs, so we
     * check it first since it's faster than instr_valid, but we have to
     * also check instr_valid to catch all illegal instrs.
     */
#ifndef ARM
    if (sz == 0 || !instr_valid(instr)) {
        CLIENT_ASSERT(!instr_valid(instr), "decode_opcode: invalid instr");
        return NULL;
    }

    instr->eflags = info->eflags;
    instr_set_eflags_valid(instr, true);
#else
    instr->apsr = info->APSR;
    instr_set_apsr_valid(instr, true);
#endif
    /* operands are NOT set */
    instr_set_operands_valid(instr, false);
    /* raw bits are valid though and crucial for encoding */
    instr_set_raw_bits(instr, pc, sz);
//    /* must set rip_rel_pos after setting raw bits */
    return pc + 4;
}

/* Decodes the instruction at address pc into instr, filling in the
 * instruction's opcode, eflags usage, prefixes, and operands.
 * This corresponds to a Level 3 decoding.
 * Assumes that instr is already initialized, but uses the x86/x64 mode
 * for the current thread rather than that set in instr.
 * If caller is re-using same instr struct over multiple decodings,
 * should call instr_reset or instr_reuse.
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instruction.
 */
static byte *
decode_common(dcontext_t *dcontext, byte *pc, byte *orig_pc, instr_t *instr)
{
	// COMPLETEDD #557 decode_common
	printf("Starting decode_common\n");
    const instr_info_t *info;
    decode_info_t di;
    byte *next_pc;
    int instr_num_dsts = 0, instr_num_srcs = 0;
    opnd_t dsts[8];
    opnd_t srcs[8];

    CLIENT_ASSERT(instr->opcode == OP_INVALID || instr->opcode == OP_UNDECODED,
                  "decode: instr is already decoded, may need to call instr_reset()");

    IF_X64(di.x86_mode = get_x86_mode(dcontext));
    next_pc = read_instruction(pc, orig_pc, &info, &di, false /* not just opcode,
                                                                 decode operands too */
                               _IF_DEBUG(!TEST(INSTR_IGNORE_INVALID, instr->flags)));
    instr_set_opcode(instr, info->type);

    /* failure up to this point handled fine -- we set opcode to OP_INVALID */
    if (next_pc == NULL) {
        LOG(THREAD, LOG_INTERP, 3, "decode: invalid instr at "PFX"\n", pc);
        CLIENT_ASSERT(!instr_valid(instr), "decode: invalid instr");
        return NULL;
    }
#ifndef ARM
    instr->eflags = info->eflags;
    instr_set_eflags_valid(instr, true);
#else
    instr->apsr = info->APSR;
    instr_set_apsr_valid(instr, true);
#endif
    /* since we don't use set_src/set_dst we must explicitly say they're valid */
    instr_set_operands_valid(instr, true);
    /* read_instruction doesn't set di.len since only needed for rip-relative opnds */
    IF_X64(CLIENT_ASSERT_TRUNCATE(di.len, int, next_pc - pc,
                                  "internal truncation error"));
    di.len = (int) (next_pc - pc);

#ifndef ARM
    instr->prefixes |= di.prefixes;
#endif

    /* operands */
#ifndef ARM
    do {
        if (info->dst1_type != TYPE_NONE) {
            if (!decode_operand(&di, info->dst1_type, di->dst1_size,
                                &(dsts[instr_num_dsts++])))
                goto decode_invalid;
            ASSERT(check_is_variable_size(dsts[instr_num_dsts-1]));
        }
        if (info->dst2_type != TYPE_NONE) {
            if (!decode_operand(&di, info->dst2_type, info->dst2_size,
                                &(dsts[instr_num_dsts++])))
                goto decode_invalid;
            ASSERT(check_is_variable_size(dsts[instr_num_dsts-1]));
        }
        if (info->src1_type != TYPE_NONE) {
            if (!decode_operand(&di, info->src1_type, info->src1_size,
                                &(srcs[instr_num_srcs++])))
                goto decode_invalid;
            ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        }
        if (info->src2_type != TYPE_NONE) {
            if (!decode_operand(&di, info->src2_type, info->src2_size,
                                &(srcs[instr_num_srcs++])))
                goto decode_invalid;
            ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        }
        if (info->src3_type != TYPE_NONE) {
            if (!decode_operand(&di, info->src3_type, info->src3_size,
                                &(srcs[instr_num_srcs++])))
                goto decode_invalid;
            ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        }
        /* extra operands:
         * we take advantage of the fact that all instructions that need extra
         * operands have only one encoding, so the code field points to instr_info_t
         * structures containing the extra operands
         */
        if ((info->flags & HAS_EXTRA_OPERANDS) != 0) {
            if ((info->flags & EXTRAS_IN_CODE_FIELD) != 0)
                info = (const instr_info_t *)(info->code);
            else /* extra operands are in next entry */
                info = info + 1;
        } else
            break;
    } while (true);
#else
         if (info->dst1_type != TYPE_NONE) {
             if (!decode_operand(&di, info->dst1_type, di.regDst1,
                                 &(dsts[instr_num_dsts++])))
                 goto decode_invalid;
             ASSERT(check_is_variable_size(dsts[instr_num_dsts-1]));
         }
         if (info->dst2_type != TYPE_NONE) {
             if (!decode_operand(&di, info->dst2_type, di.regDst2,
                                 &(dsts[instr_num_dsts++])))
                 goto decode_invalid;
             ASSERT(check_is_variable_size(dsts[instr_num_dsts-1]));
         }
         if (info->src1_type != TYPE_NONE) {
        	 if(info->src1_type != Immediate)
        	 {
             if (!decode_operand(&di, info->src1_type, di.regSrc1,
                                 &(srcs[instr_num_srcs++])))
                 goto decode_invalid;
             ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
        	 else
        	 {
        		 if (!decode_operand(&di, info->src1_type, di.immed,
        				 	 	 	 	 	 	 	 	 &(srcs[instr_num_srcs++])))
        			 goto decode_invalid;
        		 ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
         }
         if (info->src2_type != TYPE_NONE) {
        	 if(info->src2_type != Immediate)
        	 {
             if (!decode_operand(&di, info->src2_type, di.regSrc2,
                                 &(srcs[instr_num_srcs++])))
                 goto decode_invalid;
             ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
        	 else
        	 {
        		 if (!decode_operand(&di, info->src2_type, di.immed,
        				 	 	 	 	 	 	 	 	 &(srcs[instr_num_srcs++])))
        			 goto decode_invalid;
        		 ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
         }
         if (info->src3_type != TYPE_NONE) {
        	 if(info->src3_type != Immediate)
        	 {
             if (!decode_operand(&di, info->src3_type, di.regSrc1,
                                 &(srcs[instr_num_srcs++])))
                 goto decode_invalid;
             ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
        	 else
        	 {
        		 if (!decode_operand(&di, info->src3_type, di.immed,
        				 	 	 	 	 	 	 	 	 &(srcs[instr_num_srcs++])))
        			 goto decode_invalid;
        		 ASSERT(check_is_variable_size(srcs[instr_num_srcs-1]));
        	 }
         }
#endif

#ifndef ARM
    /* some operands add to di.prefixes so we copy again */
    instr->prefixes |= di.prefixes;
    if (di.seg_override == SEG_FS)
        instr->prefixes |= PREFIX_SEG_FS;
    if (di.seg_override == SEG_GS)
        instr->prefixes |= PREFIX_SEG_GS;
#endif
    /* now copy operands into their real slots */
    instr_set_num_opnds(dcontext, instr, instr_num_dsts, instr_num_srcs);
    if (instr_num_dsts > 0) {
        memcpy(instr->dsts, dsts, instr_num_dsts*sizeof(opnd_t));
    }
    if (instr_num_srcs > 0) {
        /* remember that src0 is static */
        instr->src0 = srcs[0];
        if (instr_num_srcs > 1) {
            memcpy(instr->srcs, &(srcs[1]), (instr_num_srcs-1)*sizeof(opnd_t));
        }
    }

#ifndef ARM
    /* check for invalid prefixes that depend on operand types */
    if (TEST(PREFIX_LOCK, di.prefixes)) {
        /* check for invalid opcode, list on p3-397 of IA-32 vol 2 */
        switch (instr_get_opcode(instr)) {
        case OP_add: case OP_adc: case OP_and: case OP_btc: case OP_btr: case OP_bts:
        case OP_cmpxchg: case OP_cmpxchg8b: case OP_dec: case OP_inc: case OP_neg:
        case OP_not: case OP_or: case OP_sbb: case OP_sub: case OP_xor: case OP_xadd:
        case OP_xchg: {
            /* still illegal unless dest is mem op rather than src */
            CLIENT_ASSERT(instr->num_dsts > 0, "internal lock prefix check error");
            if (!opnd_is_memory_reference(instr->dsts[0])) {
                LOG(THREAD, LOG_INTERP, 3, "decode: invalid lock prefix at "PFX"\n", pc);
                goto decode_invalid;
            }
            break;
        }
        default: {
            LOG(THREAD, LOG_INTERP, 3, "decode: invalid lock prefix at "PFX"\n", pc);
            goto decode_invalid;
        }
        }
    }
#endif

    if (orig_pc != pc) {
        /* We do not want to copy when encoding and condone an invalid
         * relative target
         */
        instr_set_raw_bits_valid(instr, false);
        instr_set_translation(instr, orig_pc);
    } else {
        /* we set raw bits AFTER setting all srcs and dsts b/c setting
         * a src or dst marks instr as having invalid raw bits
         */
        instr_set_raw_bits(instr, pc, (uint)(next_pc - pc));

    }

    return next_pc;

 decode_invalid:
    instr_set_operands_valid(instr, false);
    instr_set_opcode(instr, OP_INVALID);
    return NULL;
}

byte *
decode(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	// COMPLETEDD #560 decode
    return decode_common(dcontext, pc, pc, instr);
}
