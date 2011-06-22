#ifndef _INSTR_H_
#define _INSTR_H_ 1

typedef byte opnd_size_t; /* contains a REG_ or OPSZ_ enum value */
/* we avoid typedef-ing the enum, as its storage size is compiler-specific */
typedef byte reg_id_t; /* contains a REG_ enum value */

/* x86 operand kinds */
enum {
    NULL_kind,
    IMMED_INTEGER_kind,
    IMMED_FLOAT_kind,
    PC_kind,
    REG_kind,
    BASE_DISP_kind, /* optional SEG_ reg + base reg + scaled index reg + disp */
    LAST_kind,      /* sentinal; not a valid opnd kind */
};

#ifdef ARM
#define CAT_UNCONDITIONAL 	1
#define CAT_DATAPROC				2
#define CAT_LOADSTORE1			3
#define CAT_LOADSTORE2			4
#define	CAT_BRANCH					5
#define CAT_SVC							6
#define CAT_UNDEFINED				7
#define CAT_NUMBER					6

#define DP_REGISTER					1
#define DP_REGISTERSR				2
#define	DP_MISC							3
#define DP_HWMMA						4
#define DP_MULTIPLYMA				5
#define	DP_SP								6
#define DP_EXTRALS					7
#define DP_EXTRALSUP				8
#define DP_IMM							9
#define DP_IMMLD					 	10
#define DP_IMMHILD					11
#define	DP_MSRIMM						12
#define DP_UNDEFINED				13
#define DP_NUMBER						14
#endif


#ifndef ARM
#define REG_LAST_ENUM     REG_INVALID /**< Last register enum value. */

enum {
#ifdef AVOID_API_EXPORT
    /* compiler gives weird errors for "REG_NONE" */
    /* PR 227381: genapi.pl auto-inserts doxygen comments for lines without any! */
#endif
    REG_NULL, /**< Sentinel value indicating no register, for address modes. */
    /* 64-bit general purpose */
    REG_RAX,  REG_RCX,  REG_RDX,  REG_RBX,  REG_RSP,  REG_RBP,  REG_RSI,  REG_RDI,
    REG_R8,   REG_R9,   REG_R10,  REG_R11,  REG_R12,  REG_R13,  REG_R14,  REG_R15,
    /* 32-bit general purpose */
    REG_EAX,  REG_ECX,  REG_EDX,  REG_EBX,  REG_ESP,  REG_EBP,  REG_ESI,  REG_EDI,
    REG_R8D,  REG_R9D,  REG_R10D, REG_R11D, REG_R12D, REG_R13D, REG_R14D, REG_R15D,
    /* 16-bit general purpose */
    REG_AX,   REG_CX,   REG_DX,   REG_BX,   REG_SP,   REG_BP,   REG_SI,   REG_DI,
    REG_R8W,  REG_R9W,  REG_R10W, REG_R11W, REG_R12W, REG_R13W, REG_R14W, REG_R15W,
    /* 8-bit general purpose */
    REG_AL,   REG_CL,   REG_DL,   REG_BL,   REG_AH,   REG_CH,   REG_DH,   REG_BH,
    REG_R8L,  REG_R9L,  REG_R10L, REG_R11L, REG_R12L, REG_R13L, REG_R14L, REG_R15L,
    REG_SPL,  REG_BPL,  REG_SIL,  REG_DIL,
    /* 64-BIT MMX */
    REG_MM0,  REG_MM1,  REG_MM2,  REG_MM3,  REG_MM4,  REG_MM5,  REG_MM6,  REG_MM7,
    /* 128-BIT XMM */
    REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM4, REG_XMM5, REG_XMM6, REG_XMM7,
    REG_XMM8, REG_XMM9, REG_XMM10,REG_XMM11,REG_XMM12,REG_XMM13,REG_XMM14,REG_XMM15,
    /* floating point registers */
    REG_ST0,  REG_ST1,  REG_ST2,  REG_ST3,  REG_ST4,  REG_ST5,  REG_ST6,  REG_ST7,
    /* segments (order from "Sreg" description in Intel manual) */
    SEG_ES,   SEG_CS,   SEG_SS,   SEG_DS,   SEG_FS,   SEG_GS,
    /* debug & control registers (privileged access only; 8-15 for future processors) */
    REG_DR0,  REG_DR1,  REG_DR2,  REG_DR3,  REG_DR4,  REG_DR5,  REG_DR6,  REG_DR7,
    REG_DR8,  REG_DR9,  REG_DR10, REG_DR11, REG_DR12, REG_DR13, REG_DR14, REG_DR15,
    /* cr9-cr15 do not yet exist on current x64 hardware */
    REG_CR0,  REG_CR1,  REG_CR2,  REG_CR3,  REG_CR4,  REG_CR5,  REG_CR6,  REG_CR7,
    REG_CR8,  REG_CR9,  REG_CR10, REG_CR11, REG_CR12, REG_CR13, REG_CR14, REG_CR15,
    REG_INVALID, /**< Sentinel value indicating an invalid register. */
};
enum { /* FIXME: vs RAW_OPCODE_* enum */
    FS_SEG_OPCODE        = 0x64,
    GS_SEG_OPCODE        = 0x65,

    /* For Windows, we piggyback on native TLS via gs for x64 and fs for x86.
     * For Linux, we steal a segment register, and so use fs for x86 (where
     * pthreads uses gs) and gs for x64 (where pthreads uses fs) (presumably
     * to avoid conflicts w/ wine).
     */
#ifdef X64
    TLS_SEG_OPCODE       = GS_SEG_OPCODE,
#else
    TLS_SEG_OPCODE       = FS_SEG_OPCODE,
#endif

    DATA_PREFIX_OPCODE   = 0x66,
    ADDR_PREFIX_OPCODE   = 0x67,
    REPNE_PREFIX_OPCODE  = 0xf2,
    REP_PREFIX_OPCODE    = 0xf3,
    REX_PREFIX_BASE_OPCODE = 0x40,
    REX_PREFIX_W_OPFLAG    = 0x8,
    REX_PREFIX_R_OPFLAG    = 0x4,
    REX_PREFIX_X_OPFLAG    = 0x2,
    REX_PREFIX_B_OPFLAG    = 0x1,
    REX_PREFIX_ALL_OPFLAGS = 0xf,
    MOV_REG2MEM_OPCODE   = 0x89,
    MOV_MEM2REG_OPCODE   = 0x8b,
    MOV_XAX2MEM_OPCODE   = 0xa3, /* no ModRm */
    MOV_MEM2XAX_OPCODE   = 0xa1, /* no ModRm */
    MOV_IMM2XAX_OPCODE   = 0xb8, /* no ModRm */
    MOV_IMM2XBX_OPCODE   = 0xbb, /* no ModRm */
    MOV_IMM2MEM_OPCODE   = 0xc7, /* has ModRm */
    JECXZ_OPCODE         = 0xe3,
    JMP_SHORT_OPCODE     = 0xeb,
    JMP_OPCODE           = 0xe9,
    JNE_OPCODE_1         = 0x0f,
    SAHF_OPCODE          = 0x9e,
    LAHF_OPCODE          = 0x9f,
    SETO_OPCODE_1        = 0x0f,
    SETO_OPCODE_2        = 0x90,
    ADD_AL_OPCODE        = 0x04,
    INC_MEM32_OPCODE_1   = 0xff, /* has /0 as well */
    MODRM16_DISP16       = 0x06, /* see vol.2 Table 2-1 for modR/M */
    SIB_DISP32           = 0x25, /* see vol.2 Table 2-1 for modR/M */
};
#endif

#ifndef ARM
/* length of our mangling of jecxz/loop* */
#define CTI_SHORT_REWRITE_LENGTH 9
#else
#define CTI_SHORT_REWRITE_LENGTH 4
#endif

/* typedef is in globals.h */
struct _opnd_t {
    byte kind;
    /* size field only used for immed_ints and addresses
     * it holds a OPSZ_ field from decode.h
     * we need it so we can pick the proper instruction form for
     * encoding -- an alternative would be to split all the opcodes
     * up into different data size versions.
     */
    opnd_size_t size;
    /* To avoid increasing our union beyond 64 bits, we store additional data
     * needed for x64 operand types here in the alignment padding.
     */
    union {
        /* all are 64 bits or less */
        /* NULL_kind has no value */
        ptr_int_t immed_int;   /* IMMED_INTEGER_kind */
        float immed_float;     /* IMMED_FLOAT_kind */
        /* PR 225937: today we provide no way of specifying a 16-bit immediate
         * (encoded as a data16 prefix, which also implies a 16-bit EIP,
         * making it only useful for far pcs)
         */
        app_pc pc;             /* PC_kind and FAR_PC_kind */
        /* For FAR_PC_kind and FAR_INSTR_kind, we use pc/instr, and keep the
         * segment selector (which is NOT a SEG_constant) in far_pc_seg_selector
         * above, to save space.
         */
        instr_t *instr;         /* INSTR_kind and FAR_INSTR_kind */
        reg_id_t reg;           /* REG_kind */

        void *addr;             /* REL_ADDR_kind and ABS_ADDR_kind */
    } value;
};

byte *
instr_get_raw_bits(instr_t *instr);

/* even on x64, displacements are 32 bits, so we keep the "int" type and 4-byte size */
#define PC_RELATIVE_TARGET(addr) 0

/**
 * Returns true iff \p instr is an unconditional direct branch: OP_jmp,
 * OP_jmp_short, or OP_jmp_far.
 */
bool
instr_is_ubr(instr_t *instr);

DR_API
/**
 * Returns true iff \p instr is a conditional branch: OP_jcc, OP_jcc_short,
 * OP_loop*, or OP_jecxz.
 */
bool
instr_is_cbr(instr_t *instr);

/* Sets whether instr came from our mangling. */
void
instr_set_our_mangling(instr_t *instr, bool ours);

/* FIXME: could shrink prefixes, eflags, opcode, and flags fields
 * this struct isn't a memory bottleneck though b/c it isn't persistent
 */
struct _instr_t {
    /* flags contains the constants defined above */
    uint    flags;

    /* raw bits of length length are pointed to by the bytes field */
    byte    *bytes;
    uint    length;

    uint    opcode;
    /* we dynamically allocate dst and src arrays b/c x86 instrs can have
     * up to 8 of each of them, but most have <=2 dsts and <=3 srcs, and we
     * use this struct for un-decoded instrs too
     */

    /* translation target for this instr */
    app_pc  translation;

    byte    num_dsts;
    byte    num_srcs;

    opnd_t    *dsts;
    /* for efficiency everyone has a 1st src opnd, since we often just
     * decode jumps, which all have a single source (==target)
     * yes this is an extra 10 bytes, but the whole struct is still < 64 bytes!
     */
    opnd_t    src0;
    opnd_t    *srcs; /* this array has 2nd src and beyond */

    /* fields for building instructions into instruction lists */
    instr_t   *prev;
    instr_t   *next;
};

DR_UNS_API
/**
 * If the translation pointer is set for \p instr, returns that
 * else returns NULL.
 * \note The translation pointer is not automatically set when
 * decoding instructions from raw bytes (via decode(), e.g.); it is
 * set for instructions in instruction lists generated by DR (see
 * dr_register_bb_event()).
 *
 */
app_pc
instr_get_translation(instr_t *instr);

DR_API
/**
 * Sets the translation pointer for \p instr, used to recreate the
 * application address corresponding to this instruction.  When adding
 * or modifying instructions that are to be considered application
 * instructions (i.e., non meta-instructions: see
 * #instr_ok_to_mangle), the translation should always be set.  Pick
 * the application address that if executed will be equivalent to
 * restarting \p instr.
 * Returns the supplied \p instr (for easy chaining).  Use
 * #instr_get_app_pc to see the current value of the translation.
 */
instr_t *
instr_set_translation(instr_t *instr, app_pc addr);

DR_API
/**
 * Returns a copy of \p orig with separately allocated memory for
 * operands and raw bytes if they were present in \p orig.
 */
instr_t *
instr_clone(dcontext_t *dcontext, instr_t *orig);

DR_API
/**
 * Returns \p instr's source operand at position \p pos (0-based).
 */
opnd_t
instr_get_src(instr_t *instr, uint pos);

DR_API
/** Returns true iff \p opnd is a (near or far) instr_t pointer address operand. */
bool
opnd_is_instr(opnd_t opnd);

DR_API
/** Assumes \p opnd is an instr_t pointer, returns its value. */
instr_t*
opnd_get_instr(opnd_t opnd);

DR_API
/** Returns true iff \p opnd is a far instr_t pointer address operand. */
bool
opnd_is_far_instr(opnd_t opnd);

DR_API
/**
 * Sets \p instr's source operand at position \p pos to be \p opnd.
 * Also calls instr_set_raw_bits_valid(\p instr, false) and
 * instr_set_operands_valid(\p instr, true).
 */
void
instr_set_src(instr_t *instr, uint pos, opnd_t opnd);


DR_API
/**
 * Assumes \p opnd is a far program address.
 * Returns \p opnd's segment, a segment selector (not a SEG_ constant).
 */
ushort
opnd_get_segment_selector(opnd_t opnd);

DR_API
/**
 * Returns a far instr_t pointer address with value \p seg_selector:instr.
 * \p seg_selector is a segment selector, not a SEG_ constant.
 */
opnd_t
opnd_create_far_instr(ushort seg_selector, instr_t *instr);

DR_API
/** Returns an instr_t pointer address with value \p instr. */
opnd_t
opnd_create_instr(instr_t *instr);

DR_API
/**
 * Assumes that \p cti_instr is a control transfer instruction
 * Returns the first source operand of \p cti_instr (its target).
 */
opnd_t
instr_get_target(instr_t *cti_instr);

DR_API
/** Assumes \p opnd is a (near or far) program address, returns its value. */
app_pc
opnd_get_pc(opnd_t opnd);

DR_API
/**
 * Calculates the size, in bytes, of the memory read or write of
 * the instr at \p pc.  If the instruction is a repeating string instruction,
 * considers only one iteration.
 * Returns the pc of the following instruction.
 * If the instruction at \p pc does not reference memory, or is invalid,
 * returns NULL.
 */
app_pc
decode_memory_reference_size(dcontext_t *dcontext, app_pc pc, uint *size_in_bytes);

DR_API
/**
 * Returns true iff \p instr's opcode is valid.
 * If the opcode is ever set to other than OP_INVALID or OP_UNDECODED it is assumed
 * to be valid.  However, calling instr_get_opcode() will attempt to
 * decode a valid opcode, hence the purpose of this routine.
 */
bool
instr_opcode_valid(instr_t *instr);

DR_API
/**
 * Returns true iff \p instr is a control transfer instruction of any kind
 * This includes OP_jcc, OP_jcc_short, OP_loop*, OP_jecxz, OP_call*, and OP_jmp*.
 */
bool
instr_is_cti(instr_t *instr);

bool
instr_is_cti_short_rewrite(instr_t *instr, byte *pc);

DR_API
/**
 * Convenience routine that returns an initialized instr_t allocated on the
 * thread-local heap with opcode \p opcode and no sources or destinations.
 */
instr_t *
instr_create_0dst_0src(dcontext_t *dcontext, int opcode);

byte *
remangle_short_rewrite(dcontext_t *dcontext, instr_t *instr, byte *pc, app_pc target);

#ifdef ARM
// NEED TO GET RID
enum
{
	SEG_CS,
	SEG_DS,
	SEG_ES,
	SEG_FS,
	SEG_GS,
	SEG_SS
};
#endif

/* This should be kept in sync w/ the defines in x86/x86.asm */
enum {
    REGPARM_END_ALIGN,
    REDZONE_SIZE
};

DR_API
/** Returns \p instr's opcode (an OP_ constant). */
int
instr_get_opcode(instr_t *instr);

DR_API
/**
 * Performs address calculation in the same manner as
 * instr_compute_address() but handles multiple memory operands.  The
 * \p index parameter should be initially set to 0 and then
 * incremented with each successive call until this routine returns
 * false, which indicates that there are no more memory operands.  The
 * address of each is computed in the same manner as
 * instr_compute_address() and returned in \p addr; whether it is a
 * write is returned in \p is_write.  Either or both OUT variables can
 * be NULL.
 */
bool
instr_compute_address_ex(instr_t *instr, dr_mcontext_t *mc, uint index,
                         OUT app_pc *addr, OUT bool *write);

enum {
    EFLAGS_CF = 0x00000001, /**< The bit in the eflags register of CF (Carry Flag). */
    EFLAGS_PF = 0x00000004, /**< The bit in the eflags register of PF (Parity Flag). */
    EFLAGS_AF = 0x00000010, /**< The bit in the eflags register of AF (Aux Carry Flag). */
    EFLAGS_ZF = 0x00000040, /**< The bit in the eflags register of ZF (Zero Flag). */
    EFLAGS_SF = 0x00000080, /**< The bit in the eflags register of SF (Sign Flag). */
    EFLAGS_DF = 0x00000400, /**< The bit in the eflags register of DF (Direction Flag). */
    EFLAGS_OF = 0x00000800, /**< The bit in the eflags register of OF (Overflow Flag). */
};

DR_API
/** Returns true iff \p instr is an "undefined" instruction (ud2) */
bool
instr_is_undefined(instr_t *instr);

DR_API
/**
 * Returns true iff \p instr's opcode is NOT OP_INVALID.
 * Not to be confused with an invalid opcode, which can be OP_INVALID or
 * OP_UNDECODED.  OP_INVALID means an instruction with no valid fields:
 * raw bits (may exist but do not correspond to a valid instr), opcode,
 * eflags, or operands.  It could be an uninitialized
 * instruction or the result of decoding an invalid sequence of bytes.
 */
bool
instr_valid(instr_t *instr);

DR_API
/**
 * Returns true iff \p instr is used to implement system calls: OP_int with a
 * source operand of 0x80 on linux or 0x2e on windows, or OP_sysenter,
 * or OP_syscall, or #instr_is_wow64_syscall() for WOW64.
 */
bool
instr_is_syscall(instr_t *instr);


/* flags */
enum {

    /* PR 267260: distinguish our own mangling from client-added instrs */
    INSTR_RAW_BITS_VALID        = 0x00080000,
    INSTR_OPERANDS_VALID        = 0x00010000,
#ifndef ARM
    INSTR_EFLAGS_VALID          = 0x00020000,
#else
    INSTR_APSR_VALID        		= 0x00020000,
#endif
    INSTR_RAW_BITS_ALLOCATED    = 0x00100000,
    INSTR_DO_NOT_MANGLE         = 0x00200000,
    INSTR_OUR_MANGLING          = 0x80000000,
};

#ifdef ARM
/** Opcode constants for use in the instr_t data structure. */
enum {
/*   0 */     OP_INVALID,  /* NULL, */ /**< Indicates an invalid instr_t. */
/*   1 */     OP_UNDECODED,/* NULL, */ /**< Indicates an undecoded instr_t. */
/*   2 */     OP_CONTD,    /* NULL, */ /**< Used internally only. */
/*   3 */     OP_LABEL,    /* NULL, */ /**< A label is used for instr_t branch targets. */
/* 	 4 */			OP_AND,
/* 5 */ OP_EOR,
/* 6 */ OP_SUB,
/* 7 */ OP_RSB,
/* 8 */ OP_ADD,
/* 9 */ OP_ADC,
/* 10 */ OP_SBC,
/* 11 */ OP_RSC,
/* 12 */ OP_TST,
/* 13 */ OP_TEQ,
/* 14 */ OP_CMP,
/* 15 */ OP_CMN,
/* 16 */ OP_ORR,
/* 17 */ OP_MOV,
/* 18 */ OP_LSL,
/* 19 */ OP_LSR,
/* 20 */ OP_ASR,
/* 21 */ OP_RRX,
/* 22 */ OP_ROR,
/* 23 */ OP_BIC,
/* 24 */ OP_MVN,
/* 25 */ OP_ADR,
/* 26 */ OP_MUL,
/* 27 */ OP_MLA,
/* 28 */ OP_UMAAL,
/* 29 */ OP_MLS,
/* 30 */ OP_UMULL,
/* 31 */ OP_UMLAL,
/* 32 */ OP_SMULL,
/* 33 */ OP_SMLAL,
/* 34 */ OP_B,
/* 35 */ OP_BL,
/* 36 */ OP_STRT,
/* 37 */ OP_STRB,
/* 38 */ OP_STRBT,
/* 39 */ OP_LDRT,
/* 40 */ OP_LDRB,
/* 41 */ OP_LDRBT,
/* 42 */ OP_LDR,
/* 43 */ OP_STR,
/* 44 */ OP_NULL,
				OP_FIRST = OP_AND, /**< First real opcode. */
				OP_LAST = OP_STR,   /**< Last real opcode. */
};

typedef struct
{
  int catergoryNumber;
  unsigned int concernedBits;
    unsigned int maskedBits;
} InstructionCatergory;
#else

#define REG_START_64      REG_RAX   /**< Start of 64-bit general register enum values. */
#define REG_STOP_64       REG_R15   /**< End of 64-bit general register enum values. */
#define REG_START_32      REG_EAX   /**< Start of 32-bit general register enum values. */
#define REG_STOP_32       REG_R15D  /**< End of 32-bit general register enum values. */
#define REG_START_16      REG_AX    /**< Start of 16-bit general register enum values. */
#define REG_STOP_16       REG_R15W  /**< End of 16-bit general register enum values. */
#define REG_START_8       REG_AL    /**< Start of 8-bit general register enum values. */
#define REG_STOP_8        REG_DIL   /**< End of 8-bit general register enum values. */
#define REG_START_8HL     REG_AL    /**< Start of 8-bit high-low register enum values. */
#define REG_STOP_8HL      REG_BH    /**< End of 8-bit high-low register enum values. */
#define REG_START_x86_8   REG_AH    /**< Start of 8-bit x86-only register enum values. */
#define REG_STOP_x86_8    REG_BH    /**< Stop of 8-bit x86-only register enum values. */
#define REG_START_x64_8   REG_SPL   /**< Start of 8-bit x64-only register enum values. */
#define REG_STOP_x64_8    REG_DIL   /**< Stop of 8-bit x64-only register enum values. */
#define REG_START_MMX     REG_MM0   /**< Start of mmx register enum values. */
#define REG_STOP_MMX      REG_MM7   /**< End of mmx register enum values. */
#define REG_START_XMM     REG_XMM0  /**< Start of xmm register enum values. */
#define REG_STOP_XMM      REG_XMM15 /**< End of xmm register enum values. */
#define REG_START_FLOAT   REG_ST0   /**< Start of floating-point-register enum values. */
#define REG_STOP_FLOAT    REG_ST7   /**< End of floating-point-register enum values. */
#define REG_START_SEGMENT SEG_ES    /**< Start of segment register enum values. */
#define REG_STOP_SEGMENT  SEG_GS    /**< End of segment register enum values. */
#define REG_START_DR      REG_DR0   /**< Start of debug register enum values. */
#define REG_STOP_DR       REG_DR15  /**< End of debug register enum values. */
#define REG_START_CR      REG_CR0   /**< Start of control register enum values. */
#define REG_STOP_CR       REG_CR15  /**< End of control register enum values. */
#define REG_LAST_VALID_ENUM REG_CR15  /**< Last valid register enum value. */
#define REG_LAST_ENUM     REG_INVALID /**< Last register enum value. */

#define EFLAGS_READ_CF   0x00000001 /**< Reads CF (Carry Flag). */
#define EFLAGS_READ_PF   0x00000002 /**< Reads PF (Parity Flag). */
#define EFLAGS_READ_AF   0x00000004 /**< Reads AF (Auxiliary Carry Flag). */
#define EFLAGS_READ_ZF   0x00000008 /**< Reads ZF (Zero Flag). */
#define EFLAGS_READ_SF   0x00000010 /**< Reads SF (Sign Flag). */
#define EFLAGS_READ_TF   0x00000020 /**< Reads TF (Trap Flag). */
#define EFLAGS_READ_IF   0x00000040 /**< Reads IF (Interrupt Enable Flag). */
#define EFLAGS_READ_DF   0x00000080 /**< Reads DF (Direction Flag). */
#define EFLAGS_READ_OF   0x00000100 /**< Reads OF (Overflow Flag). */
#define EFLAGS_READ_NT   0x00000200 /**< Reads NT (Nested Task). */
#define EFLAGS_READ_RF   0x00000400 /**< Reads RF (Resume Flag). */
#define EFLAGS_WRITE_CF  0x00000800 /**< Writes CF (Carry Flag). */
#define EFLAGS_WRITE_PF  0x00001000 /**< Writes PF (Parity Flag). */
#define EFLAGS_WRITE_AF  0x00002000 /**< Writes AF (Auxiliary Carry Flag). */
#define EFLAGS_WRITE_ZF  0x00004000 /**< Writes ZF (Zero Flag). */
#define EFLAGS_WRITE_SF  0x00008000 /**< Writes SF (Sign Flag). */
#define EFLAGS_WRITE_TF  0x00010000 /**< Writes TF (Trap Flag). */
#define EFLAGS_WRITE_IF  0x00020000 /**< Writes IF (Interrupt Enable Flag). */
#define EFLAGS_WRITE_DF  0x00040000 /**< Writes DF (Direction Flag). */
#define EFLAGS_WRITE_OF  0x00080000 /**< Writes OF (Overflow Flag). */
#define EFLAGS_WRITE_NT  0x00100000 /**< Writes NT (Nested Task). */
#define EFLAGS_WRITE_RF  0x00200000 /**< Writes RF (Resume Flag). */

# define REG_XAX REG_EAX  /**< Platform-independent way to refer to rax/eax. */
# define REG_XCX REG_ECX  /**< Platform-independent way to refer to rcx/ecx. */
# define REG_XDX REG_EDX  /**< Platform-independent way to refer to rdx/edx. */
# define REG_XBX REG_EBX  /**< Platform-independent way to refer to rbx/ebx. */
# define REG_XSP REG_ESP  /**< Platform-independent way to refer to rsp/esp. */
# define REG_XBP REG_EBP  /**< Platform-independent way to refer to rbp/ebp. */
# define REG_XSI REG_ESI  /**< Platform-independent way to refer to rsi/esi. */
# define REG_XDI REG_EDI  /**< Platform-independent way to refer to rdi/edi. */

#define EFLAGS_READ_ALL  0x000007ff /**< Reads all flags. */
#define EFLAGS_WRITE_ALL 0x003ff800 /**< Writes all flags. */
/* 6 most common flags ("arithmetic flags"): CF, PF, AF, ZF, SF, OF */
/** Reads all 6 arithmetic flags (CF, PF, AF, ZF, SF, OF). */
#define EFLAGS_READ_6    0x0000011f
/** Writes all 6 arithmetic flags (CF, PF, AF, ZF, SF, OF). */
#define EFLAGS_WRITE_6   0x0008f800

/** Converts an EFLAGS_WRITE_* value to the corresponding EFLAGS_READ_* value. */
#define EFLAGS_WRITE_TO_READ(x) ((x) >> 11)
/** Converts an EFLAGS_READ_* value to the corresponding EFLAGS_WRITE_* value. */
#define EFLAGS_READ_TO_WRITE(x) ((x) << 11)
/** Opcode constants for use in the instr_t data structure. */
enum {
/*   0 */     OP_INVALID,  /* NULL, */ /**< Indicates an invalid instr_t. */
/*   1 */     OP_UNDECODED,/* NULL, */ /**< Indicates an undecoded instr_t. */
/*   2 */     OP_CONTD,    /* NULL, */ /**< Used internally only. */
/*   3 */     OP_LABEL,    /* NULL, */ /**< A label is used for instr_t branch targets. */

/*   4 */     OP_add,      /* &first_byte[0x05], */ /**< add opcode */
/*   5 */     OP_or,       /* &first_byte[0x0d], */ /**< or opcode */
/*   6 */     OP_adc,      /* &first_byte[0x15], */ /**< adc opcode */
/*   7 */     OP_sbb,      /* &first_byte[0x1d], */ /**< sbb opcode */
/*   8 */     OP_and,      /* &first_byte[0x25], */ /**< and opcode */
/*   9 */     OP_daa,      /* &first_byte[0x27], */ /**< daa opcode */
/*  10 */     OP_sub,      /* &first_byte[0x2d], */ /**< sub opcode */
/*  11 */     OP_das,      /* &first_byte[0x2f], */ /**< das opcode */
/*  12 */     OP_xor,      /* &first_byte[0x35], */ /**< xor opcode */
/*  13 */     OP_aaa,      /* &first_byte[0x37], */ /**< aaa opcode */
/*  14 */     OP_cmp,      /* &first_byte[0x3d], */ /**< cmp opcode */
/*  15 */     OP_aas,      /* &first_byte[0x3f], */ /**< aas opcode */
/*  16 */     OP_inc,      /* &x64_extensions[0][0], */ /**< inc opcode */
/*  17 */     OP_dec,      /* &x64_extensions[8][0], */ /**< dec opcode */
/*  18 */     OP_push,     /* &first_byte[0x50], */ /**< push opcode */
/*  19 */     OP_push_imm, /* &first_byte[0x68], */ /**< push_imm opcode */
/*  20 */     OP_pop,      /* &first_byte[0x58], */ /**< pop opcode */
/*  21 */     OP_pusha,    /* &first_byte[0x60], */ /**< pusha opcode */
/*  22 */     OP_popa,     /* &first_byte[0x61], */ /**< popa opcode */
/*  23 */     OP_bound,    /* &first_byte[0x62], */ /**< bound opcode */
/*  24 */     OP_arpl,     /* &x64_extensions[16][0], */ /**< arpl opcode */
/*  25 */     OP_imul,     /* &extensions[10][5], */ /**< imul opcode */

/*  26 */     OP_jo_short,     /* &first_byte[0x70], */ /**< jo_short opcode */
/*  27 */     OP_jno_short,    /* &first_byte[0x71], */ /**< jno_short opcode */
/*  28 */     OP_jb_short,     /* &first_byte[0x72], */ /**< jb_short opcode */
/*  29 */     OP_jnb_short,    /* &first_byte[0x73], */ /**< jnb_short opcode */
/*  30 */     OP_jz_short,     /* &first_byte[0x74], */ /**< jz_short opcode */
/*  31 */     OP_jnz_short,    /* &first_byte[0x75], */ /**< jnz_short opcode */
/*  32 */     OP_jbe_short,    /* &first_byte[0x76], */ /**< jbe_short opcode */
/*  33 */     OP_jnbe_short,   /* &first_byte[0x77], */ /**< jnbe_short opcode */
/*  34 */     OP_js_short,     /* &first_byte[0x78], */ /**< js_short opcode */
/*  35 */     OP_jns_short,    /* &first_byte[0x79], */ /**< jns_short opcode */
/*  36 */     OP_jp_short,     /* &first_byte[0x7a], */ /**< jp_short opcode */
/*  37 */     OP_jnp_short,    /* &first_byte[0x7b], */ /**< jnp_short opcode */
/*  38 */     OP_jl_short,     /* &first_byte[0x7c], */ /**< jl_short opcode */
/*  39 */     OP_jnl_short,    /* &first_byte[0x7d], */ /**< jnl_short opcode */
/*  40 */     OP_jle_short,    /* &first_byte[0x7e], */ /**< jle_short opcode */
/*  41 */     OP_jnle_short,   /* &first_byte[0x7f], */ /**< jnle_short opcode */

/*  42 */     OP_call,           /* &first_byte[0xe8], */ /**< call opcode */
/*  43 */     OP_call_ind,       /* &extensions[12][2], */ /**< call_ind opcode */
/*  44 */     OP_call_far,       /* &first_byte[0x9a], */ /**< call_far opcode */
/*  45 */     OP_call_far_ind,   /* &extensions[12][3], */ /**< call_far_ind opcode */
/*  46 */     OP_jmp,            /* &first_byte[0xe9], */ /**< jmp opcode */
/*  47 */     OP_jmp_short,      /* &first_byte[0xeb], */ /**< jmp_short opcode */
/*  48 */     OP_jmp_ind,        /* &extensions[12][4], */ /**< jmp_ind opcode */
/*  49 */     OP_jmp_far,        /* &first_byte[0xea], */ /**< jmp_far opcode */
/*  50 */     OP_jmp_far_ind,    /* &extensions[12][5], */ /**< jmp_far_ind opcode */

/*  51 */     OP_loopne,   /* &first_byte[0xe0], */ /**< loopne opcode */
/*  52 */     OP_loope,    /* &first_byte[0xe1], */ /**< loope opcode */
/*  53 */     OP_loop,     /* &first_byte[0xe2], */ /**< loop opcode */
/*  54 */     OP_jecxz,    /* &first_byte[0xe3], */ /**< jecxz opcode */

    /* point ld & st at eAX & al instrs, they save 1 byte (no modrm),
     * hopefully time taken considering them doesn't offset that */
/*  55 */     OP_mov_ld,      /* &first_byte[0xa1], */ /**< mov_ld opcode */
/*  56 */     OP_mov_st,      /* &first_byte[0xa3], */ /**< mov_st opcode */
    /* note that store of immed is mov_st not mov_imm, even though can be immed->reg,
     * which we address by sharing part of the mov_st template chain */
/*  57 */     OP_mov_imm,     /* &first_byte[0xb8], */ /**< mov_imm opcode */
/*  58 */     OP_mov_seg,     /* &first_byte[0x8e], */ /**< mov_seg opcode */
/*  59 */     OP_mov_priv,    /* &second_byte[0x20], */ /**< mov_priv opcode */

/*  60 */     OP_test,     /* &first_byte[0xa9], */ /**< test opcode */
/*  61 */     OP_lea,      /* &first_byte[0x8d], */ /**< lea opcode */
/*  62 */     OP_xchg,     /* &first_byte[0x91], */ /**< xchg opcode */
/*  63 */     OP_cwde,     /* &first_byte[0x98], */ /**< cwde opcode */
/*  64 */     OP_cdq,      /* &first_byte[0x99], */ /**< cdq opcode */
/*  65 */     OP_fwait,    /* &first_byte[0x9b], */ /**< fwait opcode */
/*  66 */     OP_pushf,    /* &first_byte[0x9c], */ /**< pushf opcode */
/*  67 */     OP_popf,     /* &first_byte[0x9d], */ /**< popf opcode */
/*  68 */     OP_sahf,     /* &first_byte[0x9e], */ /**< sahf opcode */
/*  69 */     OP_lahf,     /* &first_byte[0x9f], */ /**< lahf opcode */

/*  70 */     OP_ret,       /* &first_byte[0xc2], */ /**< ret opcode */
/*  71 */     OP_ret_far,   /* &first_byte[0xca], */ /**< ret_far opcode */

/*  72 */     OP_les,      /* &first_byte[0xc4], */ /**< les opcode */
/*  73 */     OP_lds,      /* &first_byte[0xc5], */ /**< lds opcode */
/*  74 */     OP_enter,    /* &first_byte[0xc8], */ /**< enter opcode */
/*  75 */     OP_leave,    /* &first_byte[0xc9], */ /**< leave opcode */
/*  76 */     OP_int3,     /* &first_byte[0xcc], */ /**< int3 opcode */
/*  77 */     OP_int,      /* &first_byte[0xcd], */ /**< int opcode */
/*  78 */     OP_into,     /* &first_byte[0xce], */ /**< into opcode */
/*  79 */     OP_iret,     /* &first_byte[0xcf], */ /**< iret opcode */
/*  80 */     OP_aam,      /* &first_byte[0xd4], */ /**< aam opcode */
/*  81 */     OP_aad,      /* &first_byte[0xd5], */ /**< aad opcode */
/*  82 */     OP_xlat,     /* &first_byte[0xd7], */ /**< xlat opcode */
/*  83 */     OP_in,       /* &first_byte[0xe5], */ /**< in opcode */
/*  84 */     OP_out,      /* &first_byte[0xe7], */ /**< out opcode */
/*  85 */     OP_hlt,      /* &first_byte[0xf4], */ /**< hlt opcode */
/*  86 */     OP_cmc,      /* &first_byte[0xf5], */ /**< cmc opcode */
/*  87 */     OP_clc,      /* &first_byte[0xf8], */ /**< clc opcode */
/*  88 */     OP_stc,      /* &first_byte[0xf9], */ /**< stc opcode */
/*  89 */     OP_cli,      /* &first_byte[0xfa], */ /**< cli opcode */
/*  90 */     OP_sti,      /* &first_byte[0xfb], */ /**< sti opcode */
/*  91 */     OP_cld,      /* &first_byte[0xfc], */ /**< cld opcode */
/*  92 */     OP_std,      /* &first_byte[0xfd], */ /**< std opcode */


/*  93 */     OP_lar,          /* &second_byte[0x02], */ /**< lar opcode */
/*  94 */     OP_lsl,          /* &second_byte[0x03], */ /**< lsl opcode */
/*  95 */     OP_syscall,      /* &second_byte[0x05], */ /**< syscall opcode */
/*  96 */     OP_clts,         /* &second_byte[0x06], */ /**< clts opcode */
/*  97 */     OP_sysret,       /* &second_byte[0x07], */ /**< sysret opcode */
/*  98 */     OP_invd,         /* &second_byte[0x08], */ /**< invd opcode */
/*  99 */     OP_wbinvd,       /* &second_byte[0x09], */ /**< wbinvd opcode */
/* 100 */     OP_ud2a,         /* &second_byte[0x0b], */ /**< ud2a opcode */
/* 101 */     OP_nop_modrm,    /* &second_byte[0x1f], */ /**< nop_modrm opcode */
/* 102 */     OP_movntps,      /* &prefix_extensions[11][0], */ /**< movntps opcode */
/* 103 */     OP_movntpd,      /* &prefix_extensions[11][2], */ /**< movntpd opcode */
/* 104 */     OP_wrmsr,        /* &second_byte[0x30], */ /**< wrmsr opcode */
/* 105 */     OP_rdtsc,        /* &second_byte[0x31], */ /**< rdtsc opcode */
/* 106 */     OP_rdmsr,        /* &second_byte[0x32], */ /**< rdmsr opcode */
/* 107 */     OP_rdpmc,        /* &second_byte[0x33], */ /**< rdpmc opcode */
/* 108 */     OP_sysenter,     /* &second_byte[0x34], */ /**< sysenter opcode */
/* 109 */     OP_sysexit,      /* &second_byte[0x35], */ /**< sysexit opcode */

/* 110 */     OP_cmovo,        /* &second_byte[0x40], */ /**< cmovo opcode */
/* 111 */     OP_cmovno,       /* &second_byte[0x41], */ /**< cmovno opcode */
/* 112 */     OP_cmovb,        /* &second_byte[0x42], */ /**< cmovb opcode */
/* 113 */     OP_cmovnb,       /* &second_byte[0x43], */ /**< cmovnb opcode */
/* 114 */     OP_cmovz,        /* &second_byte[0x44], */ /**< cmovz opcode */
/* 115 */     OP_cmovnz,       /* &second_byte[0x45], */ /**< cmovnz opcode */
/* 116 */     OP_cmovbe,       /* &second_byte[0x46], */ /**< cmovbe opcode */
/* 117 */     OP_cmovnbe,      /* &second_byte[0x47], */ /**< cmovnbe opcode */
/* 118 */     OP_cmovs,        /* &second_byte[0x48], */ /**< cmovs opcode */
/* 119 */     OP_cmovns,       /* &second_byte[0x49], */ /**< cmovns opcode */
/* 120 */     OP_cmovp,        /* &second_byte[0x4a], */ /**< cmovp opcode */
/* 121 */     OP_cmovnp,       /* &second_byte[0x4b], */ /**< cmovnp opcode */
/* 122 */     OP_cmovl,        /* &second_byte[0x4c], */ /**< cmovl opcode */
/* 123 */     OP_cmovnl,       /* &second_byte[0x4d], */ /**< cmovnl opcode */
/* 124 */     OP_cmovle,       /* &second_byte[0x4e], */ /**< cmovle opcode */
/* 125 */     OP_cmovnle,      /* &second_byte[0x4f], */ /**< cmovnle opcode */

/* 126 */     OP_punpcklbw,    /* &prefix_extensions[32][0], */ /**< punpcklbw opcode */
/* 127 */     OP_punpcklwd,    /* &prefix_extensions[33][0], */ /**< punpcklwd opcode */
/* 128 */     OP_punpckldq,    /* &prefix_extensions[34][0], */ /**< punpckldq opcode */
/* 129 */     OP_packsswb,     /* &prefix_extensions[35][0], */ /**< packsswb opcode */
/* 130 */     OP_pcmpgtb,      /* &prefix_extensions[36][0], */ /**< pcmpgtb opcode */
/* 131 */     OP_pcmpgtw,      /* &prefix_extensions[37][0], */ /**< pcmpgtw opcode */
/* 132 */     OP_pcmpgtd,      /* &prefix_extensions[38][0], */ /**< pcmpgtd opcode */
/* 133 */     OP_packuswb,     /* &prefix_extensions[39][0], */ /**< packuswb opcode */
/* 134 */     OP_punpckhbw,    /* &prefix_extensions[40][0], */ /**< punpckhbw opcode */
/* 135 */     OP_punpckhwd,    /* &prefix_extensions[41][0], */ /**< punpckhwd opcode */
/* 136 */     OP_punpckhdq,    /* &prefix_extensions[42][0], */ /**< punpckhdq opcode */
/* 137 */     OP_packssdw,     /* &prefix_extensions[43][0], */ /**< packssdw opcode */
/* 138 */     OP_punpcklqdq,   /* &prefix_extensions[44][2], */ /**< punpcklqdq opcode */
/* 139 */     OP_punpckhqdq,   /* &prefix_extensions[45][2], */ /**< punpckhqdq opcode */
/* 140 */     OP_movd,         /* &prefix_extensions[46][0], */ /**< movd opcode */
/* 141 */     OP_movq,         /* &prefix_extensions[112][0], */ /**< movq opcode */
/* 142 */     OP_movdqu,       /* &prefix_extensions[112][1],  */ /**< movdqu opcode */
/* 143 */     OP_movdqa,       /* &prefix_extensions[112][2], */ /**< movdqa opcode */
/* 144 */     OP_pshufw,       /* &prefix_extensions[47][0], */ /**< pshufw opcode */
/* 145 */     OP_pshufd,       /* &prefix_extensions[47][2], */ /**< pshufd opcode */
/* 146 */     OP_pshufhw,      /* &prefix_extensions[47][1], */ /**< pshufhw opcode */
/* 147 */     OP_pshuflw,      /* &prefix_extensions[47][3], */ /**< pshuflw opcode */
/* 148 */     OP_pcmpeqb,      /* &prefix_extensions[48][0], */ /**< pcmpeqb opcode */
/* 149 */     OP_pcmpeqw,      /* &prefix_extensions[49][0], */ /**< pcmpeqw opcode */
/* 150 */     OP_pcmpeqd,      /* &prefix_extensions[50][0], */ /**< pcmpeqd opcode */
/* 151 */     OP_emms,         /* &second_byte[0x77], */ /**< emms opcode */

/* 152 */     OP_jo,       /* &second_byte[0x80], */ /**< jo opcode */
/* 153 */     OP_jno,      /* &second_byte[0x81], */ /**< jno opcode */
/* 154 */     OP_jb,       /* &second_byte[0x82], */ /**< jb opcode */
/* 155 */     OP_jnb,      /* &second_byte[0x83], */ /**< jnb opcode */
/* 156 */     OP_jz,       /* &second_byte[0x84], */ /**< jz opcode */
/* 157 */     OP_jnz,      /* &second_byte[0x85], */ /**< jnz opcode */
/* 158 */     OP_jbe,      /* &second_byte[0x86], */ /**< jbe opcode */
/* 159 */     OP_jnbe,     /* &second_byte[0x87], */ /**< jnbe opcode */
/* 160 */     OP_js,       /* &second_byte[0x88], */ /**< js opcode */
/* 161 */     OP_jns,      /* &second_byte[0x89], */ /**< jns opcode */
/* 162 */     OP_jp,       /* &second_byte[0x8a], */ /**< jp opcode */
/* 163 */     OP_jnp,      /* &second_byte[0x8b], */ /**< jnp opcode */
/* 164 */     OP_jl,       /* &second_byte[0x8c], */ /**< jl opcode */
/* 165 */     OP_jnl,      /* &second_byte[0x8d], */ /**< jnl opcode */
/* 166 */     OP_jle,      /* &second_byte[0x8e], */ /**< jle opcode */
/* 167 */     OP_jnle,     /* &second_byte[0x8f], */ /**< jnle opcode */

/* 168 */     OP_seto,         /* &second_byte[0x90], */ /**< seto opcode */
/* 169 */     OP_setno,        /* &second_byte[0x91], */ /**< setno opcode */
/* 170 */     OP_setb,         /* &second_byte[0x92], */ /**< setb opcode */
/* 171 */     OP_setnb,        /* &second_byte[0x93], */ /**< setnb opcode */
/* 172 */     OP_setz,         /* &second_byte[0x94], */ /**< setz opcode */
/* 173 */     OP_setnz,        /* &second_byte[0x95], */ /**< setnz opcode */
/* 174 */     OP_setbe,        /* &second_byte[0x96], */ /**< setbe opcode */
/* 175 */     OP_setnbe,         /* &second_byte[0x97], */ /**< setnbe opcode */
/* 176 */     OP_sets,         /* &second_byte[0x98], */ /**< sets opcode */
/* 177 */     OP_setns,        /* &second_byte[0x99], */ /**< setns opcode */
/* 178 */     OP_setp,         /* &second_byte[0x9a], */ /**< setp opcode */
/* 179 */     OP_setnp,        /* &second_byte[0x9b], */ /**< setnp opcode */
/* 180 */     OP_setl,         /* &second_byte[0x9c], */ /**< setl opcode */
/* 181 */     OP_setnl,        /* &second_byte[0x9d], */ /**< setnl opcode */
/* 182 */     OP_setle,        /* &second_byte[0x9e], */ /**< setle opcode */
/* 183 */     OP_setnle,         /* &second_byte[0x9f], */ /**< setnle opcode */

/* 184 */     OP_cpuid,        /* &second_byte[0xa2], */ /**< cpuid opcode */
/* 185 */     OP_bt,           /* &second_byte[0xa3], */ /**< bt opcode */
/* 186 */     OP_shld,         /* &second_byte[0xa4], */ /**< shld opcode */
/* 187 */     OP_rsm,          /* &second_byte[0xaa], */ /**< rsm opcode */
/* 188 */     OP_bts,          /* &second_byte[0xab], */ /**< bts opcode */
/* 189 */     OP_shrd,         /* &second_byte[0xac], */ /**< shrd opcode */
/* 190 */     OP_cmpxchg,      /* &second_byte[0xb1], */ /**< cmpxchg opcode */
/* 191 */     OP_lss,          /* &second_byte[0xb2], */ /**< lss opcode */
/* 192 */     OP_btr,          /* &second_byte[0xb3], */ /**< btr opcode */
/* 193 */     OP_lfs,          /* &second_byte[0xb4], */ /**< lfs opcode */
/* 194 */     OP_lgs,          /* &second_byte[0xb5], */ /**< lgs opcode */
/* 195 */     OP_movzx,        /* &second_byte[0xb7], */ /**< movzx opcode */
/* 196 */     OP_ud2b,         /* &second_byte[0xb9], */ /**< ud2b opcode */
/* 197 */     OP_btc,          /* &second_byte[0xbb], */ /**< btc opcode */
/* 198 */     OP_bsf,          /* &second_byte[0xbc], */ /**< bsf opcode */
/* 199 */     OP_bsr,          /* &prefix_extensions[136][0], */ /**< bsr opcode */
/* 200 */     OP_movsx,        /* &second_byte[0xbf], */ /**< movsx opcode */
/* 201 */     OP_xadd,         /* &second_byte[0xc1], */ /**< xadd opcode */
/* 202 */     OP_movnti,       /* &second_byte[0xc3], */ /**< movnti opcode */
/* 203 */     OP_pinsrw,       /* &prefix_extensions[53][0], */ /**< pinsrw opcode */
/* 204 */     OP_pextrw,       /* &prefix_extensions[54][0], */ /**< pextrw opcode */
/* 205 */     OP_bswap,        /* &second_byte[0xc8], */ /**< bswap opcode */
/* 206 */     OP_psrlw,        /* &prefix_extensions[56][0], */ /**< psrlw opcode */
/* 207 */     OP_psrld,        /* &prefix_extensions[57][0], */ /**< psrld opcode */
/* 208 */     OP_psrlq,        /* &prefix_extensions[58][0], */ /**< psrlq opcode */
/* 209 */     OP_paddq,        /* &prefix_extensions[59][0], */ /**< paddq opcode */
/* 210 */     OP_pmullw,       /* &prefix_extensions[60][0], */ /**< pmullw opcode */
/* 211 */     OP_pmovmskb,     /* &prefix_extensions[62][0], */ /**< pmovmskb opcode */
/* 212 */     OP_psubusb,      /* &prefix_extensions[63][0], */ /**< psubusb opcode */
/* 213 */     OP_psubusw,      /* &prefix_extensions[64][0], */ /**< psubusw opcode */
/* 214 */     OP_pminub,       /* &prefix_extensions[65][0], */ /**< pminub opcode */
/* 215 */     OP_pand,         /* &prefix_extensions[66][0], */ /**< pand opcode */
/* 216 */     OP_paddusb,      /* &prefix_extensions[67][0], */ /**< paddusb opcode */
/* 217 */     OP_paddusw,      /* &prefix_extensions[68][0], */ /**< paddusw opcode */
/* 218 */     OP_pmaxub,       /* &prefix_extensions[69][0], */ /**< pmaxub opcode */
/* 219 */     OP_pandn,        /* &prefix_extensions[70][0], */ /**< pandn opcode */
/* 220 */     OP_pavgb,        /* &prefix_extensions[71][0], */ /**< pavgb opcode */
/* 221 */     OP_psraw,        /* &prefix_extensions[72][0], */ /**< psraw opcode */
/* 222 */     OP_psrad,        /* &prefix_extensions[73][0], */ /**< psrad opcode */
/* 223 */     OP_pavgw,        /* &prefix_extensions[74][0], */ /**< pavgw opcode */
/* 224 */     OP_pmulhuw,      /* &prefix_extensions[75][0], */ /**< pmulhuw opcode */
/* 225 */     OP_pmulhw,       /* &prefix_extensions[76][0], */ /**< pmulhw opcode */
/* 226 */     OP_movntq,       /* &prefix_extensions[78][0], */ /**< movntq opcode */
/* 227 */     OP_movntdq,      /* &prefix_extensions[78][2], */ /**< movntdq opcode */
/* 228 */     OP_psubsb,       /* &prefix_extensions[79][0], */ /**< psubsb opcode */
/* 229 */     OP_psubsw,       /* &prefix_extensions[80][0], */ /**< psubsw opcode */
/* 230 */     OP_pminsw,       /* &prefix_extensions[81][0], */ /**< pminsw opcode */
/* 231 */     OP_por,          /* &prefix_extensions[82][0], */ /**< por opcode */
/* 232 */     OP_paddsb,       /* &prefix_extensions[83][0], */ /**< paddsb opcode */
/* 233 */     OP_paddsw,       /* &prefix_extensions[84][0], */ /**< paddsw opcode */
/* 234 */     OP_pmaxsw,       /* &prefix_extensions[85][0], */ /**< pmaxsw opcode */
/* 235 */     OP_pxor,         /* &prefix_extensions[86][0], */ /**< pxor opcode */
/* 236 */     OP_psllw,        /* &prefix_extensions[87][0], */ /**< psllw opcode */
/* 237 */     OP_pslld,        /* &prefix_extensions[88][0], */ /**< pslld opcode */
/* 238 */     OP_psllq,        /* &prefix_extensions[89][0], */ /**< psllq opcode */
/* 239 */     OP_pmuludq,      /* &prefix_extensions[90][0], */ /**< pmuludq opcode */
/* 240 */     OP_pmaddwd,      /* &prefix_extensions[91][0], */ /**< pmaddwd opcode */
/* 241 */     OP_psadbw,       /* &prefix_extensions[92][0], */ /**< psadbw opcode */
/* 242 */     OP_maskmovq,     /* &prefix_extensions[93][0], */ /**< maskmovq opcode */
/* 243 */     OP_maskmovdqu,   /* &prefix_extensions[93][2], */ /**< maskmovdqu opcode */
/* 244 */     OP_psubb,        /* &prefix_extensions[94][0], */ /**< psubb opcode */
/* 245 */     OP_psubw,        /* &prefix_extensions[95][0], */ /**< psubw opcode */
/* 246 */     OP_psubd,        /* &prefix_extensions[96][0], */ /**< psubd opcode */
/* 247 */     OP_psubq,        /* &prefix_extensions[97][0], */ /**< psubq opcode */
/* 248 */     OP_paddb,        /* &prefix_extensions[98][0], */ /**< paddb opcode */
/* 249 */     OP_paddw,        /* &prefix_extensions[99][0], */ /**< paddw opcode */
/* 250 */     OP_paddd,        /* &prefix_extensions[100][0], */ /**< paddd opcode */
/* 251 */     OP_psrldq,       /* &prefix_extensions[101][2], */ /**< psrldq opcode */
/* 252 */     OP_pslldq,       /* &prefix_extensions[102][2], */ /**< pslldq opcode */


/* 253 */     OP_rol,           /* &extensions[ 4][0], */ /**< rol opcode */
/* 254 */     OP_ror,           /* &extensions[ 4][1], */ /**< ror opcode */
/* 255 */     OP_rcl,           /* &extensions[ 4][2], */ /**< rcl opcode */
/* 256 */     OP_rcr,           /* &extensions[ 4][3], */ /**< rcr opcode */
/* 257 */     OP_shl,           /* &extensions[ 4][4], */ /**< shl opcode */
/* 258 */     OP_shr,           /* &extensions[ 4][5], */ /**< shr opcode */
/* 259 */     OP_sar,           /* &extensions[ 4][7], */ /**< sar opcode */
/* 260 */     OP_not,           /* &extensions[10][2], */ /**< not opcode */
/* 261 */     OP_neg,           /* &extensions[10][3], */ /**< neg opcode */
/* 262 */     OP_mul,           /* &extensions[10][4], */ /**< mul opcode */
/* 263 */     OP_div,           /* &extensions[10][6], */ /**< div opcode */
/* 264 */     OP_idiv,          /* &extensions[10][7], */ /**< idiv opcode */
/* 265 */     OP_sldt,          /* &extensions[13][0], */ /**< sldt opcode */
/* 266 */     OP_str,           /* &extensions[13][1], */ /**< str opcode */
/* 267 */     OP_lldt,          /* &extensions[13][2], */ /**< lldt opcode */
/* 268 */     OP_ltr,           /* &extensions[13][3], */ /**< ltr opcode */
/* 269 */     OP_verr,          /* &extensions[13][4], */ /**< verr opcode */
/* 270 */     OP_verw,          /* &extensions[13][5], */ /**< verw opcode */
/* 271 */     OP_sgdt,          /* &mod_extensions[0][0], */ /**< sgdt opcode */
/* 272 */     OP_sidt,          /* &mod_extensions[1][0], */ /**< sidt opcode */
/* 273 */     OP_lgdt,          /* &extensions[14][2], */ /**< lgdt opcode */
/* 274 */     OP_lidt,          /* &extensions[14][3], */ /**< lidt opcode */
/* 275 */     OP_smsw,          /* &extensions[14][4], */ /**< smsw opcode */
/* 276 */     OP_lmsw,          /* &extensions[14][6], */ /**< lmsw opcode */
/* 277 */     OP_invlpg,        /* &mod_extensions[2][0], */ /**< invlpg opcode */
/* 278 */     OP_cmpxchg8b,     /* &extensions[16][1], */ /**< cmpxchg8b opcode */
/* 279 */     OP_fxsave,        /* &extensions[22][0], */ /**< fxsave opcode */
/* 280 */     OP_fxrstor,       /* &extensions[22][1], */ /**< fxrstor opcode */
/* 281 */     OP_ldmxcsr,       /* &extensions[22][2], */ /**< ldmxcsr opcode */
/* 282 */     OP_stmxcsr,       /* &extensions[22][3], */ /**< stmxcsr opcode */
/* 283 */     OP_lfence,        /* &extensions[22][5], */ /**< lfence opcode */
/* 284 */     OP_mfence,        /* &extensions[22][6], */ /**< mfence opcode */
/* 285 */     OP_clflush,       /* &mod_extensions[3][0], */ /**< clflush opcode */
/* 286 */     OP_sfence,        /* &mod_extensions[3][1], */ /**< sfence opcode */
/* 287 */     OP_prefetchnta,   /* &extensions[23][0], */ /**< prefetchnta opcode */
/* 288 */     OP_prefetcht0,    /* &extensions[23][1], */ /**< prefetcht0 opcode */
/* 289 */     OP_prefetcht1,    /* &extensions[23][2], */ /**< prefetcht1 opcode */
/* 290 */     OP_prefetcht2,    /* &extensions[23][3], */ /**< prefetcht2 opcode */
/* 291 */     OP_prefetch,      /* &extensions[24][0], */ /**< prefetch opcode */
/* 292 */     OP_prefetchw,     /* &extensions[24][1], */ /**< prefetchw opcode */


/* 293 */     OP_movups,      /* &prefix_extensions[ 0][0], */ /**< movups opcode */
/* 294 */     OP_movss,       /* &prefix_extensions[ 0][1], */ /**< movss opcode */
/* 295 */     OP_movupd,      /* &prefix_extensions[ 0][2], */ /**< movupd opcode */
/* 296 */     OP_movsd,       /* &prefix_extensions[ 0][3], */ /**< movsd opcode */
/* 297 */     OP_movlps,      /* &prefix_extensions[ 2][0], */ /**< movlps opcode */
/* 298 */     OP_movlpd,      /* &prefix_extensions[ 2][2], */ /**< movlpd opcode */
/* 299 */     OP_unpcklps,    /* &prefix_extensions[ 4][0], */ /**< unpcklps opcode */
/* 300 */     OP_unpcklpd,    /* &prefix_extensions[ 4][2], */ /**< unpcklpd opcode */
/* 301 */     OP_unpckhps,    /* &prefix_extensions[ 5][0], */ /**< unpckhps opcode */
/* 302 */     OP_unpckhpd,    /* &prefix_extensions[ 5][2], */ /**< unpckhpd opcode */
/* 303 */     OP_movhps,      /* &prefix_extensions[ 6][0], */ /**< movhps opcode */
/* 304 */     OP_movhpd,      /* &prefix_extensions[ 6][2], */ /**< movhpd opcode */
/* 305 */     OP_movaps,      /* &prefix_extensions[ 8][0], */ /**< movaps opcode */
/* 306 */     OP_movapd,      /* &prefix_extensions[ 8][2], */ /**< movapd opcode */
/* 307 */     OP_cvtpi2ps,    /* &prefix_extensions[10][0], */ /**< cvtpi2ps opcode */
/* 308 */     OP_cvtsi2ss,    /* &prefix_extensions[10][1], */ /**< cvtsi2ss opcode */
/* 309 */     OP_cvtpi2pd,    /* &prefix_extensions[10][2], */ /**< cvtpi2pd opcode */
/* 310 */     OP_cvtsi2sd,    /* &prefix_extensions[10][3], */ /**< cvtsi2sd opcode */
/* 311 */     OP_cvttps2pi,   /* &prefix_extensions[12][0], */ /**< cvttps2pi opcode */
/* 312 */     OP_cvttss2si,   /* &prefix_extensions[12][1], */ /**< cvttss2si opcode */
/* 313 */     OP_cvttpd2pi,   /* &prefix_extensions[12][2], */ /**< cvttpd2pi opcode */
/* 314 */     OP_cvttsd2si,   /* &prefix_extensions[12][3], */ /**< cvttsd2si opcode */
/* 315 */     OP_cvtps2pi,    /* &prefix_extensions[13][0], */ /**< cvtps2pi opcode */
/* 316 */     OP_cvtss2si,    /* &prefix_extensions[13][1], */ /**< cvtss2si opcode */
/* 317 */     OP_cvtpd2pi,    /* &prefix_extensions[13][2], */ /**< cvtpd2pi opcode */
/* 318 */     OP_cvtsd2si,    /* &prefix_extensions[13][3], */ /**< cvtsd2si opcode */
/* 319 */     OP_ucomiss,     /* &prefix_extensions[14][0], */ /**< ucomiss opcode */
/* 320 */     OP_ucomisd,     /* &prefix_extensions[14][2], */ /**< ucomisd opcode */
/* 321 */     OP_comiss,      /* &prefix_extensions[15][0], */ /**< comiss opcode */
/* 322 */     OP_comisd,      /* &prefix_extensions[15][2], */ /**< comisd opcode */
/* 323 */     OP_movmskps,    /* &prefix_extensions[16][0], */ /**< movmskps opcode */
/* 324 */     OP_movmskpd,    /* &prefix_extensions[16][2], */ /**< movmskpd opcode */
/* 325 */     OP_sqrtps,      /* &prefix_extensions[17][0], */ /**< sqrtps opcode */
/* 326 */     OP_sqrtss,      /* &prefix_extensions[17][1], */ /**< sqrtss opcode */
/* 327 */     OP_sqrtpd,      /* &prefix_extensions[17][2], */ /**< sqrtpd opcode */
/* 328 */     OP_sqrtsd,      /* &prefix_extensions[17][3], */ /**< sqrtsd opcode */
/* 329 */     OP_rsqrtps,     /* &prefix_extensions[18][0], */ /**< rsqrtps opcode */
/* 330 */     OP_rsqrtss,     /* &prefix_extensions[18][1], */ /**< rsqrtss opcode */
/* 331 */     OP_rcpps,       /* &prefix_extensions[19][0], */ /**< rcpps opcode */
/* 332 */     OP_rcpss,       /* &prefix_extensions[19][1], */ /**< rcpss opcode */
/* 333 */     OP_andps,       /* &prefix_extensions[20][0], */ /**< andps opcode */
/* 334 */     OP_andpd,       /* &prefix_extensions[20][2], */ /**< andpd opcode */
/* 335 */     OP_andnps,      /* &prefix_extensions[21][0], */ /**< andnps opcode */
/* 336 */     OP_andnpd,      /* &prefix_extensions[21][2], */ /**< andnpd opcode */
/* 337 */     OP_orps,        /* &prefix_extensions[22][0], */ /**< orps opcode */
/* 338 */     OP_orpd,        /* &prefix_extensions[22][2], */ /**< orpd opcode */
/* 339 */     OP_xorps,       /* &prefix_extensions[23][0], */ /**< xorps opcode */
/* 340 */     OP_xorpd,       /* &prefix_extensions[23][2], */ /**< xorpd opcode */
/* 341 */     OP_addps,       /* &prefix_extensions[24][0], */ /**< addps opcode */
/* 342 */     OP_addss,       /* &prefix_extensions[24][1], */ /**< addss opcode */
/* 343 */     OP_addpd,       /* &prefix_extensions[24][2], */ /**< addpd opcode */
/* 344 */     OP_addsd,       /* &prefix_extensions[24][3], */ /**< addsd opcode */
/* 345 */     OP_mulps,       /* &prefix_extensions[25][0], */ /**< mulps opcode */
/* 346 */     OP_mulss,       /* &prefix_extensions[25][1], */ /**< mulss opcode */
/* 347 */     OP_mulpd,       /* &prefix_extensions[25][2], */ /**< mulpd opcode */
/* 348 */     OP_mulsd,       /* &prefix_extensions[25][3], */ /**< mulsd opcode */
/* 349 */     OP_cvtps2pd,    /* &prefix_extensions[26][0], */ /**< cvtps2pd opcode */
/* 350 */     OP_cvtss2sd,    /* &prefix_extensions[26][1], */ /**< cvtss2sd opcode */
/* 351 */     OP_cvtpd2ps,    /* &prefix_extensions[26][2], */ /**< cvtpd2ps opcode */
/* 352 */     OP_cvtsd2ss,    /* &prefix_extensions[26][3], */ /**< cvtsd2ss opcode */
/* 353 */     OP_cvtdq2ps,    /* &prefix_extensions[27][0], */ /**< cvtdq2ps opcode */
/* 354 */     OP_cvttps2dq,   /* &prefix_extensions[27][1], */ /**< cvttps2dq opcode */
/* 355 */     OP_cvtps2dq,    /* &prefix_extensions[27][2], */ /**< cvtps2dq opcode */
/* 356 */     OP_subps,       /* &prefix_extensions[28][0], */ /**< subps opcode */
/* 357 */     OP_subss,       /* &prefix_extensions[28][1], */ /**< subss opcode */
/* 358 */     OP_subpd,       /* &prefix_extensions[28][2], */ /**< subpd opcode */
/* 359 */     OP_subsd,       /* &prefix_extensions[28][3], */ /**< subsd opcode */
/* 360 */     OP_minps,       /* &prefix_extensions[29][0], */ /**< minps opcode */
/* 361 */     OP_minss,       /* &prefix_extensions[29][1], */ /**< minss opcode */
/* 362 */     OP_minpd,       /* &prefix_extensions[29][2], */ /**< minpd opcode */
/* 363 */     OP_minsd,       /* &prefix_extensions[29][3], */ /**< minsd opcode */
/* 364 */     OP_divps,       /* &prefix_extensions[30][0], */ /**< divps opcode */
/* 365 */     OP_divss,       /* &prefix_extensions[30][1], */ /**< divss opcode */
/* 366 */     OP_divpd,       /* &prefix_extensions[30][2], */ /**< divpd opcode */
/* 367 */     OP_divsd,       /* &prefix_extensions[30][3], */ /**< divsd opcode */
/* 368 */     OP_maxps,       /* &prefix_extensions[31][0], */ /**< maxps opcode */
/* 369 */     OP_maxss,       /* &prefix_extensions[31][1], */ /**< maxss opcode */
/* 370 */     OP_maxpd,       /* &prefix_extensions[31][2], */ /**< maxpd opcode */
/* 371 */     OP_maxsd,       /* &prefix_extensions[31][3], */ /**< maxsd opcode */
/* 372 */     OP_cmpps,       /* &prefix_extensions[52][0], */ /**< cmpps opcode */
/* 373 */     OP_cmpss,       /* &prefix_extensions[52][1], */ /**< cmpss opcode */
/* 374 */     OP_cmppd,       /* &prefix_extensions[52][2], */ /**< cmppd opcode */
/* 375 */     OP_cmpsd,       /* &prefix_extensions[52][3], */ /**< cmpsd opcode */
/* 376 */     OP_shufps,      /* &prefix_extensions[55][0], */ /**< shufps opcode */
/* 377 */     OP_shufpd,      /* &prefix_extensions[55][2], */ /**< shufpd opcode */
/* 378 */     OP_cvtdq2pd,    /* &prefix_extensions[77][1], */ /**< cvtdq2pd opcode */
/* 379 */     OP_cvttpd2dq,   /* &prefix_extensions[77][2], */ /**< cvttpd2dq opcode */
/* 380 */     OP_cvtpd2dq,    /* &prefix_extensions[77][3], */ /**< cvtpd2dq opcode */
/* 381 */     OP_nop,         /* &prefix_extensions[103][0], */ /**< nop opcode */
/* 382 */     OP_pause,       /* &prefix_extensions[103][1], */ /**< pause opcode */

/* 383 */     OP_ins,          /* &rep_extensions[1][0], */ /**< ins opcode */
/* 384 */     OP_rep_ins,      /* &rep_extensions[1][2], */ /**< rep_ins opcode */
/* 385 */     OP_outs,         /* &rep_extensions[3][0], */ /**< outs opcode */
/* 386 */     OP_rep_outs,     /* &rep_extensions[3][2], */ /**< rep_outs opcode */
/* 387 */     OP_movs,         /* &rep_extensions[5][0], */ /**< movs opcode */
/* 388 */     OP_rep_movs,     /* &rep_extensions[5][2], */ /**< rep_movs opcode */
/* 389 */     OP_stos,         /* &rep_extensions[7][0], */ /**< stos opcode */
/* 390 */     OP_rep_stos,     /* &rep_extensions[7][2], */ /**< rep_stos opcode */
/* 391 */     OP_lods,         /* &rep_extensions[9][0], */ /**< lods opcode */
/* 392 */     OP_rep_lods,     /* &rep_extensions[9][2], */ /**< rep_lods opcode */
/* 393 */     OP_cmps,         /* &repne_extensions[1][0], */ /**< cmps opcode */
/* 394 */     OP_rep_cmps,     /* &repne_extensions[1][2], */ /**< rep_cmps opcode */
/* 395 */     OP_repne_cmps,   /* &repne_extensions[1][4], */ /**< repne_cmps opcode */
/* 396 */     OP_scas,         /* &repne_extensions[3][0], */ /**< scas opcode */
/* 397 */     OP_rep_scas,     /* &repne_extensions[3][2], */ /**< rep_scas opcode */
/* 398 */     OP_repne_scas,   /* &repne_extensions[3][4], */ /**< repne_scas opcode */


/* 399 */     OP_fadd,      /* &float_low_modrm[0x00], */ /**< fadd opcode */
/* 400 */     OP_fmul,      /* &float_low_modrm[0x01], */ /**< fmul opcode */
/* 401 */     OP_fcom,      /* &float_low_modrm[0x02], */ /**< fcom opcode */
/* 402 */     OP_fcomp,     /* &float_low_modrm[0x03], */ /**< fcomp opcode */
/* 403 */     OP_fsub,      /* &float_low_modrm[0x04], */ /**< fsub opcode */
/* 404 */     OP_fsubr,     /* &float_low_modrm[0x05], */ /**< fsubr opcode */
/* 405 */     OP_fdiv,      /* &float_low_modrm[0x06], */ /**< fdiv opcode */
/* 406 */     OP_fdivr,     /* &float_low_modrm[0x07], */ /**< fdivr opcode */
/* 407 */     OP_fld,       /* &float_low_modrm[0x08], */ /**< fld opcode */
/* 408 */     OP_fst,       /* &float_low_modrm[0x0a], */ /**< fst opcode */
/* 409 */     OP_fstp,      /* &float_low_modrm[0x0b], */ /**< fstp opcode */
/* 410 */     OP_fldenv,    /* &float_low_modrm[0x0c], */ /**< fldenv opcode */
/* 411 */     OP_fldcw,     /* &float_low_modrm[0x0d], */ /**< fldcw opcode */
/* 412 */     OP_fnstenv,   /* &float_low_modrm[0x0e], */ /**< fnstenv opcode */
/* 413 */     OP_fnstcw,    /* &float_low_modrm[0x0f], */ /**< fnstcw opcode */
/* 414 */     OP_fiadd,     /* &float_low_modrm[0x10], */ /**< fiadd opcode */
/* 415 */     OP_fimul,     /* &float_low_modrm[0x11], */ /**< fimul opcode */
/* 416 */     OP_ficom,     /* &float_low_modrm[0x12], */ /**< ficom opcode */
/* 417 */     OP_ficomp,    /* &float_low_modrm[0x13], */ /**< ficomp opcode */
/* 418 */     OP_fisub,     /* &float_low_modrm[0x14], */ /**< fisub opcode */
/* 419 */     OP_fisubr,    /* &float_low_modrm[0x15], */ /**< fisubr opcode */
/* 420 */     OP_fidiv,     /* &float_low_modrm[0x16], */ /**< fidiv opcode */
/* 421 */     OP_fidivr,    /* &float_low_modrm[0x17], */ /**< fidivr opcode */
/* 422 */     OP_fild,      /* &float_low_modrm[0x18], */ /**< fild opcode */
/* 423 */     OP_fist,      /* &float_low_modrm[0x1a], */ /**< fist opcode */
/* 424 */     OP_fistp,     /* &float_low_modrm[0x1b], */ /**< fistp opcode */
/* 425 */     OP_frstor,    /* &float_low_modrm[0x2c], */ /**< frstor opcode */
/* 426 */     OP_fnsave,    /* &float_low_modrm[0x2e], */ /**< fnsave opcode */
/* 427 */     OP_fnstsw,    /* &float_low_modrm[0x2f], */ /**< fnstsw opcode */

/* 428 */     OP_fbld,      /* &float_low_modrm[0x3c], */ /**< fbld opcode */
/* 429 */     OP_fbstp,     /* &float_low_modrm[0x3e], */ /**< fbstp opcode */


/* 430 */     OP_fxch,       /* &float_high_modrm[1][0x08], */ /**< fxch opcode */
/* 431 */     OP_fnop,       /* &float_high_modrm[1][0x10], */ /**< fnop opcode */
/* 432 */     OP_fchs,       /* &float_high_modrm[1][0x20], */ /**< fchs opcode */
/* 433 */     OP_fabs,       /* &float_high_modrm[1][0x21], */ /**< fabs opcode */
/* 434 */     OP_ftst,       /* &float_high_modrm[1][0x24], */ /**< ftst opcode */
/* 435 */     OP_fxam,       /* &float_high_modrm[1][0x25], */ /**< fxam opcode */
/* 436 */     OP_fld1,       /* &float_high_modrm[1][0x28], */ /**< fld1 opcode */
/* 437 */     OP_fldl2t,     /* &float_high_modrm[1][0x29], */ /**< fldl2t opcode */
/* 438 */     OP_fldl2e,     /* &float_high_modrm[1][0x2a], */ /**< fldl2e opcode */
/* 439 */     OP_fldpi,      /* &float_high_modrm[1][0x2b], */ /**< fldpi opcode */
/* 440 */     OP_fldlg2,     /* &float_high_modrm[1][0x2c], */ /**< fldlg2 opcode */
/* 441 */     OP_fldln2,     /* &float_high_modrm[1][0x2d], */ /**< fldln2 opcode */
/* 442 */     OP_fldz,       /* &float_high_modrm[1][0x2e], */ /**< fldz opcode */
/* 443 */     OP_f2xm1,      /* &float_high_modrm[1][0x30], */ /**< f2xm1 opcode */
/* 444 */     OP_fyl2x,      /* &float_high_modrm[1][0x31], */ /**< fyl2x opcode */
/* 445 */     OP_fptan,      /* &float_high_modrm[1][0x32], */ /**< fptan opcode */
/* 446 */     OP_fpatan,     /* &float_high_modrm[1][0x33], */ /**< fpatan opcode */
/* 447 */     OP_fxtract,    /* &float_high_modrm[1][0x34], */ /**< fxtract opcode */
/* 448 */     OP_fprem1,     /* &float_high_modrm[1][0x35], */ /**< fprem1 opcode */
/* 449 */     OP_fdecstp,    /* &float_high_modrm[1][0x36], */ /**< fdecstp opcode */
/* 450 */     OP_fincstp,    /* &float_high_modrm[1][0x37], */ /**< fincstp opcode */
/* 451 */     OP_fprem,      /* &float_high_modrm[1][0x38], */ /**< fprem opcode */
/* 452 */     OP_fyl2xp1,    /* &float_high_modrm[1][0x39], */ /**< fyl2xp1 opcode */
/* 453 */     OP_fsqrt,      /* &float_high_modrm[1][0x3a], */ /**< fsqrt opcode */
/* 454 */     OP_fsincos,    /* &float_high_modrm[1][0x3b], */ /**< fsincos opcode */
/* 455 */     OP_frndint,    /* &float_high_modrm[1][0x3c], */ /**< frndint opcode */
/* 456 */     OP_fscale,     /* &float_high_modrm[1][0x3d], */ /**< fscale opcode */
/* 457 */     OP_fsin,       /* &float_high_modrm[1][0x3e], */ /**< fsin opcode */
/* 458 */     OP_fcos,       /* &float_high_modrm[1][0x3f], */ /**< fcos opcode */
/* 459 */     OP_fcmovb,     /* &float_high_modrm[2][0x00], */ /**< fcmovb opcode */
/* 460 */     OP_fcmove,     /* &float_high_modrm[2][0x08], */ /**< fcmove opcode */
/* 461 */     OP_fcmovbe,    /* &float_high_modrm[2][0x10], */ /**< fcmovbe opcode */
/* 462 */     OP_fcmovu,     /* &float_high_modrm[2][0x18], */ /**< fcmovu opcode */
/* 463 */     OP_fucompp,    /* &float_high_modrm[2][0x29], */ /**< fucompp opcode */
/* 464 */     OP_fcmovnb,    /* &float_high_modrm[3][0x00], */ /**< fcmovnb opcode */
/* 465 */     OP_fcmovne,    /* &float_high_modrm[3][0x08], */ /**< fcmovne opcode */
/* 466 */     OP_fcmovnbe,   /* &float_high_modrm[3][0x10], */ /**< fcmovnbe opcode */
/* 467 */     OP_fcmovnu,    /* &float_high_modrm[3][0x18], */ /**< fcmovnu opcode */
/* 468 */     OP_fnclex,     /* &float_high_modrm[3][0x22], */ /**< fnclex opcode */
/* 469 */     OP_fninit,     /* &float_high_modrm[3][0x23], */ /**< fninit opcode */
/* 470 */     OP_fucomi,     /* &float_high_modrm[3][0x28], */ /**< fucomi opcode */
/* 471 */     OP_fcomi,      /* &float_high_modrm[3][0x30], */ /**< fcomi opcode */
/* 472 */     OP_ffree,      /* &float_high_modrm[5][0x00], */ /**< ffree opcode */
/* 473 */     OP_fucom,      /* &float_high_modrm[5][0x20], */ /**< fucom opcode */
/* 474 */     OP_fucomp,     /* &float_high_modrm[5][0x28], */ /**< fucomp opcode */
/* 475 */     OP_faddp,      /* &float_high_modrm[6][0x00], */ /**< faddp opcode */
/* 476 */     OP_fmulp,      /* &float_high_modrm[6][0x08], */ /**< fmulp opcode */
/* 477 */     OP_fcompp,     /* &float_high_modrm[6][0x19], */ /**< fcompp opcode */
/* 478 */     OP_fsubrp,     /* &float_high_modrm[6][0x20], */ /**< fsubrp opcode */
/* 479 */     OP_fsubp,      /* &float_high_modrm[6][0x28], */ /**< fsubp opcode */
/* 480 */     OP_fdivrp,     /* &float_high_modrm[6][0x30], */ /**< fdivrp opcode */
/* 481 */     OP_fdivp,      /* &float_high_modrm[6][0x38], */ /**< fdivp opcode */
/* 482 */     OP_fucomip,    /* &float_high_modrm[7][0x28], */ /**< fucomip opcode */
/* 483 */     OP_fcomip,     /* &float_high_modrm[7][0x30], */ /**< fcomip opcode */

    /* SSE3 instructions */
/* 484 */     OP_fisttp,       /* &float_low_modrm[0x29], */ /**< fisttp opcode */
/* 485 */     OP_haddpd,       /* &prefix_extensions[114][2], */ /**< haddpd opcode */
/* 486 */     OP_haddps,       /* &prefix_extensions[114][3], */ /**< haddps opcode */
/* 487 */     OP_hsubpd,       /* &prefix_extensions[115][2], */ /**< hsubpd opcode */
/* 488 */     OP_hsubps,       /* &prefix_extensions[115][3], */ /**< hsubps opcode */
/* 489 */     OP_addsubpd,     /* &prefix_extensions[116][2], */ /**< addsubpd opcode */
/* 490 */     OP_addsubps,     /* &prefix_extensions[116][3], */ /**< addsubps opcode */
/* 491 */     OP_lddqu,        /* &prefix_extensions[117][3], */ /**< lddqu opcode */
/* 492 */     OP_monitor,      /* &rm_extensions[1][0], */ /**< monitor opcode */
/* 493 */     OP_mwait,        /* &rm_extensions[1][1], */ /**< mwait opcode */
/* 494 */     OP_movsldup,     /* &prefix_extensions[ 2][1], */ /**< movsldup opcode */
/* 495 */     OP_movshdup,     /* &prefix_extensions[ 6][1], */ /**< movshdup opcode */
/* 496 */     OP_movddup,      /* &prefix_extensions[ 2][3], */ /**< movddup opcode */

    /* 3D-Now! instructions */
/* 497 */     OP_femms,          /* &second_byte[0x0e], */ /**< femms opcode */
/* 498 */     OP_unknown_3dnow,  /* &suffix_extensions[0], */ /**< unknown_3dnow opcode */
/* 499 */     OP_pavgusb,        /* &suffix_extensions[1], */ /**< pavgusb opcode */
/* 500 */     OP_pfadd,          /* &suffix_extensions[2], */ /**< pfadd opcode */
/* 501 */     OP_pfacc,          /* &suffix_extensions[3], */ /**< pfacc opcode */
/* 502 */     OP_pfcmpge,        /* &suffix_extensions[4], */ /**< pfcmpge opcode */
/* 503 */     OP_pfcmpgt,        /* &suffix_extensions[5], */ /**< pfcmpgt opcode */
/* 504 */     OP_pfcmpeq,        /* &suffix_extensions[6], */ /**< pfcmpeq opcode */
/* 505 */     OP_pfmin,          /* &suffix_extensions[7], */ /**< pfmin opcode */
/* 506 */     OP_pfmax,          /* &suffix_extensions[8], */ /**< pfmax opcode */
/* 507 */     OP_pfmul,          /* &suffix_extensions[9], */ /**< pfmul opcode */
/* 508 */     OP_pfrcp,          /* &suffix_extensions[10], */ /**< pfrcp opcode */
/* 509 */     OP_pfrcpit1,       /* &suffix_extensions[11], */ /**< pfrcpit1 opcode */
/* 510 */     OP_pfrcpit2,       /* &suffix_extensions[12], */ /**< pfrcpit2 opcode */
/* 511 */     OP_pfrsqrt,        /* &suffix_extensions[13], */ /**< pfrsqrt opcode */
/* 512 */     OP_pfrsqit1,       /* &suffix_extensions[14], */ /**< pfrsqit1 opcode */
/* 513 */     OP_pmulhrw,        /* &suffix_extensions[15], */ /**< pmulhrw opcode */
/* 514 */     OP_pfsub,          /* &suffix_extensions[16], */ /**< pfsub opcode */
/* 515 */     OP_pfsubr,         /* &suffix_extensions[17], */ /**< pfsubr opcode */
/* 516 */     OP_pi2fd,          /* &suffix_extensions[18], */ /**< pi2fd opcode */
/* 517 */     OP_pf2id,          /* &suffix_extensions[19], */ /**< pf2id opcode */
/* 518 */     OP_pi2fw,          /* &suffix_extensions[20], */ /**< pi2fw opcode */
/* 519 */     OP_pf2iw,          /* &suffix_extensions[21], */ /**< pf2iw opcode */
/* 520 */     OP_pfnacc,         /* &suffix_extensions[22], */ /**< pfnacc opcode */
/* 521 */     OP_pfpnacc,        /* &suffix_extensions[23], */ /**< pfpnacc opcode */
/* 522 */     OP_pswapd,         /* &suffix_extensions[24], */ /**< pswapd opcode */

    /* SSSE3 */
/* 523 */     OP_pshufb,         /* &prefix_extensions[118][0], */ /**< pshufb opcode */
/* 524 */     OP_phaddw,         /* &prefix_extensions[119][0], */ /**< phaddw opcode */
/* 525 */     OP_phaddd,         /* &prefix_extensions[120][0], */ /**< phaddd opcode */
/* 526 */     OP_phaddsw,        /* &prefix_extensions[121][0], */ /**< phaddsw opcode */
/* 527 */     OP_pmaddubsw,      /* &prefix_extensions[122][0], */ /**< pmaddubsw opcode */
/* 528 */     OP_phsubw,         /* &prefix_extensions[123][0], */ /**< phsubw opcode */
/* 529 */     OP_phsubd,         /* &prefix_extensions[124][0], */ /**< phsubd opcode */
/* 530 */     OP_phsubsw,        /* &prefix_extensions[125][0], */ /**< phsubsw opcode */
/* 531 */     OP_psignb,         /* &prefix_extensions[126][0], */ /**< psignb opcode */
/* 532 */     OP_psignw,         /* &prefix_extensions[127][0], */ /**< psignw opcode */
/* 533 */     OP_psignd,         /* &prefix_extensions[128][0], */ /**< psignd opcode */
/* 534 */     OP_pmulhrsw,       /* &prefix_extensions[129][0], */ /**< pmulhrsw opcode */
/* 535 */     OP_pabsb,          /* &prefix_extensions[130][0], */ /**< pabsb opcode */
/* 536 */     OP_pabsw,          /* &prefix_extensions[131][0], */ /**< pabsw opcode */
/* 537 */     OP_pabsd,          /* &prefix_extensions[132][0], */ /**< pabsd opcode */
/* 538 */     OP_palignr,        /* &prefix_extensions[133][0], */ /**< palignr opcode */

    /* SSE4 (incl AMD and Intel-specific extensions */
/* 539 */     OP_popcnt,         /* &second_byte[0xb8], */ /**< popcnt opcode */
/* 540 */     OP_movntss,        /* &prefix_extensions[11][1], */ /**< movntss opcode */
/* 541 */     OP_movntsd,        /* &prefix_extensions[11][3], */ /**< movntsd opcode */
/* 542 */     OP_extrq,          /* &prefix_extensions[134][2], */ /**< extrq opcode */
/* 543 */     OP_insertq,        /* &prefix_extensions[134][3], */ /**< insertq opcode */
/* 544 */     OP_lzcnt,          /* &prefix_extensions[136][1], */ /**< lzcnt opcode */
/* 545 */     OP_pblendvb,       /* &third_byte_38[16], */ /**< pblendvb opcode */
/* 546 */     OP_blendvps,       /* &third_byte_38[17], */ /**< blendvps opcode */
/* 547 */     OP_blendvpd,       /* &third_byte_38[18], */ /**< blendvpd opcode */
/* 548 */     OP_ptest,          /* &third_byte_38[19], */ /**< ptest opcode */
/* 549 */     OP_pmovsxbw,       /* &third_byte_38[20], */ /**< pmovsxbw opcode */
/* 550 */     OP_pmovsxbd,       /* &third_byte_38[21], */ /**< pmovsxbd opcode */
/* 551 */     OP_pmovsxbq,       /* &third_byte_38[22], */ /**< pmovsxbq opcode */
/* 552 */     OP_pmovsxdw,       /* &third_byte_38[23], */ /**< pmovsxdw opcode */
/* 553 */     OP_pmovsxwq,       /* &third_byte_38[24], */ /**< pmovsxwq opcode */
/* 554 */     OP_pmovsxdq,       /* &third_byte_38[25], */ /**< pmovsxdq opcode */
/* 555 */     OP_pmuldq,         /* &third_byte_38[26], */ /**< pmuldq opcode */
/* 556 */     OP_pcmpeqq,        /* &third_byte_38[27], */ /**< pcmpeqq opcode */
/* 557 */     OP_movntdqa,       /* &third_byte_38[28], */ /**< movntdqa opcode */
/* 558 */     OP_packusdw,       /* &third_byte_38[29], */ /**< packusdw opcode */
/* 559 */     OP_pmovzxbw,       /* &third_byte_38[30], */ /**< pmovzxbw opcode */
/* 560 */     OP_pmovzxbd,       /* &third_byte_38[31], */ /**< pmovzxbd opcode */
/* 561 */     OP_pmovzxbq,       /* &third_byte_38[32], */ /**< pmovzxbq opcode */
/* 562 */     OP_pmovzxdw,       /* &third_byte_38[33], */ /**< pmovzxdw opcode */
/* 563 */     OP_pmovzxwq,       /* &third_byte_38[34], */ /**< pmovzxwq opcode */
/* 564 */     OP_pmovzxdq,       /* &third_byte_38[35], */ /**< pmovzxdq opcode */
/* 565 */     OP_pcmpgtq,        /* &third_byte_38[36], */ /**< pcmpgtq opcode */
/* 566 */     OP_pminsb,         /* &third_byte_38[37], */ /**< pminsb opcode */
/* 567 */     OP_pminsd,         /* &third_byte_38[38], */ /**< pminsd opcode */
/* 568 */     OP_pminuw,         /* &third_byte_38[39], */ /**< pminuw opcode */
/* 569 */     OP_pminud,         /* &third_byte_38[40], */ /**< pminud opcode */
/* 570 */     OP_pmaxsb,         /* &third_byte_38[41], */ /**< pmaxsb opcode */
/* 571 */     OP_pmaxsd,         /* &third_byte_38[42], */ /**< pmaxsd opcode */
/* 572 */     OP_pmaxuw,         /* &third_byte_38[43], */ /**< pmaxuw opcode */
/* 573 */     OP_pmaxud,         /* &third_byte_38[44], */ /**< pmaxud opcode */
/* 574 */     OP_pmulld,         /* &third_byte_38[45], */ /**< pmulld opcode */
/* 575 */     OP_phminposuw,     /* &third_byte_38[46], */ /**< phminposuw opcode */
/* 576 */     OP_crc32,          /* &third_byte_38[48], */ /**< crc32 opcode */
/* 577 */     OP_pextrb,         /* &third_byte_3a[2], */ /**< pextrb opcode */
/* 578 */     OP_pextrd,         /* &third_byte_3a[4], */ /**< pextrd opcode */
/* 579 */     OP_extractps,      /* &third_byte_3a[5], */ /**< extractps opcode */
/* 580 */     OP_roundps,        /* &third_byte_3a[6], */ /**< roundps opcode */
/* 581 */     OP_roundpd,        /* &third_byte_3a[7], */ /**< roundpd opcode */
/* 582 */     OP_roundss,        /* &third_byte_3a[8], */ /**< roundss opcode */
/* 583 */     OP_roundsd,        /* &third_byte_3a[9], */ /**< roundsd opcode */
/* 584 */     OP_blendps,        /* &third_byte_3a[10], */ /**< blendps opcode */
/* 585 */     OP_blendpd,        /* &third_byte_3a[11], */ /**< blendpd opcode */
/* 586 */     OP_pblendw,        /* &third_byte_3a[12], */ /**< pblendw opcode */
/* 587 */     OP_pinsrb,         /* &third_byte_3a[13], */ /**< pinsrb opcode */
/* 588 */     OP_insertps,       /* &third_byte_3a[14], */ /**< insertps opcode */
/* 589 */     OP_pinsrd,         /* &third_byte_3a[15], */ /**< pinsrd opcode */
/* 590 */     OP_dpps,           /* &third_byte_3a[16], */ /**< dpps opcode */
/* 591 */     OP_dppd,           /* &third_byte_3a[17], */ /**< dppd opcode */
/* 592 */     OP_mpsadbw,        /* &third_byte_3a[18], */ /**< mpsadbw opcode */
/* 593 */     OP_pcmpestrm,      /* &third_byte_3a[19], */ /**< pcmpestrm opcode */
/* 594 */     OP_pcmpestri,      /* &third_byte_3a[20], */ /**< pcmpestri opcode */
/* 595 */     OP_pcmpistrm,      /* &third_byte_3a[21], */ /**< pcmpistrm opcode */
/* 596 */     OP_pcmpistri,      /* &third_byte_3a[22], */ /**< pcmpistri opcode */

    /* x64 */
/* 597 */     OP_movsxd,         /* &x64_extensions[16][1], */ /**< movsxd opcode */
/* 598 */     OP_swapgs,         /* &rm_extensions[2][0], */ /**< swapgs opcode */

    /* VMX */
/* 599 */     OP_vmcall,         /* &rm_extensions[0][1], */ /**< vmcall opcode */
/* 600 */     OP_vmlaunch,       /* &rm_extensions[0][2], */ /**< vmlaunch opcode */
/* 601 */     OP_vmresume,       /* &rm_extensions[0][3], */ /**< vmresume opcode */
/* 602 */     OP_vmxoff,         /* &rm_extensions[0][4], */ /**< vmxoff opcode */
/* 603 */     OP_vmptrst,        /* &extensions[16][7], */ /**< vmptrst opcode */
/* 604 */     OP_vmptrld,        /* &prefix_extensions[137][0], */ /**< vmptrld opcode */
/* 605 */     OP_vmxon,          /* &prefix_extensions[137][1], */ /**< vmxon opcode */
/* 606 */     OP_vmclear,        /* &prefix_extensions[137][2], */ /**< vmclear opcode */
/* 607 */     OP_vmread,         /* &prefix_extensions[134][0], */ /**< vmread opcode */
/* 608 */     OP_vmwrite,        /* &prefix_extensions[135][0], */ /**< vmwrite opcode */

    /* undocumented */
/* 609 */     OP_int1,           /* &first_byte[0xf1], */ /**< int1 opcode */
/* 610 */     OP_salc,           /* &first_byte[0xd6], */ /**< salc opcode */
/* 611 */     OP_ffreep,         /* &float_high_modrm[7][0x00], */ /**< ffreep opcode */

    /* Keep these at the end so that ifdefs don't change internal enum values */
#ifdef IA32_ON_IA64
/* 612 */     OP_jmpe,       /* &extensions[13][6], */ /**< jmpe opcode */
/* 613 */     OP_jmpe_abs,   /* &second_byte[0xb8], */ /**< jmpe_abs opcode */
#endif

    OP_FIRST = OP_add, /**< First real opcode. */
#ifdef IA32_ON_IA64
    OP_LAST = OP_jmpe_abs, /**< Last real opcode. */
#else
    OP_LAST = OP_ffreep,   /**< Last real opcode. */
#endif
};
#endif

DR_API
/** Returns true iff \p instr's raw bits are a valid encoding of instr. */
bool
instr_raw_bits_valid(instr_t *instr);

/* DR_API EXPORT TOFILE dr_ir_instrlist.h */
DR_UNS_API
/**
 * If the first instr is at Level 0 (i.e., a bundled group of instrs as raw bits),
 * expands it into a sequence of Level 1 instrs using decode_raw() which
 * are added in place to ilist.  Then returns the new first instr.
 */
instr_t*
instrlist_first_expanded(dcontext_t *dcontext, instrlist_t *ilist);

DR_API
/**
 * Assumes that \p instr does not currently have any raw bits allocated.
 * Sets \p instr's raw bits to be \p length bytes starting at \p addr.
 * Does not set the operands invalid.
 */
void
instr_set_raw_bits(instr_t *instr, byte * addr, uint length);

DR_API
/**
 * Allocates \p num_bytes of memory for \p instr's raw bits.
 * If \p instr currently points to raw bits, the allocated memory is
 * initialized with the bytes pointed to.
 * \p instr is then set to point to the allocated memory.
 */
void
instr_allocate_raw_bits(dcontext_t *dcontext, instr_t *instr, uint num_bytes);

DR_API
/**
 * Returns true iff \p instr is a control transfer instruction that takes an
 * 8-bit offset: OP_loop*, OP_jecxz, OP_jmp_short, or OP_jcc_short
 */
#ifdef UNSUPPORTED_API
/**
 * This routine does NOT try to decode an opcode in a Level 1 or Level
 * 0 routine, and can thus be called on Level 0 routines.
 */
#endif
bool
instr_is_cti_short(instr_t *instr);

DR_API
/** Returns true iff \p instr has its own allocated memory for raw bits. */
bool
instr_has_allocated_bits(instr_t *instr);

DR_API
/** Assumes \p opcode is an OP_ constant and sets it to be instr's opcode. */
void
instr_set_opcode(instr_t *instr, int opcode);
#endif
