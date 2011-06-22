#include "../globals.h"
#include "decode_fast.h"
#include "../link.h"
#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "disassemble.h"

#ifndef ARM
enum {
    VARLEN_NONE,
    VARLEN_MODRM,
    VARLEN_FP_OP,
    VARLEN_ESCAPE, /* 2-byte opcodes */
    VARLEN_3BYTE_38_ESCAPE, /* 3-byte opcodes 0f 38 */
    VARLEN_3BYTE_3A_ESCAPE, /* 3-byte opcodes 0f 3a */
};

/* Data table for fixed part of an x86 instruction.  The table is
   indexed by the 1st (primary) opcode byte.  Zero entries are
   reserved opcodes. */
static const byte fixed_length[256] = {
    1,1,1,1, 2,5,1,1, 1,1,1,1, 2,5,1,1,  /* 0 */
    1,1,1,1, 2,5,1,1, 1,1,1,1, 2,5,1,1,  /* 1 */
    1,1,1,1, 2,5,1,1, 1,1,1,1, 2,5,1,1,  /* 2 */
    1,1,1,1, 2,5,1,1, 1,1,1,1, 2,5,1,1,  /* 3 */

    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 4 */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 5 */
    1,1,1,1, 1,1,1,1, 5,5,2,2, 1,1,1,1,  /* 6 */
    2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,  /* 7 */

    2,5,2,2, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 8 */
    1,1,1,1, 1,1,1,1, 1,1,7,1, 1,1,1,1,  /* 9 */
    5,5,5,5, 1,1,1,1, 2,5,1,1, 1,1,1,1,  /* A */
    2,2,2,2, 2,2,2,2, 5,5,5,5, 5,5,5,5,  /* B */

    2,2,3,1, 1,1,2,5, 4,1,3,1, 1,2,1,1,  /* C */
    1,1,1,1, 2,2,1,1, 1,1,1,1, 1,1,1,1,  /* D */
    2,2,2,2, 2,2,2,2, 5,5,7,2, 1,1,1,1,  /* E */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1   /* F */
    /* f6 and f7 OP_test immeds are handled specially in decode_sizeof() */
};

/* Data table for fixed immediate part of an x86 instruction that
   depends upon the existence of an operand-size byte.  The table is
   indexed by the 1st (primary) opcode byte.  Entries with non-zero
   values indicate opcodes with a variable-length immediate field.  We
   use this table if we've seen a operand-size prefix byte to adjust
   the fixed_length from dword to word.
 */
static const signed char immed_adjustment[256] = {
     0, 0, 0, 0,  0,-2, 0, 0,  0, 0, 0, 0,  0,-2, 0, 0,  /* 0 */
     0, 0, 0, 0,  0,-2, 0, 0,  0, 0, 0, 0,  0,-2, 0, 0,  /* 1 */
     0, 0, 0, 0,  0,-2, 0, 0,  0, 0, 0, 0,  0,-2, 0, 0,  /* 2 */
     0, 0, 0, 0,  0,-2, 0, 0,  0, 0, 0, 0,  0,-2, 0, 0,  /* 3 */

     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 4 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 5 */
     0, 0, 0, 0,  0, 0, 0, 0, -2,-2, 0, 0,  0, 0, 0, 0,  /* 6 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 7 */

     0,-2, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 8 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0,-2, 0,  0, 0, 0, 0,  /* 9 */
     0, 0, 0, 0,  0, 0, 0, 0,  0,-2, 0, 0,  0, 0, 0, 0,  /* A */
     0, 0, 0, 0,  0, 0, 0, 0, -2,-2,-2,-2, -2,-2,-2,-2,  /* B */

     0, 0, 0, 0,  0, 0, 0,-2,  0, 0, 0, 0,  0, 0, 0, 0,  /* C */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* D */
     0, 0, 0, 0,  0, 0, 0, 0, -2,-2,-2,-2,  0, 0, 0, 0,  /* E */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0   /* F */
};

/* Data table for fixed immediate part of an x86 instruction that
 * depends upon the existence of an address-size byte.  The table is
 * indexed by the 1st (primary) opcode byte.
 * The value here is doubled for x64 mode.
 */
static const signed char disp_adjustment[256] = {
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 0 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 1 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 2 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 3 */

     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 4 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 5 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 6 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 7 */

     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 8 */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* 9 */
    -2,-2,-2,-2,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* A */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* B */

     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* C */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* D */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  /* E */
     0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0   /* F */
};

/* Some macros to make the following table look better. */
#define m VARLEN_MODRM
#define f VARLEN_FP_OP
#define e VARLEN_ESCAPE

/* Data table indicating what function to use to calculate
   the variable part of the x86 instruction.  This table
   is indexed by the primary opcode.  */
static const byte variable_length[256] = {
    m,m,m,m, 0,0,0,0, m,m,m,m, 0,0,0,e,   /* 0 */
    m,m,m,m, 0,0,0,0, m,m,m,m, 0,0,0,0,   /* 1 */
    m,m,m,m, 0,0,0,0, m,m,m,m, 0,0,0,0,   /* 2 */
    m,m,m,m, 0,0,0,0, m,m,m,m, 0,0,0,0,   /* 3 */

    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* 4 */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* 5 */
    0,0,m,m, 0,0,0,0, 0,m,0,m, 0,0,0,0,   /* 6 */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* 7 */

    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m,   /* 8 */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* 9 */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* A */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* B */

    m,m,0,0, m,m,m,m, 0,0,0,0, 0,0,0,0,   /* C */
    m,m,m,m, 0,0,0,0, f,f,f,f, f,f,f,f,   /* D */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,   /* E */
    0,0,0,0, 0,0,m,m, 0,0,0,0, 0,0,m,m    /* F */
};

/* eliminate the macros */
#undef m
#undef f
#undef e


/* Data table for the additional fixed part of a two-byte opcode.
 * This table is indexed by the 2nd opcode byte.  Zero entries are
 * reserved/bad opcodes.
 * N.B.: none of these (except IA32_ON_IA64) need adjustment
 * for data16 or addr16.
 */
static const byte escape_fixed_length[256] = {
    1,1,1,1, 0,1,1,1, 1,1,0,1, 0,1,1,2,  /* 0 */ /* 0f0f has extra suffix opcode byte */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 1 */
    1,1,1,1, 0,0,0,0, 1,1,1,1, 1,1,1,1,  /* 2 */
    1,1,1,1, 1,1,0,0, 1,0,1,0, 0,0,0,0,  /* 3 */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 4 */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 5 */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 6 */
    2,2,2,2, 1,1,1,1, 1,1,0,0, 1,1,1,1,  /* 7 */

    5,5,5,5, 5,5,5,5, 5,5,5,5, 5,5,5,5,  /* 8 */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* 9 */
    1,1,1,1, 2,1,0,0, 1,1,1,1, 2,1,1,1,  /* A */
#ifdef IA32_ON_IA64
    /* change is the 5, could also be 3 depending on which mode we are */
    /* FIXME : no modrm byte so the standard variable thing won't work */
    /* (need a escape_disp_adjustment table) */
    1,1,1,1, 1,1,1,1, 5,1,2,1, 1,1,1,1,  /* B */
#else
    1,1,1,1, 1,1,1,1, 1,1,2,1, 1,1,1,1,  /* B */
#endif

    1,1,2,1, 2,2,2,1, 1,1,1,1, 1,1,1,1,  /* C */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* D */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* E */
    1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0   /* F */
    /* 0f78 has immeds depending on prefixes: handled in decode_sizeof() */
};

/* Some macros to make the following table look better. */
#define m VARLEN_MODRM
#define e1 VARLEN_3BYTE_38_ESCAPE
#define e2 VARLEN_3BYTE_3A_ESCAPE

/* Data table indicating what function to use to calcuate
   the variable part of the escaped x86 instruction.  This table
   is indexed by the 2nd opcode byte.  */
static const byte escape_variable_length[256] = {
    m,m,m,m, 0,0,0,0, 0,0,0,0, 0,m,0,m, /* 0 */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* 1 */
    m,m,m,m, 0,0,0,0, m,m,m,m, m,m,m,m, /* 2 */
    0,0,0,0, 0,0,0,0, e1,0,e2,0, 0,0,0,0, /* 3 */

    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* 4 */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* 5 */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* 6 */
    m,m,m,m, m,m,m,0, m,m,0,0, m,m,m,m, /* 7 */

    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, /* 8 */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* 9 */
    0,0,0,m, m,m,0,0, 0,0,0,m, m,m,m,m, /* A */
#ifdef IA32_ON_IA64
    m,m,m,m, m,m,m,m, 0,0,m,m, m,m,m,m, /* B */
#else
    m,m,m,m, m,m,m,m, m,0,m,m, m,m,m,m, /* B */
#endif
    m,m,m,m, m,m,m,m, 0,0,0,0, 0,0,0,0, /* C */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* D */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,m, /* E */
    m,m,m,m, m,m,m,m, m,m,m,m, m,m,m,0  /* F */
};

/* eliminate the macros */
#undef m
#undef e
/* Prototypes for the functions that calculate the variable
 * part of the x86 instruction length. */
static int sizeof_modrm(dcontext_t *dcontext, byte *pc, bool addr16
                        _IF_X64(byte **rip_rel_pc));
static int sizeof_fp_op(dcontext_t *dcontext, byte *pc, bool addr16
                        _IF_X64(byte **rip_rel_pc));
static int sizeof_escape(dcontext_t *dcontext, byte *pc, bool addr16
                         _IF_X64(byte **rip_rel_pc));

/* Two-byte opcode map (Tables A-4 and A-5).  You use this routine
 * when you have identified the primary opcode as 0x0f.  You pass this
 * routine the next byte to determine the number of extra bytes in the
 * entire instruction.
 * May return 0 size for certain invalid instructions.
 */
static int
sizeof_escape(dcontext_t *dcontext, byte *pc, bool addr16 _IF_X64(byte **rip_rel_pc))
{
    uint opc = (uint)*pc;
    int sz = escape_fixed_length[opc];
    ushort varlen = escape_variable_length[opc];

    /* for a valid instr, sz must be > 0 here, but we don't want to assert
     * since we need graceful failure
     */

    if (varlen == VARLEN_MODRM)
        return sz + sizeof_modrm(dcontext, pc+1, addr16 _IF_X64(rip_rel_pc));
    else if (varlen == VARLEN_3BYTE_38_ESCAPE) {
        opc = *(++pc);
        /* so far all 3-byte instrs have modrm bytes */
        /* to be robust for future additions we don't actually
         * use the threebyte_38_fixed_length[opc] entry and assume 1 */
        return sz + 1 + sizeof_modrm(dcontext, pc+1, addr16 _IF_X64(rip_rel_pc));
    }
    else if (varlen == VARLEN_3BYTE_3A_ESCAPE) {
        opc = *(++pc);
        /* so far all 0f 3a 3-byte instrs have modrm bytes and 1-byte immeds */
        /* to be robust for future additions we don't actually
         * use the threebyte_3a_fixed_length[opc] entry and assume 1 */
        return sz + 1 + sizeof_modrm(dcontext, pc+1, addr16 _IF_X64(rip_rel_pc)) + 1;
    }
    else
        CLIENT_ASSERT(varlen == VARLEN_NONE, "internal decoding error");

    return sz;
}

/* General floating-point instruction formats (Table B-22).  You use
 * this routine when you have identified the primary opcode as one in
 * the range 0xb8 through 0xbf.  You pass this routine the next byte
 * to determine the number of extra bytes in the entire
 * instruction. */
static int
sizeof_fp_op(dcontext_t *dcontext, byte *pc, bool addr16 _IF_X64(byte **rip_rel_pc))
{
    if (*pc > 0xbf)
        return 1;       /* entire ModR/M byte is an opcode extension */

    /* fp opcode in reg/opcode field */
    return sizeof_modrm(dcontext, pc, addr16 _IF_X64(rip_rel_pc));
}


/* 32-bit addressing forms with the ModR/M Byte (Table 2-2).  You call
 * this routine with the byte following the primary opcode byte when you
 * know that the operation's next byte is a ModR/M byte.  This routine
 * passes back the size of the Eaddr specification in bytes based on the
 * following encoding of Table 2-2.
 *
 *   Mod        R/M
 *        0 1 2 3 4 5 6 7
 *    0   1 1 1 1 * 5 1 1
 *    1   2 2 2 2 3 2 2 2
 *    2   5 5 5 5 6 5 5 5
 *    3   1 1 1 1 1 1 1 1
 *   where (*) is 6 if base==5 and 2 otherwise.
 */
static int
sizeof_modrm(dcontext_t *dcontext, byte *pc, bool addr16 _IF_X64(byte **rip_rel_pc))
{
    int l = 0;          /* return value for sizeof(eAddr) */

    uint modrm = (uint)*pc;
    int r_m = modrm & 0x7;
    uint mod = modrm >> 6;
    uint sib;

#ifdef X64
    if (rip_rel_pc != NULL && X64_MODE_DC(dcontext) && mod == 0 && r_m == 5) {
        *rip_rel_pc = pc + 1; /* no sib: next 4 bytes are disp */
    }
#endif

    if (addr16 && !X64_MODE_DC(dcontext)) {
        if (mod == 1)
            return 2; /* modrm + disp8 */
        else if (mod == 2)
            return 3; /* modrm + disp16 */
        else if (mod == 3)
            return 1; /* just modrm */
        else {
            CLIENT_ASSERT(mod == 0, "internal decoding error on addr16 prefix");
            if (r_m == 6)
                return 3; /* modrm + disp16 */
            else
                return 1; /* just modrm */
        }
        CLIENT_ASSERT(false, "internal decoding error on addr16 prefix");
    }

    /* for x64, addr16 simply truncates the computed address: there is
     * no change in disp sizes */

    if (mod == 3)       /* register operand */
        return 1;

    switch (mod) {      /* memory or immediate operand */
      case 0: l = (r_m == 5) ? 5 : 1; break;
      case 1: l = 2; break;
      case 2: l = 5; break;
    }
    if (r_m == 4) {
        l += 1;         /* adjust for sib byte */
        sib = (uint)(*(pc+1));
        if ((sib & 0x7) == 5) {
            if (mod == 0)
                l += 4; /* disp32(,index,s) */
        }
    }

    return l;
}

#endif
byte *
decode_cti(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	return 0;
}

#ifndef ARM
/* Returns the length of the instruction at pc.
 * If num_prefixes is non-NULL, returns the number of prefix bytes.
 * If rip_rel_pos is non-NULL, returns the offset into the instruction
 * of a rip-relative addressing displacement (for data only: ignores
 * control-transfer relative addressing), or 0 if none.
 * May return 0 size for certain invalid instructions
 */
int
decode_sizeof(dcontext_t *dcontext, byte *start_pc, int *num_prefixes
              _IF_X64(uint *rip_rel_pos))
{
	byte *pc = start_pc;
	uint opc = (uint)*pc;
	int sz = 0;
	ushort varlen;
	bool word_operands = false; /* data16 */
	bool qword_operands = false; /* rex.w */
	bool addr16 = false; /* really "addr32" for x64 mode */
	bool found_prefix = true;
	bool rep_prefix = false;
	byte reg_opcode;    /* reg_opcode field of modrm byte */
	#ifdef X64
	byte *rip_rel_pc = NULL;
	#endif
/* Check for prefix byte(s) */
while (found_prefix) {
    /* NOTE - rex prefixes must come after all other prefixes (including
     * prefixes that are part of the opcode xref PR 271878).  We match
     * read_instruction() in considering pre-prefix rex bytes as part of
     * the following instr, event when ignored, rather then treating them
     * as invalid.  This in effect nops improperly placed rex prefixes which
     * (xref PR 241563 and Intel Manual 2A 2.2.1) is the correct thing to do.
     * Rex prefixes are 0x40-0x4f; >=0x48 has rex.w bit set.
     */
    if (X64_MODE_DC(dcontext) && opc >= REX_PREFIX_BASE_OPCODE &&
        opc <= (REX_PREFIX_BASE_OPCODE | REX_PREFIX_ALL_OPFLAGS)) {
        if (opc >= (REX_PREFIX_BASE_OPCODE | REX_PREFIX_W_OPFLAG)) {
            qword_operands = true;
            if (word_operands)
                word_operands = false; /* rex.w trumps data16 */
        } /* else, doesn't affect instr size */
        opc = (uint)*(++pc);
        sz += 1;
    } else {
        switch (opc) {
        case 0x66:    /* operand size */
            /* rex.w before other prefixes is a nop */
            if (qword_operands)
                qword_operands = false;
            word_operands = true;
            opc = (uint)*(++pc);
            sz += 1;
            break;
        case 0xf2: case 0xf3: /* REP */
            rep_prefix = true;
            /* fall through */
        case 0xf0:          /* LOCK */
        case 0x64: case 0x65: /* segment overrides */
        case 0x26: case 0x36:
        case 0x2e: case 0x3e:
            opc = (uint)*(++pc);
            sz += 1;
            break;
        case 0x67:
            addr16 = true;
            opc = (uint)*(++pc);
            sz += 1;
            /* up to caller to check for addr prefix! */
            break;
        default:
            found_prefix = false;
        }
    }
}
if (num_prefixes != NULL)
    *num_prefixes = sz;
if (word_operands) {
#ifdef X64
    /* for x64 Intel, always 64-bit addr ("f64" in Intel table)
     * FIXME: what about 2-byte jcc?
     */
    if (X64_MODE_DC(dcontext) && proc_get_vendor() == VENDOR_INTEL)
        sz += immed_adjustment_intel64[opc];
    else
#endif
        sz += immed_adjustment[opc]; /* no adjustment for 2-byte escapes */
}
if (addr16) {  /* no adjustment for 2-byte escapes */
    if (X64_MODE_DC(dcontext)) /* from 64 bits down to 32 bits */
        sz += 2*disp_adjustment[opc];
    else /* from 32 bits down to 16 bits */
        sz += disp_adjustment[opc];
}
#ifdef X64
if (X64_MODE_DC(dcontext)) {
    int adj64 = x64_adjustment[opc];
    if (adj64 > 0) /* default size adjustment */
        sz += adj64;
    else if (qword_operands)
        sz += -adj64; /* negative indicates prefix, not default, adjust */
    /* else, no adjustment */
}
#endif

/* opc now really points to opcode */
sz += fixed_length[opc];
varlen = variable_length[opc];

/* for a valid instr, sz must be > 0 here, but we don't want to assert
 * since we need graceful failure
 */

if (varlen == VARLEN_MODRM)
    sz += sizeof_modrm(dcontext, pc+1, addr16 _IF_X64(&rip_rel_pc));
else if (varlen == VARLEN_ESCAPE) {
    sz += sizeof_escape(dcontext, pc+1, addr16 _IF_X64(&rip_rel_pc));
    /* special case: Intel and AMD added size-differing prefix-dependent instrs! */
    if (*(pc+1) == 0x78) {
        if (word_operands || rep_prefix) {
            /* extrq, insertq: 2 1-byte immeds */
            sz += 2;
        } /* else, vmread, w/ no immeds */
    }
} else if (varlen == VARLEN_FP_OP)
    sz += sizeof_fp_op(dcontext, pc+1, addr16 _IF_X64(&rip_rel_pc));
else
    CLIENT_ASSERT(varlen == VARLEN_NONE, "internal decoding error");

/* special case that doesn't fit the mold (of course one had to exist) */
reg_opcode = (byte) (((*(pc + 1)) & 0x38) >> 3);
if (opc == 0xf6 && reg_opcode == 0) {
    sz += 1;        /* TEST Eb,ib -- add size of immediate */
} else if (opc == 0xf7 && reg_opcode == 0) {
    if (word_operands)
        sz += 2;    /* TEST Ew,iw -- add size of immediate */
    else
        sz += 4;    /* TEST El,il -- add size of immediate */
}

#ifdef X64
if (rip_rel_pos != NULL) {
    if (rip_rel_pc != NULL) {
        CLIENT_ASSERT(X64_MODE_DC(dcontext),
                      "decode_sizeof: invalid non-x64 rip_rel instr");
        CLIENT_ASSERT(CHECK_TRUNCATE_TYPE_uint(rip_rel_pc - start_pc),
                      "decode_sizeof: unknown rip_rel instr type");
        *rip_rel_pos = (uint) (rip_rel_pc - start_pc);
    } else
        *rip_rel_pos = 0;
}
#endif
printf("Just Finished decode_sizeof and have returned %d\n", sz);
return sz;
}
#else
int
decode_sizeof(dcontext_t *dcontext, byte *start_pc, int *num_prefixes
              _IF_X64(uint *rip_rel_pos))
{
	// COMPLETEDD #374 decode_sizeof
	// Note this will have to change for Thumb instructions
	printf("Asking size of instruction to be 4 as all Arm Instructions\n");
	return 4;
}
#endif

/* Decodes the size of the instruction at address pc and points instr
 * at the raw bits for the instruction.
 * This corresponds to a Level 1 decoding.
 * Assumes that instr is already initialized, but uses the x86/x64 mode
 * for the current thread rather than that set in instr.
 * If caller is re-using same instr struct over multiple decodings,
 * should call instr_reset or instr_reuse.
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instr and sets opcode to OP_INVALID.
 */
byte *
decode_raw(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	  // COMPLETEDD #376 decode_raw
	  printf("Starting decode_raw\n");
    int sz = decode_sizeof(dcontext, pc, NULL _IF_X64(NULL));
    IF_X64(instr_set_x86_mode(instr, get_x86_mode(dcontext)));
    if (sz == 0) {
        /* invalid instruction! */
        instr_set_opcode(instr, OP_INVALID);
        return NULL;
    }
    instr_set_opcode(instr, OP_UNDECODED);
    instr_set_raw_bits(instr, pc, sz);
    /* assumption: operands are already marked invalid (instr was reset) */
    return (pc + sz);
}
