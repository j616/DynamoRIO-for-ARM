#include "../globals.h"
#include "arch.h"
#include "../link.h"
#include "../fragment.h"
#include "../instrlist.h"
#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "decode_fast.h"
#include "disassemble.h"
#include "instrument.h" /* for dr_insert_call */
#include <sys/syscall.h>

void
finalize_selfmod_sandbox(dcontext_t *dcontext, fragment_t *f)
{
}

/* If skip is false:
 *   changes the jmp right before the next syscall (after pc) to target the
 *   exit cti immediately following it;
 * If skip is true:
 *   changes back to the default, where skip hops over the exit cti,
 *   which is assumed to be located at pc.
 */
bool
mangle_syscall_code(dcontext_t *dcontext, fragment_t *f, byte *pc, bool skip)
{
	return 0;
}

/* TOP-LEVEL MANGLE
 * This routine is responsible for mangling a fragment into the form
 * we'd like prior to placing it in the code cache
 * If mangle_calls is false, ignores calls
 * If record_translation is true, records translation target for each
 * inserted instr -- but this slows down encoding in current implementation
 */
void
mangle(dcontext_t *dcontext, instrlist_t *ilist, uint flags,
       bool mangle_calls, bool record_translation)
{
}

/* For jecxz and loop*, we create 3 instructions in a single
 * instr that we treat like a single conditional branch.
 * On re-decoding our own output we need to recreate that instr.
 * This routine assumes that the instructions encoded at pc
 * are indeed a mangled cti short.
 * Assumes that the first instr has already been decoded into instr,
 * that pc points to the start of that instr.
 * Converts instr into a new 3-raw-byte-instr with a private copy of the
 * original raw bits.
 * Optionally modifies the target to "target" if "target" is non-null.
 * Returns the pc of the instruction after the remangled sequence.
 */
byte *
remangle_short_rewrite(dcontext_t *dcontext,
                       instr_t *instr, byte *pc, app_pc target)
{
	// INPROCESSS remangle_short_rewrite
//  ASSERT(instr_is_cti_short_rewrite(instr, pc));
//
//  /* first set the target in the actual operand src0 */
//  if (target == NULL) {
//      /* acquire existing absolute target */
//      int rel_target = *((int *)(pc+5));
//      target = pc + CTI_SHORT_REWRITE_LENGTH + rel_target;
//  }
//  instr_set_target(instr, opnd_create_pc(target));
//  /* now set up the bundle of raw instructions
//   * we've already read the first 2-byte instruction, jecxz/loop*
//   * they all take up CTI_SHORT_REWRITE_LENGTH bytes
//   */
//  instr_allocate_raw_bits(dcontext, instr, CTI_SHORT_REWRITE_LENGTH);
//  instr_set_raw_bytes(instr, pc, CTI_SHORT_REWRITE_LENGTH);
//  /* for x64 we may not reach, but we go ahead and try */
//  instr_set_raw_word(instr, 5, (int)(target - (pc + CTI_SHORT_REWRITE_LENGTH)));
//  /* now make operands valid */
//  instr_set_operands_valid(instr, true);
//  return (pc+CTI_SHORT_REWRITE_LENGTH);
	return 0;
}

static void
sandbox_top_of_bb(dcontext_t *dcontext, instrlist_t *ilist,
                  bool s2ro, uint flags,
                  app_pc start_pc, app_pc end_pc, /* end is open */
                  bool for_cache,
                  /* for obtaining the two patch locations: */
                  patch_list_t *patchlist,
                  cache_pc *copy_start_loc, cache_pc *copy_end_loc)
{
	printf("SANDBOX_TOP_OF_BB STILL NEED TO DO THIS\n");
//	// INPROCESSS sandbox_top_of_bb
//    /* add check at top of ilist that compares actual app instructions versus
//     * copy we saved, stored in cache right after fragment itself.  leave its
//     * start address blank here, will be touched up after emitting this ilist.
//     *
//     * FIXME case 8165/PR 212600: optimize this: move reg restores to
//     * custom fcache_return, use cmpsd instead of cmpsb, etc.
//     *
//     * if eflags live entering this bb:
//     *   save xax
//     *   lahf
//     *   seto  %al
//     * endif
//     * if (-sandbox2ro_threshold > 0)
//     *  if x64: save xcx
//     *     incl  &vm_area_t->exec_count (for x64, via xcx)
//     *     cmp   sandbox2ro_threshold, vm_area_t->exec_count (for x64, via xcx)
//     *  if eflags live entering this bb, or x64:
//     *     jl    past_threshold
//     *   if x64: restore xcx
//     *   if eflags live entering this bb:
//     *     jmp restore_eflags_and_exit
//     *   else
//     *     jmp   start_pc marked as selfmod exit
//     *   endif
//     *   past_threshold:
//     *  else
//     *     jge   start_pc marked as selfmod exit
//     *  endif
//     * endif
//     * if (-sandbox2ro_threshold == 0) && !x64)
//     *   save xcx
//     * endif
//     *   save xsi
//     *   save xdi
//     * if stats:
//     *   inc num_sandbox_execs stat (for x64, via xsi)
//     * endif
//     *   mov start_pc,xsi
//     *   mov copy_start_pc,xdi  # 1 opcode byte, then offset
//     *       # => patch point 1
//     *   cmpsb
//     * if copy_size > 1 # not an opt: for correctness: if "repe cmpsb" has xcx==0, it
//     *                  # doesn't touch eflags and we treat cmp results as cmpsb results
//     *     jne check_results
//     *   if x64 && start_pc > 4GB
//     *     mov start_pc, xcx
//     *     cmp xsi, xcx
//     *   else
//     *     cmp xsi, start_pc
//     *   endif
//     *     mov copy_size-1, xcx
//     *     jge forward
//     *     mov copy_end_pc, xdi
//     *         # => patch point 2
//     *     mov end_pc, xsi
//     *   forward:
//     *     repe cmpsb
//     * endif # copy_size > 1
//     *   check_results:
//     *     restore xcx
//     *     restore xsi
//     *     restore xdi
//     * if eflags live:
//     *   je start_bb
//     *  restore_eflags_and_exit:
//     *   add   $0x7f,%al
//     *   sahf
//     *   restore xax
//     *   jmp start_pc marked as selfmod exit
//     * else
//     *   jne start_pc marked as selfmod exit
//     * endif
//     * start_bb:
//     * if eflags live:
//     *   add   $0x7f,%al
//     *   sahf
//     *   restore xax
//     * endif
//     */
//    instr_t *instr, *jmp;
//    instr_t *restore_eflags_and_exit = NULL;
//    // INPROCESSS Another TLS reference
//    bool use_tls = false;
////    bool use_tls = IF_X64_ELSE(true, false);
//    bool saved_xcx = false;
//    instr_t *check_results = INSTR_CREATE_label(dcontext);
//
//    instr = instrlist_first_expanded(dcontext, ilist);
//
//    insert_save_eflags(dcontext, ilist, instr, flags, use_tls, !use_tls);
//
//    if (s2ro) {
//        /* It's difficult to use lea/jecxz here as we want to use a shared
//         * counter but no lock, and thus need a relative comparison, while
//         * lea/jecxz can only do an exact comparison.  We could be exact by
//         * having a separate counter per (private) fragment but by spilling
//         * eflags we can inc memory, making the scheme here not inefficient.
//         */
//        uint thresh = DYNAMO_OPTION(sandbox2ro_threshold);
//        uint *counter;
//        if (for_cache)
//            counter = get_selfmod_exec_counter(start_pc);
//        else {
//            /* Won't find exec area since not a real fragment (probably
//             * a recreation post-flush).  Won't execute, so NULL is fine.
//             */
//            counter = NULL;
//        }
//#ifdef X64
//        PRE(ilist, instr,
//            SAVE_TO_DC_OR_TLS(dcontext, REG_XCX, TLS_XCX_SLOT, XCX_OFFSET));
//        saved_xcx = true;
//        PRE(ilist, instr,
//            INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XCX),
//                                 OPND_CREATE_INTPTR(counter)));
//        PRE(ilist, instr,
//            INSTR_CREATE_inc(dcontext, OPND_CREATE_MEM32(REG_XCX, 0)));
//        PRE(ilist, instr,
//            INSTR_CREATE_cmp(dcontext, OPND_CREATE_MEM32(REG_XCX, 0),
//                             OPND_CREATE_INT_32OR8((int)thresh)));
//#else
//        PRE(ilist, instr,
//            INSTR_CREATE_inc(dcontext, OPND_CREATE_ABSMEM(counter, OPSZ_4)));
//        PRE(ilist, instr,
//            INSTR_CREATE_cmp(dcontext,
//                             OPND_CREATE_ABSMEM(counter, OPSZ_4),
//                             OPND_CREATE_INT_32OR8(thresh)));
//#endif
//        if (TEST(FRAG_WRITES_EFLAGS_6, flags) IF_X64(&& false)) {
//            jmp = INSTR_CREATE_jcc(dcontext, OP_jge, opnd_create_pc(start_pc));
//            instr_branch_set_selfmod_exit(jmp, true);
//            /* an exit cti, not a meta instr */
//            instrlist_preinsert(ilist, instr, jmp);
//        } else {
//            instr_t *past_threshold = INSTR_CREATE_label(dcontext);
//            PRE(ilist, instr,
//                INSTR_CREATE_jcc_short(dcontext, OP_jl_short,
//                                       opnd_create_instr(past_threshold)));
//#ifdef X64
//            PRE(ilist, instr,
//                RESTORE_FROM_DC_OR_TLS(dcontext, REG_XCX, TLS_XCX_SLOT, XCX_OFFSET));
//#endif
//            if (!TEST(FRAG_WRITES_EFLAGS_6, flags)) {
//                ASSERT(restore_eflags_and_exit == NULL);
//                restore_eflags_and_exit = INSTR_CREATE_label(dcontext);
//                PRE(ilist, instr, INSTR_CREATE_jmp
//                    (dcontext, opnd_create_instr(restore_eflags_and_exit)));
//            }
//#ifdef X64
//            else {
//                jmp = INSTR_CREATE_jmp(dcontext, opnd_create_pc(start_pc));
//                instr_branch_set_selfmod_exit(jmp, true);
//                /* an exit cti, not a meta instr */
//                instrlist_preinsert(ilist, instr, jmp);
//            }
//#endif
//            PRE(ilist, instr, past_threshold);
//        }
//    }
//
//    if (!saved_xcx) {
//        PRE(ilist, instr,
//            SAVE_TO_DC_OR_TLS(dcontext, REG_XCX, TLS_XCX_SLOT, XCX_OFFSET));
//    }
//    PRE(ilist, instr,
//        SAVE_TO_DC_OR_TLS(dcontext, REG_XSI, TLS_XBX_SLOT, XSI_OFFSET));
//    PRE(ilist, instr,
//        SAVE_TO_DC_OR_TLS(dcontext, REG_XDI, TLS_XDX_SLOT, XDI_OFFSET));
//    DOSTATS({
//        if (GLOBAL_STATS_ON()) {
//            /* We only do global inc, not bothering w/ thread-private stats.
//             * We don't care about races: ballpark figure is good enough.
//             * We could do a direct inc of memory for 32-bit.
//             */
//            PRE(ilist, instr, INSTR_CREATE_mov_imm
//                (dcontext, opnd_create_reg(REG_XSI),
//                 OPND_CREATE_INTPTR(GLOBAL_STAT_ADDR(num_sandbox_execs))));
//            PRE(ilist, instr, INSTR_CREATE_inc
//                (dcontext, opnd_create_base_disp(REG_XSI, REG_NULL, 0, 0, OPSZ_STATS)));
//         }
//    });
//    PRE(ilist, instr,
//        INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XSI),
//                             OPND_CREATE_INTPTR(start_pc)));
//    PRE(ilist, instr,
//        INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XDI),
//                             /* will become copy start */
//                             OPND_CREATE_INTPTR(start_pc)));
//    if (patchlist != NULL) {
//        ASSERT(copy_start_loc != NULL);
//        add_patch_marker(patchlist, instr_get_prev(instr), PATCH_ASSEMBLE_ABSOLUTE,
//                         -(short)sizeof(cache_pc), (ptr_uint_t*)copy_start_loc);
//    }
//    PRE(ilist, instr, INSTR_CREATE_cmps_1(dcontext));
//    /* For a 1-byte copy size we cannot use "repe cmpsb" as it won't
//     * touch eflags and we'll treat the cmp results as cmpsb results, which
//     * doesn't work (cmp will never be equal)
//     */
//    if (end_pc - start_pc > 1) {
//        instr_t *forward = INSTR_CREATE_label(dcontext);
//        PRE(ilist, instr,
//            INSTR_CREATE_jcc(dcontext, OP_jne, opnd_create_instr(check_results)));
//#ifdef X64
//        if ((ptr_uint_t)start_pc > UINT_MAX) {
//            PRE(ilist, instr,
//                INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XCX),
//                                     OPND_CREATE_INTPTR(start_pc)));
//            PRE(ilist, instr,
//                INSTR_CREATE_cmp(dcontext, opnd_create_reg(REG_XSI),
//                                 opnd_create_reg(REG_XCX)));
//        } else {
//#endif
//            PRE(ilist, instr,
//                INSTR_CREATE_cmp(dcontext, opnd_create_reg(REG_XSI),
//                                 OPND_CREATE_INT32((int)(ptr_int_t)start_pc)));
//#ifdef X64
//        }
//#endif
//        PRE(ilist, instr,
//            INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XCX),
//                                 OPND_CREATE_INTPTR(end_pc - (start_pc + 1))));
//        PRE(ilist, instr,
//            INSTR_CREATE_jcc(dcontext, OP_jge, opnd_create_instr(forward)));
//        PRE(ilist, instr,
//            INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XDI),
//                                 /* will become copy end */
//                                 OPND_CREATE_INTPTR(end_pc)));
//        if (patchlist != NULL) {
//            ASSERT(copy_end_loc != NULL);
//            add_patch_marker(patchlist, instr_get_prev(instr), PATCH_ASSEMBLE_ABSOLUTE,
//                             -(short)sizeof(cache_pc), (ptr_uint_t*)copy_end_loc);
//        }
//        PRE(ilist, instr,
//            INSTR_CREATE_mov_imm(dcontext, opnd_create_reg(REG_XSI),
//                                 OPND_CREATE_INTPTR(end_pc)));
//        PRE(ilist, instr, forward);
//        PRE(ilist, instr, INSTR_CREATE_rep_cmps_1(dcontext));
//    }
//    PRE(ilist, instr, check_results);
//    PRE(ilist, instr,
//        RESTORE_FROM_DC_OR_TLS(dcontext, REG_XCX, TLS_XCX_SLOT, XCX_OFFSET));
//    PRE(ilist, instr,
//        RESTORE_FROM_DC_OR_TLS(dcontext, REG_XSI, TLS_XBX_SLOT, XSI_OFFSET));
//    PRE(ilist, instr,
//        RESTORE_FROM_DC_OR_TLS(dcontext, REG_XDI, TLS_XDX_SLOT, XDI_OFFSET));
//    if (!TEST(FRAG_WRITES_EFLAGS_6, flags)) {
//        instr_t *start_bb = INSTR_CREATE_label(dcontext);
//        PRE(ilist, instr,
//            INSTR_CREATE_jcc(dcontext, OP_je, opnd_create_instr(start_bb)));
//        if (restore_eflags_and_exit != NULL) /* somebody needs this label */
//            PRE(ilist, instr, restore_eflags_and_exit);
//        insert_restore_eflags(dcontext, ilist, instr, flags, use_tls, !use_tls);
//        jmp = INSTR_CREATE_jmp(dcontext, opnd_create_pc(start_pc));
//        instr_branch_set_selfmod_exit(jmp, true);
//        /* an exit cti, not a meta instr */
//        instrlist_preinsert(ilist, instr, jmp);
//        PRE(ilist, instr, start_bb);
//    } else {
//        jmp = INSTR_CREATE_jcc(dcontext, OP_jne, opnd_create_pc(start_pc));
//        instr_branch_set_selfmod_exit(jmp, true);
//        /* an exit cti, not a meta instr */
//        instrlist_preinsert(ilist, instr, jmp);
//    }
//    insert_restore_eflags(dcontext, ilist, instr, flags, use_tls, !use_tls);
//    /* fall-through to bb start */
}

/* Offsets within selfmod sandbox top-of-bb code that we patch once
 * the code is emitted, as the values depend on the emitted address.
 * These vary by whether sandbox_top_of_bb_check_s2ro() and whether
 * eflags are not written, all written, or just OF is written.
 * For the copy_size == 1 variation, we simply ignore the 2nd patch point.
 */
static bool selfmod_s2ro[] = { false, true };
#define SELFMOD_NUM_S2RO   (sizeof(selfmod_s2ro)/sizeof(selfmod_s2ro[0]))

#ifndef ARM
static uint selfmod_eflags[] = { FRAG_WRITES_EFLAGS_6, FRAG_WRITES_EFLAGS_OF, 0 };
#define SELFMOD_NUM_EFLAGS (sizeof(selfmod_eflags)/sizeof(selfmod_eflags[0]))
#else
static uint selfmod_apsr[] = { FRAG_WRITES_APSR_5, FRAG_WRITES_APSR_Q, 0 };
#define SELFMOD_NUM_APSR (sizeof(selfmod_apsr) / sizeof(selfmod_apsr[0]))
#endif

void
set_selfmod_sandbox_offsets(dcontext_t *dcontext)
{
	// INPROCESSS set_selfmod_sandbox_offsets
	printf("WARNING STARTING SET_SELFMOD_SANDBOX_OFFSETS. STILL NEED TO DO\n");
//	printf("Starting set_selfmod_sandbox_offsets");
//    int i, j;
//    instrlist_t ilist;
//    patch_list_t patch;
//    static byte buf[256];
//    uint len;
//    /* We assume this is called at init, when .data is +w and we need no
//     * synch on accessing buf */
//    ASSERT(!dynamo_initialized);
//    for (i = 0; i < SELFMOD_NUM_S2RO; i++) {
//#ifndef ARM
//    	for (j = 0; j < SELFMOD_NUM_EFLAGS; j++) {
//#else
//    	for (j = 0; j < SELFMOD_NUM_APSR; j++) {
//#endif
//                cache_pc start_pc, end_pc;
//                app_pc app_start;
//                instrlist_init(&ilist);
//                /* sandbox_top_of_bb assumes there's an instr there */
//                instrlist_append(&ilist, INSTR_CREATE_label(dcontext));
//                init_patch_list(&patch, PATCH_TYPE_ABSOLUTE);
//                app_start = IF_X64_ELSE(selfmod_gt4G[k], NULL);
//#ifndef ARM
//                sandbox_top_of_bb(dcontext, &ilist,
//                                  selfmod_s2ro[i], selfmod_eflags[j],
//                                  /* we must have a >1-byte region to get
//                                   * both patch points */
//                                  app_start, app_start + 2, false,
//                                  &patch, &start_pc, &end_pc);
//#else
//                sandbox_top_of_bb(dcontext, &ilist,
//                                  selfmod_s2ro[i], selfmod_apsr[j],
//                                  /* we must have a >1-byte region to get
//                                   * both patch points */
//                                  app_start, app_start + 2, false,
//                                  &patch, &start_pc, &end_pc);
//#endif
////                len = encode_with_patch_list(dcontext, &patch, &ilist, buf);
////                ASSERT(len < BUFFER_SIZE_BYTES(buf));
////                IF_X64(ASSERT(CHECK_TRUNCATE_TYPE_uint(start_pc - buf)));
////                selfmod_copy_start_offs[i][j]IF_X64([k]) = (uint) (start_pc - buf);
////                IF_X64(ASSERT(CHECK_TRUNCATE_TYPE_uint(end_pc - buf)));
////                selfmod_copy_end_offs[i][j]IF_X64([k]) = (uint) (end_pc - buf);
////                LOG(THREAD, LOG_EMIT, 3, "selfmod offs %d %d"IF_X64(" %d")": %u %u\n",
////                    i, j, IF_X64_(k)
////                    selfmod_copy_start_offs[i][j]IF_X64([k]),
////                    selfmod_copy_end_offs[i][j]IF_X64([k]));
////                /* free the instrlist_t elements */
////                instrlist_clear(dcontext, &ilist);
//        }
//    }
}
