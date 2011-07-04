#include "../globals.h"
#include "decode_fast.h"
#include "../link.h"
#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "disassemble.h"


byte *
decode_cti(dcontext_t *dcontext, byte *pc, instr_t *instr)
{
	return 0;
}


int
decode_sizeof(dcontext_t *dcontext, byte *start_pc, int *num_prefixes
              _IF_X64(uint *rip_rel_pos))
{
	// COMPLETEDD #374 decode_sizeof
	// Note this will have to change for Thumb instructions
	printf("Asking size of instruction to be 4 as all Arm Instructions\n");
	return 4;
}

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
