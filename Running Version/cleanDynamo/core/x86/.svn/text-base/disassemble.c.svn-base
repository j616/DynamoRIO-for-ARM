#include "../globals.h"
#include "arch.h"
#include "instr.h"
#include "decode.h"
#include "decode_fast.h"
#include "disassemble.h"
#include <string.h>

/* these are only needed for symbolic address lookup: */
#include "../fragment.h" /* for fragment_pclookup */
#include "../link.h" /* for linkstub lookup */

#include "../fcache.h" /* for in_fcache */

/*
 * Prints the instruction instr to file outfile.
 * Does not print addr16 or data16 prefixes for other than just-decoded instrs,
 * and does not check that the instruction has a valid encoding.
 * Prints each operand with leading zeros indicating the size.
 */
void
instr_disassemble(dcontext_t *dcontext, instr_t *instr, file_t outfile)
{
	// INPROCESSS instr_disassemble
	printf("INSTRUNCTION DISASSEMBLE STILL TO DO\n");
}
void
disassemble_fragment_body(dcontext_t *dcontext, fragment_t *f, file_t outfile)
{
}

void
disassemble_app_bb(dcontext_t *dcontext, app_pc tag, file_t outfile)
{
}

void
dump_dr_callstack(file_t outfile)
{
	// INPROCESSS dump_dr_callstack
	printf("Built by Stephen Barton: dump_dr_callstack\n");
}

/* Disassembles a single instruction, optionally printing its pc (if show_pc)
 * and its raw bytes (show_bytes) beforehand.
 * Returns the pc of the next instruction.
 * FIXME: vs disassemble_with_bytes -- didn't want to update all callers
 * so leaving, though should probably remove.
 * Returns NULL if the instruction at pc is invalid.
 */
byte *
disassemble_with_info(dcontext_t *dcontext, byte *pc, file_t outfile,
                      bool show_pc, bool show_bytes)
{
    return 0;
}
