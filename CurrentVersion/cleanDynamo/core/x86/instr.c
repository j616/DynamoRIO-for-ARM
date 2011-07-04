#include "../globals.h"
#include "instr.h"
#include "arch.h"
#include "../link.h"
#include "decode.h"
#include "decode_fast.h"
#include "instr_create.h"

#include <string.h> /* for memcpy */

#ifdef DEBUG
# include "disassemble.h"
#endif

#ifdef VMX86_SERVER
# include "vmkuw.h" /* VMKUW_SYSCALL_GATEWAY */
#endif

byte *
instr_get_raw_bits(instr_t *instr)
{
	// COMPLETEDD #380 instr_get_raw_bits
	return instr->bytes;
}

#ifdef ARM
// All instructions are of size 4 bytes so this instruction is kind of redundant. But left
// in because of thumb and things
int
instr_length(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #550 instr_length
  return 4;
}
#else
int
instr_length(dcontext_t *dcontext, instr_t *instr)
{
    if (!instr_needs_encoding(instr))
        return instr->length;

    /* hardcode length for cti */
    switch (instr_get_opcode(instr)) {
    case OP_jmp:
    case OP_call:
        return 5;
    case OP_jb: case OP_jnb: case OP_jbe: case OP_jnbe:
    case OP_jl: case OP_jnl: case OP_jle: case OP_jnle:
    case OP_jo: case OP_jno: case OP_jp: case OP_jnp:
    case OP_js: case OP_jns: case OP_jz: case OP_jnz:
        return 6 + ((TEST(PREFIX_JCC_TAKEN, instr_get_prefixes(instr)) ||
                     TEST(PREFIX_JCC_NOT_TAKEN, instr_get_prefixes(instr))) ? 1 : 0);
    case OP_jb_short: case OP_jnb_short: case OP_jbe_short: case OP_jnbe_short:
    case OP_jl_short: case OP_jnl_short: case OP_jle_short: case OP_jnle_short:
    case OP_jo_short: case OP_jno_short: case OP_jp_short: case OP_jnp_short:
    case OP_js_short: case OP_jns_short: case OP_jz_short: case OP_jnz_short:
        return 2 + ((TEST(PREFIX_JCC_TAKEN, instr_get_prefixes(instr)) ||
                     TEST(PREFIX_JCC_NOT_TAKEN, instr_get_prefixes(instr))) ? 1 : 0);
        /* alternative names (e.g., OP_jae_short) are equivalent,
         * so don't need to list them */
    case OP_jmp_short:
        return 2;
    case OP_jecxz:
    case OP_loop:
    case OP_loope:
    case OP_loopne:
        if (opnd_get_reg(instr_get_src(instr, 1)) != REG_XCX)
            return 3; /* need addr prefix */
        else
            return 2;
    case OP_LABEL:
        return 0;
    }

    /* else, encode to find length */
    return private_instr_encode(dcontext, instr, false/*don't need to cache*/);
}
#endif


/* return instr->next */
instr_t*
instr_get_next(instr_t *instr)
{
	// COMPLETEDD #364 instr_get_next
	printf("Starting instr_get_next\n");
	return instr->next;
}

bool
instr_is_exit_cti(instr_t *instr)
{
	return 0;
}

bool
instr_is_ubr(instr_t *instr)      /* unconditional branch */
{
	return 0;
}

/* return instr->prev */
instr_t*
instr_get_prev(instr_t *instr)
{
	// COMPLETEDD #358 instr_get_prev
	printf("Starting instr_get_prev\n");
  return instr->prev;
}

/*** WARNING!  The following rely on ordering of opcodes! ***/

bool
instr_is_cbr(instr_t *instr)      /* conditional branch */
{
	return 0;
}

/* Returns the type of the original indirect branch of an exit
 */
int
instr_exit_branch_type(instr_t *instr)
{
	return 0;
}

DR_API
/* return the taken target pc of the (direct branch) inst */
app_pc
instr_get_branch_target_pc(instr_t *cti_instr)
{
	return 0;
}

DR_API
/* set the taken target pc of the (direct branch) inst */
void
instr_set_branch_target_pc(instr_t *cti_instr, app_pc pc)
{
}

void
instr_set_our_mangling(instr_t *instr, bool ours)
{
	// COMPLETEDD #277 instr_set_our_mangling
	printf("Starting instr_set_our_mangling\n");
  if (ours)
      instr->flags |= INSTR_OUR_MANGLING;
  else
      instr->flags &= ~INSTR_OUR_MANGLING;
}

bool
instr_ok_to_emit(instr_t *instr)
{
	return 0;
}

/* set the note field of instr to value */
void
instr_set_note(instr_t *instr, void *value)
{
}

/* Frees all dynamically allocated storage that was allocated by instr */
void
instr_free(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #377 instr_free
	printf("Starting instr_free\n");
    if ((instr->flags & INSTR_RAW_BITS_ALLOCATED) != 0) {
        heap_free(dcontext, instr->bytes, instr->length HEAPACCT(ACCT_IR));
        instr->bytes = NULL;
        instr->flags &= ~INSTR_RAW_BITS_ALLOCATED;
    }
    if (instr->dsts != NULL) {
        heap_free(dcontext, instr->dsts, instr->num_dsts*sizeof(opnd_t)
                  HEAPACCT(ACCT_IR));
        instr->dsts = NULL;
        instr->num_dsts = 0;
    }
    if (instr->srcs != NULL) {
        /* remember one src is static, rest are dynamic */
        heap_free(dcontext, instr->srcs, (instr->num_srcs-1)*sizeof(opnd_t)
                  HEAPACCT(ACCT_IR));
        instr->srcs = NULL;
        instr->num_srcs = 0;
    }
}

/* deletes the instr_t object with handle "inst" and frees its storage */
void
instr_destroy(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #378 instr_destroy
	printf("Starting instr_destroy\n");
    instr_free(dcontext, instr);

    /* CAUTION: assumes that instr is not part of any instrlist */
    heap_free(dcontext, instr, sizeof(instr_t) HEAPACCT(ACCT_IR));
}

app_pc
instr_get_translation(instr_t *instr)
{
	// COMPLETEDD #361 instr_get_translation
	printf("Starting instr_get_tranlation\n");
	return instr->translation;
}

instr_t *
instr_set_translation(instr_t *instr, app_pc addr)
{
	// COMPLETEDD #362 instr_set_translation
	printf("Starting instr_set_translation\n");
    instr->translation = addr;
    return instr;
}

void
instr_set_next(instr_t *instr, instr_t *next)
{
	// COMPLETEDD #365 instr_set_next
	printf("Starting instr_set_next\n");
  instr->next = next;
}

/* set instr->prev to prev */
void
instr_set_prev(instr_t *instr, instr_t *prev)
{
	// COMPLETEDD #366 instr_set_prev
	printf("Starting instr_set_prev\n");
  instr->prev = prev;
}

/* returns a clone of orig, but with next and prev fields set to NULL */
instr_t *
instr_clone(dcontext_t *dcontext, instr_t *orig)
{
	return 0;
}

/* Returns the pos-th source operand of instr.
 * If instr's operands are not decoded, goes ahead and decodes them.
 * Assumes that instr is a single instr (i.e., NOT Level 0).
 */
opnd_t
instr_get_src(instr_t *instr, uint pos)
{
	opnd_t returnValue;
	return returnValue;
}

bool opnd_is_instr(opnd_t opnd)
{
	return 0;
}

/* return the note field of instr */
void *
instr_get_note(instr_t *instr)
{
	return 0;
}

instr_t *
opnd_get_instr(opnd_t opnd)
{
	return 0;
}

bool opnd_is_far_instr(opnd_t opnd) { return 0; }

ushort
opnd_get_segment_selector(opnd_t opnd)
{
	return 0;
}

/****************************************************************************/
/* utility routines */

void
loginst(dcontext_t *dcontext, uint level, instr_t *instr, char *string)
{
	// COMPLETEDD #372 loginst
    DOLOG(level, LOG_ALL, {
        LOG(THREAD, LOG_ALL, level, "%s: ", string);
        instr_disassemble(dcontext,instr,THREAD);
        LOG(THREAD, LOG_ALL, level,"\n");
    });
}

opnd_t
opnd_create_far_instr(ushort seg_selector, instr_t *instr)
{
	opnd_t returnValue;
	return returnValue;
}

opnd_t
opnd_create_instr(instr_t *instr)
{
	opnd_t returnValue;
	return returnValue;
}

/* Frees all dynamically allocated storage that was allocated by instr,
 * except for allocated raw bits.
 * Also zeroes out instr's fields, except for raw bit fields and next and prev
 * fields, whether instr is ok to mangle, and instr's x86 mode.
 * Use this routine when you want to decode more information into the
 * same instr_t structure.
 * This instr must have been initialized before!
 */
void
instr_reuse(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #382 instr_reuse
	printf("Starting instr_reuse\n");
    byte *bits = NULL;
    uint len = 0;
    bool alloc = false;
    bool mangle = instr_ok_to_mangle(instr);

    instr_t *next = instr->next;
    instr_t *prev = instr->prev;
    if (instr_raw_bits_valid(instr)) {
        if (instr_has_allocated_bits(instr)) {
            /* pretend has no allocated bits to prevent freeing of them */
            instr->flags &= ~INSTR_RAW_BITS_ALLOCATED;
            alloc = true;
        }
        bits = instr->bytes;
        len = instr->length;
    }
    instr_free(dcontext, instr);
    instr_init(dcontext, instr);
    /* now re-add them */
    instr->next = next;
    instr->prev = prev;
    if (bits != NULL) {
        instr->bytes = bits;
        instr->length = len;
        /* assume that the bits are now valid and the operands are not
         * (operand and eflags flags are already unset from init)
         */
        instr->flags |= INSTR_RAW_BITS_VALID;
        if (alloc)
            instr->flags |= INSTR_RAW_BITS_ALLOCATED;
    }
    if (!mangle)
        instr->flags |= INSTR_DO_NOT_MANGLE;
}

/* If instr is not already at the level of decode_opcode, decodes enough
 * from the raw bits pointed to by instr to bring it to that level.
 * Assumes that instr is a single instr (i.e., NOT Level 0).
 *
 * decode_opcode decodes the opcode and eflags usage of the instruction.
 * This corresponds to a Level 2 decoding.
 */
void
instr_decode_opcode(dcontext_t *dcontext, instr_t *instr)
{
	// INPROCESSS instr_decode_opcode
	printf("Starting instr_decode_opcode\n");
//    if (!instr_opcode_valid(instr)) {
//        byte *next_pc;
//        DEBUG_DECLARE(int old_len = instr->length;)
//        CLIENT_ASSERT(instr_raw_bits_valid(instr),
//                      "instr_decode_opcode: raw bits are invalid");
//        instr_reuse(dcontext, instr);
//        next_pc = decode_opcode(dcontext, instr->bytes, instr);
//#ifdef X64
//        set_x86_mode(dcontext, old_mode);
//        /* decode_opcode sets raw bits which invalidates rip_rel, but
//         * it should still be valid on an up-decode of the opcode */
//        if (rip_rel_valid)
//            instr_set_rip_rel_pos(instr, instr->rip_rel_pos);
//#endif
//        /* ok to be invalid, let caller deal with it */
//        CLIENT_ASSERT(next_pc == NULL || (next_pc - instr->bytes == old_len),
//                      "instr_decode_opcode requires a Level 1 or higher instruction");
//    }
}

/* this is sort of a hack, used to allow dynamic reallocation of
 * the trace buffer, which requires shifting the addresses of all
 * the trace Instrs since they point into the old buffer
 */
void
instr_shift_raw_bits(instr_t *instr, ssize_t offs)
{
}

/* These next two functions assume that, if an instr_t has a target
   field, the target is kept in the 0th src location. */
opnd_t
instr_get_target(instr_t *instr)
{
	opnd_t returnValue;
	return returnValue;
}

app_pc
opnd_get_pc(opnd_t opnd)
{
	return 0;
}

/* Calculates the size, in bytes, of the memory read or write of
 * the instr at pc.
 * Returns the pc of the following instr.
 * If the instr at pc does not reference memory, or is invalid,
 * returns NULL.
 */
app_pc
decode_memory_reference_size(dcontext_t *dcontext, app_pc pc, uint *size_in_bytes)
{
	return 0;
}

/*************************
 ***       instr_t       ***
 *************************/

/* returns an empty instr_t object */
instr_t*
instr_create(dcontext_t *dcontext)
{
	  // COMPLETEDD #275 instr_create
	  printf("Starting instr_create\n");
    instr_t *instr = (instr_t*) heap_alloc(dcontext, sizeof(instr_t) HEAPACCT(ACCT_IR));
    /* everything initializes to 0, even flags, to indicate
     * an uninitialized instruction */
    memset((void *)instr, 0, sizeof(instr_t));
    return instr;
}

/* Frees all dynamically allocated storage that was allocated by instr
 * Also zeroes out instr's fields
 * This instr must have been initialized before!
 */
void
instr_reset(dcontext_t *dcontext, instr_t *instr)
{
}

/* Returns true iff instr's opcode is valid.  If the opcode is not
 * OP_INVALID or OP_UNDECODED it is assumed to be valid.  However, calling
 * instr_get_opcode() will attempt to decode an OP_UNDECODED opcode, hence the
 * purpose of this routine.
 */
bool
instr_opcode_valid(instr_t *instr)
{
	// COMPLETEDD #369 instr_opcode_valid
	printf("Starting instr_opcode_valid\n");
    return (instr->opcode != OP_INVALID && instr->opcode != OP_UNDECODED);
}

bool
instr_is_cti(instr_t *instr)      /* any control-transfer instruction */
{
	return 0;
}

int
instr_get_opcode(instr_t *instr)
{
	// INPROCESSS instr_get_opcode
//	printf("Starting instr_get_opcode\n");
//  if (instr->opcode == OP_UNDECODED)
//      instr_decode_opcode(get_thread_private_dcontext(), instr);
//  return instr->opcode;
}

DR_API
/* Emulates instruction to find the address of the index-th memory operand.
 * Either or both OUT variables can be NULL.
 */
bool
instr_compute_address_ex(instr_t *instr, dr_mcontext_t *mc, uint index,
                         OUT app_pc *addr, OUT bool *is_write)
{
	return 0;
}
/* Returns true iff instr is an "undefined" instruction (ud2) */
bool
instr_is_undefined(instr_t *instr)
{
	return 0;
}

void
instr_set_operands_valid(instr_t *instr, bool valid)
{
	// COMPLETEDD #281 instr_set_operands_valid
	printf("Starting instr_set_operands_valid\n");
    if (valid)
        instr->flags |= INSTR_OPERANDS_VALID;
    else
        instr->flags &= ~INSTR_OPERANDS_VALID;
}
/* moves the instruction from USE_ORIGINAL_BITS state to a
 * needs-full-encoding state
 */
void
instr_set_raw_bits_valid(instr_t *instr, bool valid)
{
	// COMPLETEDD #276 instr_set_raw_bits_valid
	printf("Starting instr_set_raw_bits_valid\n");
    if (valid)
        instr->flags |= INSTR_RAW_BITS_VALID;
    else {
        instr->flags &= ~INSTR_RAW_BITS_VALID;
        /* DO NOT set bytes to NULL or length to 0, we still want to be
         * able to point at the original instruction for use in translating
         * addresses for exception/signal handlers
         * Also do not de-allocate allocated bits
         */
    }
}

static inline void
instr_being_modified(instr_t *instr, bool raw_bits_valid)
{
	// COMPLETEDD #278 instr_being_modified
	printf("Starting instr_being_modified\n");
    if (!raw_bits_valid) {
        /* if we're modifying the instr, don't use original bits to encode! */
        instr_set_raw_bits_valid(instr, false);
    }
    /* PR 214962: if client changes our mangling, un-mark to avoid bad translation */
    instr_set_our_mangling(instr, false);
}

bool
instr_operands_valid(instr_t *instr)
{
	// COMPLETEDD #279 instr_operands_valid
	printf("Starting instr_operands_valid\n");
    return ((instr->flags & INSTR_OPERANDS_VALID) != 0);
}

void
instr_set_opcode(instr_t *instr, int opcode)
{
	// COMPLETEDD #280 instr_set_opcode
	printf("Starting instr_set_opcode\n");
    instr->opcode = opcode;
    /* if we're modifying opcode, don't use original bits to encode! */
    instr_being_modified(instr, false/*raw bits invalid*/);
    /* do not assume operands are valid, they are separate from opcode,
     * but if opcode is invalid operands shouldn't be valid
     */
    CLIENT_ASSERT((opcode != OP_INVALID && opcode != OP_UNDECODED) ||
                  !instr_operands_valid(instr),
                  "instr_set_opcode: operand-opcode validity mismatch");
}

/* allocates storage for instr_num_srcs src operands and instr_num_dsts dst operands
 * assumes that instr is currently all zeroed out!
 */
void
instr_set_num_opnds(dcontext_t *dcontext,
                    instr_t *instr, int instr_num_dsts, int instr_num_srcs)
{
	// COMPLETEDD #282 instr_set_num_opnds
	printf("instr_set_num_opnds\n");
    if (instr_num_dsts > 0) {
        CLIENT_ASSERT(instr->num_dsts == 0 && instr->dsts == NULL,
                      "instr_set_num_opnds: dsts are already set");
        CLIENT_ASSERT_TRUNCATE(instr->num_dsts, byte, instr_num_dsts,
                               "instr_set_num_opnds: too many dsts");
        instr->num_dsts = (byte) instr_num_dsts;
        instr->dsts = (opnd_t *) heap_alloc(dcontext, instr_num_dsts*sizeof(opnd_t)
                                          HEAPACCT(ACCT_IR));
    }
    if (instr_num_srcs > 0) {
        /* remember that src0 is static, rest are dynamic */
        if (instr_num_srcs > 1) {
            CLIENT_ASSERT(instr->num_srcs <= 1 && instr->srcs == NULL,
                          "instr_set_num_opnds: srcs are already set");
            instr->srcs = (opnd_t *) heap_alloc(dcontext, (instr_num_srcs-1)*sizeof(opnd_t)
                                              HEAPACCT(ACCT_IR));
        }
        CLIENT_ASSERT_TRUNCATE(instr->num_srcs, byte, instr_num_srcs,
                               "instr_set_num_opnds: too many srcs");
        instr->num_srcs = (byte) instr_num_srcs;
    }
    instr_being_modified(instr, false/*raw bits invalid*/);
    /* assume all operands are valid */
    instr_set_operands_valid(instr, true);
}

instr_t *
instr_build(dcontext_t *dcontext, int opcode, int instr_num_dsts, int instr_num_srcs)
{
	// COMPLETEDD #283 instr_build
	printf("Starting Instr_build\n");
    instr_t *instr = instr_create(dcontext);
    instr_set_opcode(instr, opcode);
    instr_set_num_opnds(dcontext, instr, instr_num_dsts, instr_num_srcs);
    return instr;
}

/* sets the src opnd at position pos in instr */
void
instr_set_src(instr_t *instr, uint pos, opnd_t opnd)
{
	// COMPLETEDD #286 instr_set_src
	  printf("Starting instr_set_src\n");
    CLIENT_ASSERT(pos >= 0 && pos < instr->num_srcs, "instr_set_src: ordinal invalid");
    /* remember that src0 is static, rest are dynamic */
    if (pos == 0)
        instr->src0 = opnd;
    else
        instr->srcs[pos-1] = opnd;
    /* if we're modifying operands, don't use original bits to encode! */
    instr_being_modified(instr, false/*raw bits invalid*/);
    /* assume all operands are valid */
    instr_set_operands_valid(instr, true);
}

/* sets the dst opnd at position pos in instr */
void
instr_set_dst(instr_t *instr, uint pos, opnd_t opnd)
{
	// COMPLETEDD #287 instr_set_dst
	printf("Starting instr_set_dst\n");
    CLIENT_ASSERT(pos >= 0 && pos < instr->num_dsts, "instr_set_dst: ordinal invalid");
    instr->dsts[pos] = opnd;
    /* if we're modifying operands, don't use original bits to encode! */
    instr_being_modified(instr, false/*raw bits invalid*/);
    /* assume all operands are valid */
    instr_set_operands_valid(instr, true);
}

/***********************************************************************
 * instr_t creation routines
 * To use 16-bit data sizes, must call set_prefix after creating instr
 * To support this, all relevant registers must be of eAX form!
 * FIXME: how do that?
 * will an all-operand replacement work, or do some instrs have some
 * var-size regs but some const-size also?
 *
 * FIXME: what if want eflags or modrm info on constructed instr?!?
 *
 * FIXME: pusha/popa/leave/enter/interrupt i_eSP below are left as OPSZ_4_short2
 * but their size really varies...OPSZ_4_short2 is needed now to satisfy the encoder,
 * xref case 10541 on giving these their correct size.
 *
 * fld pushes onto top of stack, call that writing to ST0 or ST7?
 * f*p pops the stack -- not modeled at all!
 * should floating point constants be doubles, not floats?!?
 *
 * opcode complaints:
 * OP_imm vs. OP_st
 * OP_ret: build routines have to separate ret_imm and ret_far_imm
 * others, see FIXME's in instr_create.h
 */
instr_t *
instr_create_0dst_0src(dcontext_t *dcontext, int opcode)
{
	// COMPLETEDD #309 instr_create_0dst_0src
	printf("Starting instr_create_0dst_0src\n");
    instr_t *in = instr_build(dcontext, opcode, 0, 0);
    return in;
}

instr_t *
instr_create_1dst_1src(dcontext_t *dcontext, int opcode,
                       opnd_t dst, opnd_t src)
{
	// COMPLETEDD #288 instr_create_1dst_1src
	printf("Starting instr_create_1dst_1src\n");
    instr_t *in = instr_build(dcontext, opcode, 1, 1);
    instr_set_dst(in, 0, dst);
    instr_set_src(in, 0, src);
    return in;
}

/* zeroes out the fields of instr */
void
instr_init(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #381 instr_init
    /* everything initializes to 0, even flags, to indicate
     * an uninitialized instruction */
    memset((void *)instr, 0, sizeof(instr_t));
}

bool
instr_raw_bits_valid(instr_t *instr)
{
	// COMPLETEDD #373 instr_raw_bits_valid
	printf("Starting instr_raw_bits_valid\n");
    return ((instr->flags & INSTR_RAW_BITS_VALID) != 0);
}

opnd_t
opnd_create_reg(reg_id_t r)
{
	// COMPLETEDD #289 opnd_create_reg
	printf("Starting opnd_create_reg\n");
    opnd_t opnd;
    CLIENT_ASSERT(r < REG_LAST_VALID_ENUM, "opnd_create_reg: invalid register");
    opnd.kind = REG_kind;
    opnd.value.reg = r;
    return opnd;
}

/* Returns true iff instr's opcode is NOT OP_INVALID.
 * Not to be confused with an invalid opcode, which can be OP_INVALID or
 * OP_UNDECODED.  OP_INVALID means an instruction with no valid fields:
 * raw bits (may exist but do not correspond to a valid instr), opcode,
 * eflags, or operands.  It could be an uninitialized
 * instruction or the result of decoding an invalid sequence of bytes.
 */
bool
instr_valid(instr_t *instr)
{
	// COMPLETEDD #371 instr_valid
  return (instr->opcode != OP_INVALID);
}

bool
instr_has_allocated_bits(instr_t *instr)
{
	// COMPLETEDD #379 instr_has_allocated_bits
    return ((instr->flags & INSTR_RAW_BITS_ALLOCATED) != 0);
}

bool
instr_ok_to_mangle(instr_t *instr)
{
	// COMPLETEDD #370 instr_ok_to_mangle
	printf("Starting inst_ok_to_mangle\n");
    return ((instr->flags & INSTR_DO_NOT_MANGLE) == 0);
}

void
instr_set_ok_to_mangle(instr_t *instr, bool val)
{
	// COMPLETEDD #385 instr_set_ok_to_mangle
	printf("Starting instr_set_ok_to_mangle\n");
    if (val)
        instr->flags &= ~INSTR_DO_NOT_MANGLE;
    else
        instr->flags |= INSTR_DO_NOT_MANGLE;
}

bool
instr_is_syscall(instr_t *instr)
{
	return 0;
}

/* Checks whether instr is a jecxz/loop* that was originally an app instruction.
 * All such app instructions are mangled into a jecxz/loop*,jmp_short,jmp sequence.
 * If pc != NULL, pc is expected to point the the beginning of the encoding of
 * instr, and the following instructions are assumed to be encoded in sequence
 * after instr.
 * Otherwise, the encoding is expected to be found in instr's allocated bits.
 * This routine does NOT decode instr to the opcode level.
 */
bool
instr_is_cti_short_rewrite(instr_t *instr, byte *pc)
{
//	// INPROCESSS instr_is_cti_short_rewrite
//    /* ASSUMPTION: all app jecxz/loop* are converted to the pattern
//     * (jecxz/loop*,jmp_short,jmp), and all jecxz/loop* generated by DynamoRIO
//     * DO NOT MATCH THAT PATTERN.
//     *
//     * For clients, I belive we're robust in the presence of a client adding a
//     * pattern that matches ours exactly: decode_fragment() won't think it's an
//     * exit cti if it's in a fine-grained fragment where we have Linkstubs.  Since
//     * bb building marks as non-coarse if a client adds any cti at all (meta or
//     * not), we're protected there.  The other uses of remangle are in perscache,
//     * which is only for coarse once again (coarse in general has a hard time
//     * finding exit ctis: case 8711/PR 213146), and instr_expand(), which shouldn't
//     * be used in the presence of clients w/ bb hooks.
//     * Note that we now help clients make jecxz/loop transformations that look
//     * just like ours: instr_convert_short_meta_jmp_to_long() (PR 266292).
//     */
//	printf("Starting instr_is_cti_short_rewrite\n");
//    if (pc == NULL) {
//        if (!instr_has_allocated_bits(instr) || instr->length != CTI_SHORT_REWRITE_LENGTH)
//            return false;
//        pc = instr_get_raw_bits(instr);
//    }
//    if (instr_opcode_valid(instr)) {
//        int opc = instr_get_opcode(instr);
//        if (opc < OP_loopne || opc > OP_jecxz)
//            return false;
//    }
//    else {
//        /* don't require decoding to opcode level */
//        int raw_opc = (int) *(pc);
//        if (raw_opc < RAW_OPCODE_loop_start || raw_opc > RAW_OPCODE_loop_end)
//            return false;
//    }
//    /* now check remaining undecoded bytes */
//    if (*(pc+2) != decode_first_opcode_byte(OP_jmp_short))
//        return false;
//    if (*(pc+4) != decode_first_opcode_byte(OP_jmp))
//        return false;
//    return true;
	return false;
}

void
instr_free_raw_bits(dcontext_t *dcontext, instr_t *instr)
{
	// COMPLETEDD #383 instr_free_raw_bits
    if ((instr->flags & INSTR_RAW_BITS_ALLOCATED) == 0)
        return;
    heap_free(dcontext, instr->bytes, instr->length HEAPACCT(ACCT_IR));
    instr->flags &= ~INSTR_RAW_BITS_VALID;
    instr->flags &= ~INSTR_RAW_BITS_ALLOCATED;
}

bool
instr_is_cti_short(instr_t *instr)
{
	// INPROCESSS instr_is_cti_short
//    int opc;
//    if (instr_opcode_valid(instr)) /* 1st choice: set opcode */
//        opc = instr_get_opcode(instr);
//    else if (instr_raw_bits_valid(instr)) { /* 2nd choice: 1st byte */
//        /* get raw opcode
//         * FIXME: figure out which callers really rely on us not
//         * up-decoding here -- if nobody then just do the
//         * instr_get_opcode() and get rid of all this
//         */
//        opc = (int) *(instr_get_raw_bits(instr));
//        return (opc == RAW_OPCODE_jmp_short ||
//                (opc >= RAW_OPCODE_jcc_short_start &&
//                 opc <= RAW_OPCODE_jcc_short_end) ||
//                (opc >= RAW_OPCODE_loop_start && opc <= RAW_OPCODE_loop_end));
//    } else /* ok, fine, decode opcode */
//        opc = instr_get_opcode(instr);
//    return (opc == OP_jmp_short ||
//            (opc >= OP_jo_short && opc <= OP_jnle_short) ||
//            (opc >= OP_loopne && opc <= OP_jecxz));
	return 0;
}

/* creates array of bytes to store raw bytes of an instr into
 * (original bits are read-only)
 * initializes array to the original bits!
 */
void
instr_allocate_raw_bits(dcontext_t *dcontext, instr_t *instr, uint num_bytes)
{
	// COMPLETEDD #384 instr_allocate_raw_bits
	printf("Starting instr_allocate_raw_bits\n");
    byte *original_bits = NULL;
    if ((instr->flags & INSTR_RAW_BITS_VALID) != 0)
        original_bits = instr->bytes;
    if ((instr->flags & INSTR_RAW_BITS_ALLOCATED) == 0 ||
        instr->length != num_bytes) {
        byte * new_bits = (byte *) heap_alloc(dcontext, num_bytes HEAPACCT(ACCT_IR));
        if (original_bits != NULL) {
            /* copy original bits into modified bits so can just modify
             * a few and still have all info in one place
             */
            memcpy(new_bits, original_bits,
                   (num_bytes < instr->length) ? num_bytes : instr->length);
        }
        if ((instr->flags & INSTR_RAW_BITS_ALLOCATED) != 0)
            instr_free_raw_bits(dcontext, instr);
        instr->bytes = new_bits;
        instr->length = num_bytes;
    }
    /* assume that the bits are now valid and the operands are not */
    instr->flags |= INSTR_RAW_BITS_VALID;
    instr->flags |= INSTR_RAW_BITS_ALLOCATED;
    instr->flags &= ~INSTR_OPERANDS_VALID;
#ifndef ARM
    instr->flags &= ~INSTR_EFLAGS_VALID;
#else
    instr->flags &= ~INSTR_APSR_VALID;
#endif
}

/* decoding routines */

/* If instr is at Level 0 (i.e., a bundled group of instrs as raw bits),
 * expands instr into a sequence of Level 1 instrs using decode_raw() which
 * are added in place to ilist.
 * Returns the replacement of instr, if any expansion is performed
 * (in which case the old instr is destroyed); otherwise returns
 * instr unchanged.
 * If encounters an invalid instr, stops expanding at that instr, and keeps
 * instr in the ilist pointing to the invalid bits as an invalid instr.
 */
instr_t *
instr_expand(dcontext_t *dcontext, instrlist_t *ilist, instr_t *instr)
{
	// COMPLETEDD #388 instr_expand
    /* Sometimes deleting instr but sometimes not (when return early)
     * is painful -- so we go to the trouble of re-using instr
     * for the first expanded instr
     */
    instr_t *newinstr, *firstinstr = NULL;
    int remaining_bytes, cur_inst_len;
    byte *curbytes, *newbytes;
    IF_X64(bool old_mode;)

    /* make it easy for iterators: handle NULL
     * assume that if opcode is valid, is at Level 2, so not a bundle
     * do not expand meta-instrs -- FIXME: is that the right thing to do?
     */
    if (instr == NULL || instr_opcode_valid(instr) || !instr_ok_to_mangle(instr) ||
        /* if an invalid instr (not just undecoded) do not try to expand */
        !instr_valid(instr))
        return instr;

    DOLOG(4, LOG_ALL, { loginst(dcontext, 4, instr, "instr_expand"); });

    /* decode routines use dcontext mode, but we want instr mode */
    IF_X64(old_mode = set_x86_mode(dcontext, instr_get_x86_mode(instr)));

    /* never have opnds but not opcode */
    CLIENT_ASSERT(!instr_operands_valid(instr), "instr_expand: opnds are already valid");
    CLIENT_ASSERT(instr_raw_bits_valid(instr), "instr_expand: raw bits are invalid");
    curbytes = instr->bytes;
    if ((uint)decode_sizeof(dcontext, curbytes, NULL _IF_X64(NULL)) == instr->length) {
        IF_X64(set_x86_mode(dcontext, old_mode));
        return instr; /* Level 1 */
    }

    remaining_bytes = instr->length;
    while (remaining_bytes > 0) {
        /* insert every separated instr into list */
        newinstr = instr_create(dcontext);
        newbytes = decode_raw(dcontext, curbytes, newinstr);
        if (newbytes == NULL) {
            /* invalid instr -- stop expanding, point instr at remaining bytes */
            instr_set_raw_bits(instr, curbytes, remaining_bytes);
            instr_set_opcode(instr, OP_INVALID);
            if (firstinstr == NULL)
                firstinstr = instr;
            instr_destroy(dcontext, newinstr);
            IF_X64(set_x86_mode(dcontext, old_mode));
            return firstinstr;
        }
        DOLOG(4, LOG_ALL, { loginst(dcontext, 4, newinstr, "\tjust expanded into"); });

        /* CAREFUL of what you call here -- don't call anything that
         * auto-upgrades instr to Level 2, it will fail on Level 0 bundles!
         */

        if (instr_has_allocated_bits(instr) &&
            !instr_is_cti_short_rewrite(newinstr, curbytes)) {
            /* make sure to have our own copy of any allocated bits
             * before we destroy the original instr
             */
            IF_X64(CLIENT_ASSERT(CHECK_TRUNCATE_TYPE_uint(newbytes - curbytes),
                                 "instr_expand: internal truncation error"));
            instr_allocate_raw_bits(dcontext, newinstr, (uint)(newbytes - curbytes));
        }

        /* special case: for cti_short, do not fully decode the
         * constituent instructions, leave as a bundle.
         * the instr will still have operands valid.
         */
        if (instr_is_cti_short_rewrite(newinstr, curbytes)) {
            newbytes = remangle_short_rewrite(dcontext, newinstr, curbytes, 0);
        } else if (instr_is_cti_short(newinstr)) {
            /* make sure non-mangled short ctis, which are generated by
             * us and never left there from app's, are not marked as exit ctis
             */
            instr_set_ok_to_mangle(newinstr, false);
        }

        IF_X64(CLIENT_ASSERT(CHECK_TRUNCATE_TYPE_int(newbytes - curbytes),
                             "instr_expand: internal truncation error"));
        cur_inst_len = (int) (newbytes - curbytes);
        remaining_bytes -= cur_inst_len;
        curbytes = newbytes;

        instrlist_preinsert(ilist, instr, newinstr);
        if (firstinstr == NULL)
            firstinstr = newinstr;
    }

    /* delete original instr from list */
    instrlist_remove(ilist, instr);
    instr_destroy(dcontext, instr);

    CLIENT_ASSERT(firstinstr != NULL, "instr_expand failure");
    IF_X64(set_x86_mode(dcontext, old_mode));
    return firstinstr;
}

/* If the first instr is at Level 0 (i.e., a bundled group of instrs as raw bits),
 * expands it into a sequence of Level 1 instrs using decode_raw() which
 * are added in place to ilist.  Then returns the new first instr.
 */
instr_t *
instrlist_first_expanded(dcontext_t *dcontext, instrlist_t *ilist)
{
	// COMPLETEDD #390 instrlist_first_expanded
	printf("Starting instrlist_first_expanded\n");
    instr_expand(dcontext, ilist, instrlist_first(ilist));
    return instrlist_first(ilist);
}

/* N.B.: this routine sets the "raw bits are valid" flag */
void
instr_set_raw_bits(instr_t *instr, byte *addr, uint length)
{
	  // COMPLETEDD #375 instr_set_raw_bits
	  printf("Starting instr_set_raw_bits\n");
    if ((instr->flags & INSTR_RAW_BITS_ALLOCATED) != 0) {
        /* this does happen, when up-decoding an instr using its
         * own raw bits, so let it happen, but make sure allocated
         * bits aren't being lost
         */
        CLIENT_ASSERT(addr == instr->bytes && length == instr->length,
                      "instr_set_raw_bits: bits already there, but different");
    }
    if (!instr_valid(instr))
        instr_set_opcode(instr, OP_UNDECODED);
    instr->flags |= INSTR_RAW_BITS_VALID;
    instr->bytes = addr;
    instr->length = length;
}
