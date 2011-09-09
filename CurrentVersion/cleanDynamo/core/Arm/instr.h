#ifndef _INSTR_H_
#define _INSTR_H_ 1

#include "../link.h"

/* DR_API EXPORT TOFILE dr_ir_opnd.h */
/* DR_API EXPORT BEGIN */
/****************************************************************************
 * OPERAND ROUTINES
 */
/**
 * @file dr_ir_opnd.h
 * @brief Functions and defines to create and manipulate instruction operands.
 */

typedef byte opnd_size_t; /* contains a REG_ or OPSZ_ enum value */
/* we avoid typedef-ing the enum, as its storage size is compiler-specific */
typedef byte reg_id_t; /* contains a REG_ enum value */

// ADDME Again having trouble with getting the compiler to recognise things from link.h so had
// to recreate the enum here
//enum {
//    /***************************************************/
//    /* these first flags are also used on the instr_t data structure */
//
//    /* Type of branch and thus which struct is used for this exit.
//     * Due to a tight namespace (flags is a ushort field), we pack our
//     * 3 types into these 2 bits, so the LINKSTUB_ macros are used to
//     * distinguish, rather than raw bit tests:
//     *
//     *   name          LINK_DIRECT LINK_INDIRECT  struct
//     *   ---------------  ---         ---         --------------------------
//     *   (subset of fake)  0           0          linkstub_t
//     *   normal direct     1           0          direct_linkstub_t
//     *   normal indirect   0           1          indirect_linkstub_t
//     *   cbr fallthrough   1           1          cbr_fallthrough_linkstub_t
//     *
//     * Note that we can have fake linkstubs that should be treated as
//     * direct or indirect, so LINK_FAKE is a separate flag.
//     */
//    LINK_DIRECT          = 0x0001,
//    LINK_INDIRECT        = 0x0002,
//    /* more specifics on type of branch
//     * must check LINK_DIRECT vs LINK_INDIRECT for JMP and CALL.
//     * absence of all of these is relied on as an indicator of shared_syscall
//     * in indirect_linkstub_target(), so we can't get rid of LINK_RETURN
//     * and use absence of CALL & JMP to indicate it.
//     */
//    LINK_RETURN          = 0x0004,
//    LINK_CALL            = 0x0008,
//    LINK_JMP             = 0x0010,
//    /* if need another flag, could use JMP|CALL to indicate JMP_PLT,
//     * and make sure all testers for CALL also test for !JMP
//     */
//    LINK_IND_JMP_PLT     = 0x0020,
//
//    LINK_SELFMOD_EXIT    = 0x0040,
//#ifdef UNSUPPORTED_API
//    LINK_TARGET_PREFIX   = 0x0080,
//#endif
//#ifdef X64
//    /* PR 257963: since we don't store targets of ind branches, we need a flag
//     * so we know whether this is a trace cmp exit, which has its own ibl entry
//     */
//    LINK_TRACE_CMP       = 0x0100,
//#endif
//#ifdef WINDOWS
//    LINK_CALLBACK_RETURN = 0x0200,
//#else
//    /* PR 286922: we support both OP_sys{call,enter}- and OP_int-based system calls */
//    LINK_NI_SYSCALL_INT  = 0x0200,
//#endif
//    /* indicates whether exit is before a non-ignorable syscall */
//    LINK_NI_SYSCALL      = 0x0400,
//    LINK_FINAL_INSTR_SHARED_FLAG = LINK_NI_SYSCALL,
//    /* end of instr_t-shared flags  */
//    /***************************************************/
//
//    LINK_FRAG_OFFS_AT_END= 0x0800,
//
//    LINK_END_OF_LIST     = 0x1000,
//
//    LINK_FAKE            = 0x2000,
//
//    LINK_LINKED          = 0x4000,
//
//    LINK_SEPARATE_STUB   = 0x8000,
//
//    /* WARNING: flags field is a ushort, so max flag is 0x8000! */
//};

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

DR_API
/** Returns true iff \p instr's operands are up to date. */
bool
instr_operands_valid(instr_t *instr);

#ifndef ARM
#define REG_LAST_ENUM     REG_INVALID /**< Last register enum value. */

/* DR_API EXPORT END */

#define INT8_MIN   SCHAR_MIN
#define INT8_MAX   SCHAR_MAX
#define INT16_MIN  SHRT_MIN
#define INT16_MAX  SHRT_MAX
#define INT32_MIN  INT_MIN
#define INT32_MAX  INT_MAX
/* DR_API EXPORT BEGIN */
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

/* DR_API EXPORT END */


#ifndef ARM
/* length of our mangling of jecxz/loop* */
#define CTI_SHORT_REWRITE_LENGTH 9

DR_API
/** Returns an immediate float operand with value \p f. */
opnd_t
opnd_create_immed_float(float f);

DR_API
/**
 * Assumes that \p reg is a REG_ 32-bit register constant.
 * Returns the 16-bit version of \p reg.
 */
reg_id_t
reg_32_to_16(reg_id_t reg);
#else
#define CTI_SHORT_REWRITE_LENGTH 4
#endif

DR_API
/** Returns an empty operand. */
opnd_t
opnd_create_null(void);

DR_API
/** Returns a register operand (\p r must be a REG_ constant). */
opnd_t
opnd_create_reg(reg_id_t r);

DR_API
/**
 * Returns an immediate integer operand with value \p i and size
 * \p data_size; \p data_size must be a OPSZ_ constant.
 */
opnd_t
opnd_create_immed_int(ptr_int_t i, opnd_size_t data_size);

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

#ifdef ARM
    uint 		 conditionalBits;
    uint		 apsr;
#else
    uint    eflags;   /* contains EFLAGS_ bits, but amount of info varies
                       * depending on how instr was decoded/built */
    uint    prefixes; /* data size, addr size, or lock prefix info */
#endif

    /* this field is for the use of passes as an annotation.
     * it is also used to hold the offset of an instruction when encoding
     * pc-relative instructions.
     */
    void *note;

    /* fields for building instructions into instruction lists */
    instr_t   *prev;
    instr_t   *next;
};

/* DR_API EXPORT TOFILE dr_ir_instr.h */

/* functions to inspect and manipulate the fields of an instr_t
 * NB: a number of instr_ routines are delared in arch_exports.h.
 */

#ifndef ARM
/* Sets instr's eflags to be valid if valid is true, invalid otherwise. */
void
instr_set_eflags_valid(instr_t *instr, bool valid);
#else
void instr_set_apsr_valid(instr_t *instr, bool valid);
#endif

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
 * Assumes that \p instr has been initialized but does not have any
 * operands yet.  Allocates storage for \p num_srcs source operands
 * and \p num_dsts destination operands.
 */
void
instr_set_num_opnds(dcontext_t *dcontext, instr_t *instr, int num_dsts, int num_srcs);

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

# define LINK_NI_SYSCALL_ALL (LINK_NI_SYSCALL | LINK_NI_SYSCALL_INT)
/* flags */
enum {

  /* these first flags are shared with the LINK_ flags and are
   * used to pass on info to link stubs
   */
  /* used to determine type of indirect branch for exits */
  INSTR_DIRECT_EXIT           = LINK_DIRECT,
  INSTR_INDIRECT_EXIT         = LINK_INDIRECT,
  INSTR_RETURN_EXIT           = LINK_RETURN,
  INSTR_CALL_EXIT             = LINK_CALL,
  INSTR_JMP_EXIT              = LINK_JMP,
  /* marks an indirect jmp preceded by a call (== a PLT-style ind call) */
  INSTR_IND_JMP_PLT_EXIT      = LINK_IND_JMP_PLT,
  INSTR_BRANCH_SELFMOD_EXIT   = LINK_SELFMOD_EXIT,
#ifdef UNSUPPORTED_API
  INSTR_BRANCH_TARGETS_PREFIX = LINK_TARGET_PREFIX,
#endif
#ifdef X64
  /* PR 257963: since we don't store targets of ind branches, we need a flag
   * so we know whether this is a trace cmp exit, which has its own ibl entry
   */
  INSTR_TRACE_CMP_EXIT        = LINK_TRACE_CMP,
#endif
#ifdef WINDOWS
  INSTR_CALLBACK_RETURN       = LINK_CALLBACK_RETURN,
#else
  INSTR_NI_SYSCALL_INT        = LINK_NI_SYSCALL_INT,
#endif
  INSTR_NI_SYSCALL            = LINK_NI_SYSCALL,
  INSTR_NI_SYSCALL_ALL        = LINK_NI_SYSCALL_ALL,
  /* meta-flag */
  EXIT_CTI_TYPES              = (INSTR_DIRECT_EXIT | INSTR_INDIRECT_EXIT |
                                 INSTR_RETURN_EXIT | INSTR_CALL_EXIT |
                                 INSTR_JMP_EXIT | INSTR_IND_JMP_PLT_EXIT |
                                 INSTR_BRANCH_SELFMOD_EXIT |
                                 INSTR_NI_SYSCALL_INT |
                                 INSTR_NI_SYSCALL),
    INSTR_RAW_BITS_VALID        = 0x00080000,
    INSTR_OPERANDS_VALID        = 0x00010000,
#ifndef ARM
    INSTR_EFLAGS_VALID          = 0x00020000,
    INSTR_EFLAGS_6_VALID				= 0x00040000,
#else
    INSTR_APSR_VALID        		= 0x00020000,
    INSTR_APSR_5_VALID					= 0x00040000,
#endif
    /* Currently used for frozen coarse fragments with final jmps and
     * jmps to ib stubs that are elided: we need the jmp instr there
     * to build the linkstub_t but we do not want to emit it. */
    INSTR_DO_NOT_EMIT           = 0x10000000,
    INSTR_RAW_BITS_ALLOCATED    = 0x00100000,
    INSTR_DO_NOT_MANGLE         = 0x00200000,
    INSTR_OUR_MANGLING          = 0x80000000,
};

/* DR_API EXPORT TOFILE dr_ir_opcodes.h */
/* DR_API EXPORT BEGIN */

/****************************************************************************
 * OPCODES
 */
/**
 * @file dr_ir_opcodes.h
 * @brief Instruction opcode constants.
 */

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
/* 44 */ OP_SVC,
/* 45 */ OP_NULL,
				OP_FIRST = OP_AND, /**< First real opcode. */
				OP_LAST = OP_SVC,   /**< Last real opcode. */
};

/* DR_API EXPORT END */

typedef struct
{
  int catergoryNumber;
  unsigned int concernedBits;
    unsigned int maskedBits;
} InstructionCatergory;

DR_UNS_API
/**
 * If instr is not already fully decoded, decodes enough
 * from the raw bits pointed to by instr to bring it Level 3.
 * Assumes that instr is a single instr (i.e., NOT Level 0).
 */
void
instr_decode(dcontext_t *dcontext, instr_t *instr);

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

DR_API
/** Sets \p instr's raw bits to be valid if \p valid is true, invalid otherwise. */
void
instr_set_raw_bits_valid(instr_t *instr, bool valid);

DR_API
/** Sets \p instr's operands to be valid if \p valid is true, invalid otherwise. */
void
instr_set_operands_valid(instr_t *instr, bool valid);
#endif
