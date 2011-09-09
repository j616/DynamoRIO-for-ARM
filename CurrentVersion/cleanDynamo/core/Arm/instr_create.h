#ifndef _INSTR_CREATE_H_
#define _INSTR_CREATE_H_ 1

/* DR_API EXPORT TOFILE dr_ir_macros.h */
/* DR_API EXPORT BEGIN */
/**
 * @file dr_ir_macros.h
 * @breif Instruction creation convenience macros.
 */

/**
 * Creates an instr_t with opcode OP_LABEL.  An OP_LABEL instruction can be used as a
 * jump or call instr_t target, and when emitted it will take no space in the
 * resulting machine code.
 * \param dc The void * dcontext used to allocate memory for the instr_t.
 */
#define INSTR_CREATE_label(dc)    instr_create_0dst_0src((dc), OP_LABEL)
#ifndef ARM
#define INSTR_CREATE_mov_ld(dc, d, s) \
  instr_create_1dst_1src((dc), OP_mov_ld, (d), (s))
#endif
#endif

/* DR_API EXPORT END */
