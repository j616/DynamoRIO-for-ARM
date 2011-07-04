/* file "decode.h" */

#ifndef DECODE_H
#define DECODE_H
DR_API
/**
 * Decodes the instruction at address \p pc into \p instr, filling in the
 * instruction's opcode, eflags usage, prefixes, and operands.
 * The instruction's raw bits are set to valid and pointed at \p pc
 * (xref instr_get_raw_bits()).
 * Assumes that \p instr is already initialized, but uses the x86/x64 mode
 * for the thread \p dcontext rather than that set in instr.
 * If caller is re-using same instr_t struct over multiple decodings,
 * caller should call instr_reset() or instr_reuse().
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instr and sets opcode to OP_INVALID.
 */
byte *
decode(dcontext_t *dcontext, byte *pc, instr_t *instr);

#ifndef ARM
typedef struct decode_info_t {
    /* Holds address and data size prefixes, as well as the prefixes
     * that are shared as-is with instr_t (PREFIX_SIGNIFICANT).
     * We assume we're in the default mode (32-bit or 64-bit,
     * depending on our build) and that the address and data size
     * prefixes can be treated as absolute.
     */
    uint prefixes;
    byte seg_override; /* REG enum of seg, REG_NULL if none */
    /* modrm info */
    byte modrm;
    byte mod;
    byte reg;
    byte rm;
    bool has_sib;
    byte scale;
    byte index;
    byte base;
    bool has_disp;
    int disp;
    /* immed info */
    opnd_size_t size_immed;
    opnd_size_t size_immed2;
    ptr_int_t immed;
    ptr_int_t immed2; /* this additional field could be 32-bit on all platforms */
    /* These two fields are only used when decoding rip-relative data refs */
    byte *start_pc;
    uint len;
    /* This field is only used when encoding rip-relative data refs.
     * To save space we could make it a union with disp.
     */
    byte *disp_abs;
#ifdef X64
    /* Since the mode when an instr_t is involved is per-instr rather than
     * per-dcontext we have our own field here instead of passing dcontext around.
     * It's up to the caller to set this field to match either the instr_t
     * or the dcontext_t field.
     */
    bool x86_mode;
#endif
    /* PR 302353: support decoding as though somewhere else */
    byte *orig_pc;
} decode_info_t;

enum {
    /* register enum values are used for TYPE_*REG */
    OPSZ_NA = REG_LAST_ENUM+1, /**< Sentinel value: not a valid size. */ /* = 139 */
    OPSZ_0,  /**< Intel 'm': "sizeless": used for both start addresses
              * (lea, invlpg) and implicit constants (rol, fldl2e, etc.) */
    OPSZ_1,  /**< Intel 'b': 1 byte */
    OPSZ_2,  /**< Intel 'w': 2 bytes */
    OPSZ_4,  /**< Intel 'd','si': 4 bytes */
    OPSZ_6,  /**< Intel 'p','s': 6 bytes */
    OPSZ_8,  /**< Intel 'q','pi': 8 bytes */
    OPSZ_10, /**< Intel 's' 64-bit, or double extended precision floating point
              * (latter used by fld, fstp, fbld, fbstp) */
    OPSZ_16, /**< Intel 'dq','ps','pd','ss','sd': 16 bytes */
    OPSZ_14, /**< FPU operating environment with short data size (fldenv, fnstenv) */
    OPSZ_28, /**< FPU operating environment with normal data size (fldenv, fnstenv) */
    OPSZ_94,  /**< FPU state with short data size (fnsave, frstor) */
    OPSZ_108, /**< FPU state with normal data size (fnsave, frstor) */
    OPSZ_512, /**< FPU, MMX, XMM state (fxsave, fxrstor) */
    /**
     * The following sizes (OPSZ_*_short*) vary according to the cs segment and the
     * operand size prefix.  This IR assumes that the cs segment is set to the
     * default operand size.  The operand size prefix then functions to shrink the
     * size.  The IR does not explicitly mark the prefix; rather, a shortened size is
     * requested in the operands themselves, with the IR adding the prefix at encode
     * time.  Normally the fixed sizes above should be used rather than these
     * variable sizes, which are used internally by the IR and should only be
     * externally specified when building an operand in order to be flexible and
     * allow other operands to decide the size for the instruction (the prefix
     * applies to the entire instruction).
     */
    OPSZ_2_short1, /**< Intel 'c': 2/1 bytes ("2/1" means 2 bytes normally, but if
                    * another operand requests a short size then this size can
                    * accommodate by shifting to its short size, which is 1 byte). */
    OPSZ_4_short2, /**< Intel 'z': 4/2 bytes */
    OPSZ_4_rex8_short2, /**< Intel 'v': 8/4/2 bytes */
    OPSZ_4_rex8,   /**< Intel 'd/q' (like 'v' but never 2 bytes). */
    OPSZ_6_irex10_short4, /**< Intel 'p': On Intel processors this is 10/6/4 bytes for
                           * segment selector + address.  On AMD processors this is
                           * 6/4 bytes for segment selector + address (rex is ignored). */
    OPSZ_8_short2, /**< partially resolved 4x8_short2 */
    OPSZ_8_short4, /**< Intel 'a': pair of 4_short2 (bound) */
    OPSZ_28_short14, /**< FPU operating env variable data size (fldenv, fnstenv) */
    OPSZ_108_short94, /**< FPU state with variable data size (fnsave, frstor) */
    /** Varies by 32-bit versus 64-bit processor mode. */
    OPSZ_4x8,  /**< Full register size with no variation by prefix.
                *   Used for control and debug register moves. */
    OPSZ_6x10, /**< Intel 's': 6-byte (10-byte for 64-bit mode) table base + limit */
    /**
     * Stack operands not only vary by operand size specifications but also
     * by 32-bit versus 64-bit processor mode.
     */
    OPSZ_4x8_short2, /**< Intel 'v'/'d64' for stack operations.
                      * Also 64-bit address-size specified operands, which are
                      * short4 rather than short2 in 64-bit mode (but short2 in
                      * 32-bit mode).
                      * Note that this IR does not distinguish multiple stack
                      * operations; dispatch by opcode must be used:
                      *   X2 = far call/far ret
                      *   X3 = int/iret
                      *   X8 = pusha/popa
                      *   X* = enter (dynamically varying amount)
                      * Note that stack operations may also modify the stack
                      * pointer prior to accessing the top of the stack, so
                      * for example "(esp)" may in fact be "4(esp)" depending
                      * on the opcode.
                      */
    OPSZ_4x8_short2xi8, /**< Intel 'f64': 4_short2 for 32-bit, 8_short2 for 64-bit AMD,
                         *   always 8 for 64-bit Intel */
    OPSZ_4_short2xi4,   /**< Intel 'f64': 4_short2 for 32-bit or 64-bit AMD,
                         *   always 4 for 64-bit Intel */
    /**
     * The following sizes differ based on whether the modrm chooses a
     * register or memory.
     */
    OPSZ_1_reg4,  /**< Intel Rd/Mb: zero-extends if reg; used by pextrb */
    OPSZ_2_reg4,  /**< Intel Rd/Mw: zero-extends if reg; used by pextrw */
    OPSZ_4_reg16, /**< Intel Udq/Md: sub-xmm but we consider that whole xmm;
                   *   used by insertps. */
    OPSZ_LAST,
};
#else
typedef struct {
    /* Holds address and data size prefixes, as well as the prefixes
     * that are shared as-is with instr_t (PREFIX_SIGNIFICANT).
     * We assume we're in the default mode (32-bit or 64-bit,
     * depending on our build) and that the address and data size
     * prefixes can be treated as absolute.
     */
    unsigned int regDst1;
    unsigned int regDst2;
    unsigned int regSrc1;
    unsigned int regSrc2;
    unsigned int regSrc3;
    /* immed info */
    opnd_size_t size_immed;
    short immed;
    /* These two fields are only used when decoding rip-relative data refs */
    unsigned char *start_pc;
    uint len;
} decode_info_t;

enum {
    /* register enum values are used for TYPE_*REG */
    OPSZ_NA, /**< Sentinel value: not a valid size. */ /* = 139 */
};

#endif
#ifndef ARM
/* for dcontext_t */
#define X64_MODE_DC(dc) IF_X64_ELSE(!get_x86_mode(dc), false)
#endif

#ifdef ARM
typedef struct  {
  int type;
  unsigned int concernedBits;
  unsigned int maskedBits;
  char *name;
  unsigned char dst1_type; unsigned int dst1_mask;
  unsigned char dst2_type; unsigned int dst2_mask;
  unsigned char src1_type; unsigned int src1_mask;
  unsigned char src2_type; unsigned int src2_mask;
  unsigned char src3_type; unsigned int src3_mask;
  unsigned int APSR;
} instr_info_t;

extern const instr_info_t DataProcessingInstructionsR[];
extern const instr_info_t MultipyAndMultiplyAcc[];
extern const instr_info_t DataProcessingInstructionsIMM[];
extern const instr_info_t BranchInstructions[];
extern const instr_info_t loadStore[];
extern const InstructionCatergory Catergories[];
extern const InstructionCatergory DataProcCats[];
extern const instr_info_t NullInstruction[];
extern const instr_info_t serviceCalls[];

DR_UNS_API
/**
 * Decodes the opcode and eflags usage of instruction at address \p pc
 * into \p instr.
 * The instruction's raw bits are set to valid and pointed at \p pc
 * (xref instr_get_raw_bits()).
 * Assumes that \p instr is already initialized, and uses the x86/x64 mode
 * set for it rather than the current thread's mode!
 * If caller is re-using same instr_t struct over multiple decodings,
 * caller should call instr_reset() or instr_reuse().
 * Returns the address of the next byte after the decoded instruction.
 * Returns NULL on decoding an invalid instr and sets opcode to OP_INVALID.
 */
byte *
decode_opcode(dcontext_t *dcontext, byte *pc, instr_t *instr);
#define UNKNOWN_APSR	0


enum {
	TYPE_NONE,
	Reg,
	Immediate,
};
#else
typedef struct instr_info_t {
    int type; /* an OP_ constant or special type code below */
    /* opcode: split into bytes
     * 0th (ms) = prefix byte, if byte 3's 1st nibble's bit 3 and bit 4 are both NOT set;
     *            modrm byte, if  byte 3's 1st nibble's bit 3 IS set.
     *            suffix byte, if  byte 3's 1st nibble's bit 4 IS set.
     * 1st = 1st byte of opcode
     * 2nd = 2nd byte of opcode (if there are 2)
     * 3rd (ls) = split into nibbles
     *   1st nibble (ms) = if bit 1 (OPCODE_TWOBYTES) set, opcode has 2 bytes
     *                     if bit 2 (OPCODE_REG) set, opcode has /n
     *                     if bit 3 (OPCODE_MODRM) set, opcode based on entire modrm
     *                       that modrm is stored as the byte 0
     *                     if bit 4 (OPCODE_SUFFIX) set, opcode based on suffix byte
     *                       that byte is stored as the byte 0
     *                     FIXME: so we do not support an instr that has an opcode
     *                     dependent on both a prefix and the entire modrm or suffix!
     *   2nd nibble (ls) = bits 1-3 hold /n for OPCODE_REG
     *                     if bit 4 (OPCODE_THREEBYTES) is set, the opcode has
     *                       3 bytes, with the first being an implied 0x0f (so
     *                       the 2nd byte is stored as "1st" and 3rd as "2nd").
     */
    uint opcode;
    const char *name;
    /* operands */
    byte dst1_type;  opnd_size_t dst1_size;
    byte dst2_type;  opnd_size_t dst2_size;
    byte src1_type;  opnd_size_t src1_size;
    byte src2_type;  opnd_size_t src2_size;
    byte src3_type;  opnd_size_t src3_size;
    byte flags; /* modrm and extra operand flags */
    uint eflags; /* combination of read & write flags from instr.h */
    ptr_int_t code; /* for PREFIX: one of the PREFIX_ constants, or SEG_ constant
                     * for EXTENSION and *_EXT: index into extensions table
                     * for OP_: pointer to next entry of that opcode
                     *   may also point to extra operand table
                     */
} instr_info_t;

enum {
    /* not a valid opcode */
    INVALID = OP_LAST + 1,
    /* prefix byte */
    PREFIX,
    /* 0x0f = two-byte escape code */
    ESCAPE,
    /* floating point instruction escape code */
    FLOAT_EXT,
    /* opcode extension via reg field of modrm */
    EXTENSION,
    /* 2-byte instructions differing by presence of 0xf3/0x66/0xf2 prefixes */
    PREFIX_EXT,
    /* (rep prefix +) 1-byte-opcode string instruction */
    REP_EXT,
    /* (repne prefix +) 1-byte-opcode string instruction */
    REPNE_EXT,
    /* 2-byte instructions differing by mod bits of modrm */
    MOD_EXT,
    /* 2-byte instructions differing by rm bits of modrm */
    RM_EXT,
    /* 2-byte instructions whose opcode also depends on a suffix byte */
    SUFFIX_EXT,
    /* instructions that vary based on whether in 64-bit mode or not */
    X64_EXT,
    /* 3-byte opcodes beginning 0x0f 0x38 (SSSE3 and SSE4) */
    ESCAPE_3BYTE_38,
    /* 3-byte opcodes beginning 0x0f 0x3a (SSE4) */
    ESCAPE_3BYTE_3a,
    /* instructions differing if a rex prefix is present */
    REX_EXT,
    /* else, from OP_ enum */
};

/* These are used only in the decoding tables.  We decode the
 * information into the operands.
 * For encoding these properties are specified in the operands,
 * with our encoder auto-adding the appropriate prefixes.
 */
#define PREFIX_DATA           0x0008
#define PREFIX_ADDR           0x0010
#define PREFIX_REX_W          0x0020
#define PREFIX_REX_R          0x0040
#define PREFIX_REX_X          0x0080
#define PREFIX_REX_B          0x0100
#define PREFIX_REX_GENERAL    0x0200 /* 0x40: only matters for SPL...SDL vs AH..BH */
#define PREFIX_REX_ALL        (PREFIX_REX_W|PREFIX_REX_R|PREFIX_REX_X|PREFIX_REX_B|\
                               PREFIX_REX_GENERAL)
#define PREFIX_SIZE_SPECIFIERS (PREFIX_DATA|PREFIX_ADDR|PREFIX_REX_ALL)

/* Unused except in decode tables (we encode the prefix into the opcodes) */
#define PREFIX_REP            0x0400
#define PREFIX_REPNE          0x0800

/* PREFIX_SEG_* is set by decode or decode_cti and is only a hint
 * to the caller.  Is ignored by encode in favor of the segment
 * reg specified in the applicable opnds.  We rely on it being set during
 * bb building.
 */
#define PREFIX_SEG_FS         0x1000
#define PREFIX_SEG_GS         0x2000

/* We encode some prefixes in the operands themselves, such that we shouldn't
 * consider the whole-instr_t flags when considering equality of Instrs
 */
#define PREFIX_SIGNIFICANT (PREFIX_LOCK|PREFIX_JCC_TAKEN|PREFIX_JCC_TAKEN)


#define PREFIX_LOCK           0x1 /**< Makes the instruction's memory accesses atomic. */
#define PREFIX_JCC_NOT_TAKEN  0x2 /**< Branch hint: conditional branch is taken. */
#define PREFIX_JCC_TAKEN      0x4 /**< Branch hint: conditional branch is not taken. */

/* branch hints show up as segment modifiers */
#define SEG_JCC_NOT_TAKEN     SEG_CS
#define SEG_JCC_TAKEN         SEG_DS
#define X64_MODE(di) IF_X64_ELSE(!(di)->x86_mode, false)

/* modrm/extra operands flags == single byte only! */
#define HAS_MODRM             0x01   /* else, no modrm */
#define HAS_EXTRA_OPERANDS    0x02   /* else, <= 2 dsts, <= 3 srcs */
/* if HAS_EXTRA_OPERANDS: */
#define EXTRAS_IN_CODE_FIELD  0x04   /* next instr_info_t pointed to by code field */
/* rather than split out into little tables of 32-bit vs OP_INVALID, we use a
 * flag to indicate opcodes that are invalid in particular modes:
 */
#define X86_INVALID           0x08
#define X64_INVALID           0x10
/* to avoid needing a single-valid-entry subtable in prefix_extensions */
#define REQUIRES_PREFIX       0x20
/* exported tables */
extern const instr_info_t first_byte[];
extern const instr_info_t second_byte[];
extern const instr_info_t extensions[][8];
extern const instr_info_t prefix_extensions[][4];
extern const instr_info_t mod_extensions[][2];
extern const instr_info_t rm_extensions[][8];
extern const instr_info_t x64_extensions[][2];
extern const instr_info_t rex_extensions[][2];
extern const byte third_byte_38_index[256];
extern const byte third_byte_3a_index[256];
extern const instr_info_t third_byte_38[];
extern const instr_info_t third_byte_3a[];
extern const instr_info_t rep_extensions[][4];
extern const instr_info_t repne_extensions[][6];
extern const instr_info_t float_low_modrm[];
extern const instr_info_t float_high_modrm[][64];
extern const byte suffix_index[256];
extern const instr_info_t suffix_extensions[];
extern const instr_info_t extra_operands[];
/* table that translates opcode enums into pointers into above tables */
extern const instr_info_t * const op_instr[];
/* for debugging: printing out types and sizes */
extern const char * const type_names[];
extern const char * const size_names[];
extern const instr_info_t invalid_instr;

/* operand types have 2 parts, type and size */
enum {
    /* operand types */
    TYPE_NONE,
    TYPE_A, /* immediate that is absolute address */
    TYPE_C, /* reg of modrm selects control reg */
    TYPE_D, /* reg of modrm selects debug reg */
    TYPE_E, /* modrm selects reg or mem addr */
    /* I don't use type F, I have eflags info in separate field */
    TYPE_G, /* reg of modrm selects register */
    TYPE_I, /* immediate */
    TYPE_J, /* immediate that is relative offset of EIP */
    TYPE_M, /* modrm select mem addr */
    TYPE_O, /* immediate that is memory offset */
    TYPE_P, /* reg of modrm selects MMX */
    TYPE_Q, /* modrm selects MMX or mem addr */
    TYPE_R, /* modrm selects register */
    TYPE_S, /* reg of modrm selects segment register */
    TYPE_V, /* reg of modrm selects XMM */
    TYPE_W, /* modrm selects XMM or mem addr */
    TYPE_X, /* DS:(RE)(E)SI */
    TYPE_Y, /* ES:(RE)(E)SDI */
    TYPE_P_MODRM,  /* == Intel 'N': modrm selects MMX */
    TYPE_V_MODRM,  /* == Intel 'U': modrm selects XMM */
    TYPE_1,
    TYPE_FLOATCONST,
    TYPE_XLAT,     /* DS:(RE)(E)BX+AL */
    TYPE_MASKMOVQ, /* DS:(RE)(E)DI */
    TYPE_FLOATMEM,
    TYPE_REG,     /* hardcoded register */
    TYPE_VAR_REG, /* hardcoded register, default 32 bits, but can be
                   * 16 w/ data prefix or 64 w/ rex.w: equivalent of Intel 'v'
                   * == like OPSZ_4_rex8_short2 */
    TYPE_VARZ_REG, /* hardcoded register, default 32 bits, but can be
                    * 16 w/ data prefix: equivalent of Intel 'z'
                    * == like OPSZ_4_short2 */
    TYPE_VAR_XREG, /* hardcoded register, default 32/64 bits depending on mode,
                    * but can be 16 w/ data prefix: equivalent of Intel 'd64'
                    * == like OPSZ_4x8_short2 */
    TYPE_VAR_ADDR_XREG, /* hardcoded register, default 32/64 bits depending on mode,
                         * but can be 16/32 w/ addr prefix: equivalent of Intel 'd64' */
    /* For x64 extensions (Intel '+r.') where rex.r can select an extended
     * register (r8-r15): we could try to add a flag that modifies the above
     * register types, but we'd have to stick it inside some stolen bits.  For
     * simplicity, we just make each combination a separate type:
     */
    TYPE_REG_EX,      /* like TYPE_REG but extendable. used for mov_imm 8-bit immed */
    TYPE_VAR_REG_EX,  /* like TYPE_VAR_REG (OPSZ_4_rex8_short2) but extendable.
                       * used for xchg and mov_imm 'v' immed. */
    TYPE_VAR_XREG_EX, /* like TYPE_VAR_XREG (OPSZ_4x8_short2) but extendable.
                       * used for pop and push. */
    TYPE_VAR_REGX_EX, /* hardcoded register, default 32 bits, but can be 64 w/ rex.w,
                       * and extendable.  used for bswap.
                       * == OPSZ_4_rex8 */
    TYPE_INDIR_E,
    TYPE_INDIR_REG,
    TYPE_INDIR_VAR_XREG, /* indirected register that varies (by addr prefix),
                          * with a base of 32/64 depending on the mode;
                          * indirected size varies with data prefix */
    TYPE_INDIR_VAR_REG, /* indirected register that varies (by addr prefix),
                         * with a base of 32/64;
                         * indirected size varies with data and rex prefixes */
    TYPE_INDIR_VAR_XIREG, /* indirected register that varies (by addr prefix),
                           * with a base of 32/64 depending on the mode;
                           * indirected size varies w/ data prefix, except 64-bit Intel */
};

#ifdef X64
# define OPSZ_PTR OPSZ_8       /**< Operand size for pointer values. */
# define OPSZ_STACK OPSZ_8     /**< Operand size for stack push/pop operand sizes. */
#else
# define OPSZ_PTR OPSZ_4       /**< Operand size for pointer values. */
# define OPSZ_STACK OPSZ_4     /**< Operand size for stack push/pop operand sizes. */
#endif
#define OPSZ_VARSTACK OPSZ_4x8_short2 /**< Operand size for prefix-varying stack
                                       * push/pop operand sizes. */
#define OPSZ_REXVARSTACK OPSZ_4_rex8_short2 /* Operand size for prefix/rex-varying
                                             * stack push/pop like operand sizes. */

#define OPSZ_ret OPSZ_4x8_short2xi8 /**< Operand size for ret instruction. */
#define OPSZ_call OPSZ_ret         /**< Operand size for push portion of call. */

/* Convenience defines for specific opcodes */
#define OPSZ_lea OPSZ_0              /**< Operand size for lea memory reference. */
#define OPSZ_invlpg OPSZ_0           /**< Operand size for invlpg memory reference. */
#define OPSZ_xlat OPSZ_1             /**< Operand size for xlat memory reference. */
#define OPSZ_clflush OPSZ_1          /**< Operand size for clflush memory reference. */
#define OPSZ_prefetch OPSZ_1         /**< Operand size for prefetch memory references. */
#define OPSZ_lgdt OPSZ_6x10          /**< Operand size for lgdt memory reference. */
#define OPSZ_sgdt OPSZ_6x10          /**< Operand size for sgdt memory reference. */
#define OPSZ_lidt OPSZ_6x10          /**< Operand size for lidt memory reference. */
#define OPSZ_sidt OPSZ_6x10          /**< Operand size for sidt memory reference. */
#define OPSZ_bound OPSZ_8_short4     /**< Operand size for bound memory reference. */
#define OPSZ_maskmovq OPSZ_8         /**< Operand size for maskmovq memory reference. */
#define OPSZ_maskmovdqu OPSZ_16      /**< Operand size for maskmovdqu memory reference. */
#define OPSZ_fldenv OPSZ_28_short14  /**< Operand size for fldenv memory reference. */
#define OPSZ_fnstenv OPSZ_28_short14 /**< Operand size for fnstenv memory reference. */
#define OPSZ_fnsave OPSZ_108_short94 /**< Operand size for fnsave memory reference. */
#define OPSZ_frstor OPSZ_108_short94 /**< Operand size for frstor memory reference. */
#define OPSZ_fxsave OPSZ_512         /**< Operand size for fxsave memory reference. */
#define OPSZ_fxrstor OPSZ_512        /**< Operand size for fxrstor memory reference. */

enum {
    /* OPSZ_ constants not exposed to the user */
    OPSZ_4_of_8 = OPSZ_LAST,  /* 32 bits, but can be half of MMX register */
    OPSZ_4_of_16, /* 32 bits, but can be part of XMM register */
    OPSZ_8_of_16, /* 64 bits, but can be half of XMM register */
    OPSZ_LAST_ENUM /* note last is NOT inclusive */
};
/* DR_API EXPORT END */
#endif
#endif /* DECODE_H */
