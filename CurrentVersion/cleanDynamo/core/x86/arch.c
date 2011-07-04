/* **********************************************************
 * Copyright (c) 2000-2009 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* Copyright (c) 2003-2007 Determina Corp. */
/* Copyright (c) 2001-2003 Massachusetts Institute of Technology */
/* Copyright (c) 2000-2001 Hewlett-Packard Company */

/*
 * arch.c - x86 architecture specific routines
 */

#include "../globals.h"
#include "../link.h"
#include "../fragment.h"

#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "decode_fast.h"
#include "../fcache.h"
#include "proc.h"
#include "instrument.h"

#include <string.h> /* for memcpy */

/* The real size of generated code we need varies by cache line size and
 * options like inlining of ibl code.  We also generate different routines
 * for thread-private and thread-shared.  So, we dynamically extend the size
 * as we generate.  Currently our max is under 5 pages.
 */
#define GENCODE_RESERVE_SIZE (5*PAGE_SIZE)

#define GENCODE_COMMIT_SIZE \
    ((size_t)(ALIGN_FORWARD(sizeof(generated_code_t), PAGE_SIZE) + PAGE_SIZE))

/* Thread-shared generated routines.
 * We don't allocate the shared_code statically so that we can mark it
 * executable.
 */
generated_code_t *shared_code = NULL;

void interp_init(void);

byte *
code_align_forward(byte *pc, size_t alignment)
{
	// COMPLETEDD #270 code_align_forward
    byte *new_pc = (byte *) ALIGN_FORWARD(pc, alignment);
  	printf("Starting code_align_forward\n");
    DODEBUG({
        SET_TO_NOPS(pc, new_pc - pc);
    });
    return new_pc;
}

static int syscall_method = SYSCALL_METHOD_UNINITIALIZED;
void
protect_generated_code(generated_code_t *code, bool writable)
{
	// COMPLETEDD #409 protect_generated_code
    if (TEST(SELFPROT_GENCODE, DYNAMO_OPTION(protect_mask)) &&
        code->writable != writable) {
        byte *genstart = (byte *)PAGE_START(code->gen_start_pc);
        if (!writable) {
            ASSERT(code->writable);
            code->writable = writable;
        }
        STATS_INC(gencode_prot_changes);
        change_protection(genstart, code->commit_end_pc - genstart,
                          writable);
        if (writable) {
            ASSERT(!code->writable);
            code->writable = writable;
        }
    }
}

static byte *
move_to_start_of_cache_line(byte *pc)
{
	// COMPLETEDD #272 move_to_start_of_cache_line
	printf("Starting move_to_start_of_cache_line\n");
    return code_align_forward(pc, proc_get_cache_line_size());
}

static byte *
check_size_and_cache_line(generated_code_t *code, byte *pc)
{
	// COMPLETEDD #274 check_size_and_cache_line
    /* Assumption: no single emit uses more than a page.
     * We keep an extra page at all times and release it at the end.
     */
    byte *next_pc = move_to_start_of_cache_line(pc);
    printf("Starting check_size_and_cache_line\n");
    if ((byte *)ALIGN_FORWARD(pc, PAGE_SIZE) + PAGE_SIZE > code->commit_end_pc) {
        ASSERT(code->commit_end_pc + PAGE_SIZE <= ((byte *)code) + GENCODE_RESERVE_SIZE);
        heap_mmap_extend_commitment(code->commit_end_pc, PAGE_SIZE);
        code->commit_end_pc += PAGE_SIZE;
    }
    return next_pc;
}

static void
shared_gencode_init(IF_X64_ELSE(bool x86_mode, void))
{
	// INPROCESSS shared_gencode_init
    generated_code_t *gencode;
    ibl_branch_type_t branch_type;
    byte *pc;
    printf("shared_gencode_init has started\n");
#ifdef X64
    fragment_t *fragment;
#endif

    gencode = heap_mmap_reserve(GENCODE_RESERVE_SIZE, GENCODE_COMMIT_SIZE);
    /* we would return gencode and let caller assign, but emit routines
     * that this routine calls query the shared vars so we set here
     */
#ifdef X64
    if (x86_mode)
        shared_code_x86 = gencode;
    else
#endif
        shared_code = gencode;
    memset(gencode, 0, sizeof(*gencode));

    IF_X64(gencode->x86_mode = x86_mode);
    /* Generated code immediately follows struct */
    gencode->gen_start_pc = ((byte *)gencode) + sizeof(*gencode);
    gencode->commit_end_pc = ((byte *)gencode) + GENCODE_COMMIT_SIZE;
    for (branch_type = IBL_BRANCH_TYPE_START;
         branch_type < IBL_BRANCH_TYPE_END; branch_type++) {
        gencode->trace_ibl[branch_type].initialized = false;
        gencode->bb_ibl[branch_type].initialized = false;
        gencode->coarse_ibl[branch_type].initialized = false;
        /* cache the mode so we can pass just the ibl_code_t around */
        IF_X64(gencode->trace_ibl[branch_type].x86_mode = x86_mode);
        IF_X64(gencode->bb_ibl[branch_type].x86_mode = x86_mode);
        IF_X64(gencode->coarse_ibl[branch_type].x86_mode = x86_mode);
    }

    pc = gencode->gen_start_pc;
    pc = check_size_and_cache_line(gencode, pc);
    gencode->fcache_enter = pc;
    pc = emit_fcache_enter_shared(GLOBAL_DCONTEXT, gencode, pc);
//    pc = check_size_and_cache_line(gencode, pc);
//    gencode->fcache_return = pc;
//    pc = emit_fcache_return_shared(GLOBAL_DCONTEXT, gencode, pc);
//    if (DYNAMO_OPTION(coarse_units)) {
//        pc = check_size_and_cache_line(gencode, pc);
//        gencode->fcache_return_coarse = pc;
//        pc = emit_fcache_return_coarse(GLOBAL_DCONTEXT, gencode, pc);
//        pc = check_size_and_cache_line(gencode, pc);
//        gencode->trace_head_return_coarse = pc;
//        pc = emit_trace_head_return_coarse(GLOBAL_DCONTEXT, gencode, pc);
//    }
//#ifdef WINDOWS_PC_SAMPLE
//    gencode->fcache_enter_return_end = pc;
//#endif
//
//    /* PR 244737: thread-private uses shared gencode on x64.
//     * Should we set the option instead? */
//    if (IF_X64(!DYNAMO_OPTION(disable_traces) ||)
//        DYNAMO_OPTION(shared_trace_ibl_routine)) {
//        /* expected to be false for private trace IBL routine  */
//        pc = emit_ibl_routines(GLOBAL_DCONTEXT, gencode,
//                               pc, gencode->fcache_return,
//                               DYNAMO_OPTION(shared_traces) ?
//                               IBL_TRACE_SHARED : IBL_TRACE_PRIVATE, /* source_fragment_type */
//                               /* thread_shared */
//                               IF_X64_ELSE(true, DYNAMO_OPTION(shared_trace_ibl_routine)),
//                               true, /* target_trace_table */
//                               gencode->trace_ibl);
//    }
//    if (IF_X64_ELSE(true, DYNAMO_OPTION(shared_bbs))) {
//        pc = emit_ibl_routines(GLOBAL_DCONTEXT, gencode,
//                               pc, gencode->fcache_return,
//                               IBL_BB_SHARED, /* source_fragment_type */
//                               /* thread_shared */
//                               IF_X64_ELSE(true, SHARED_FRAGMENTS_ENABLED()),
//                               !DYNAMO_OPTION(bb_ibl_targets), /* target_trace_table */
//                               gencode->bb_ibl);
//    }
//    if (DYNAMO_OPTION(coarse_units)) {
//        pc = emit_ibl_routines(GLOBAL_DCONTEXT, gencode, pc,
//                               /* ibl routines use regular fcache_return */
//                               gencode->fcache_return,
//                               IBL_COARSE_SHARED, /* source_fragment_type */
//                               /* thread_shared */
//                               IF_X64_ELSE(true, SHARED_FRAGMENTS_ENABLED()),
//                               !DYNAMO_OPTION(bb_ibl_targets), /*target_trace_table*/
//                               gencode->coarse_ibl);
//    }
//
//#ifdef WINDOWS_PC_SAMPLE
//    gencode->ibl_routines_end = pc;
//#endif
//#if defined(WINDOWS) && !defined(X64)
//    /* no dispatch needed on x64 since syscall routines are thread-shared */
//    if (DYNAMO_OPTION(shared_fragment_shared_syscalls)) {
//        pc = check_size_and_cache_line(gencode, pc);
//        gencode->shared_syscall = pc;
//        pc = emit_shared_syscall_dispatch(GLOBAL_DCONTEXT, pc);
//        pc = check_size_and_cache_line(gencode, pc);
//        gencode->unlinked_shared_syscall = pc;
//        pc = emit_unlinked_shared_syscall_dispatch(GLOBAL_DCONTEXT, pc);
//        LOG(GLOBAL, LOG_EMIT, 3,
//            "shared_syscall_dispatch: linked "PFX", unlinked "PFX"\n",
//            gencode->shared_syscall, gencode->unlinked_shared_syscall);
//    }
//#endif
//
//#ifdef LINUX
//    /* must create before emit_do_clone_syscall() in emit_syscall_routines() */
//    pc = check_size_and_cache_line(gencode, pc);
//    gencode->new_thread_dynamo_start = pc;
//    pc = emit_new_thread_dynamo_start(GLOBAL_DCONTEXT, pc);
//#endif
//
//#ifdef X64
//# ifdef WINDOWS
//    /* plain fcache_enter indirects through edi, and next_tag is in tls,
//     * so we don't need a separate routine for callback return
//     */
//    gencode->fcache_enter_indirect = gencode->fcache_enter;
//    gencode->shared_syscall_code.x86_mode = x86_mode;
//# endif
//    /* PR 284029: for now we assume there are no syscalls in x86 code */
//    if (IF_X64_ELSE(!x86_mode, true)) {
//        /* PR 244737: syscall routines are all shared */
//        pc = emit_syscall_routines(GLOBAL_DCONTEXT, gencode, pc, true/*thread-shared*/);
//    }
//
//    /* since we always have a shared fcache_return we can make reset stub shared */
//    gencode->reset_exit_stub = pc;
//    fragment = linkstub_fragment(GLOBAL_DCONTEXT, (linkstub_t *) get_reset_linkstub());
//    if (gencode->x86_mode)
//        fragment = empty_fragment_mark_x86(fragment);
//    /* reset exit stub should look just like a direct exit stub */
//    pc += insert_exit_stub_other_flags
//        (GLOBAL_DCONTEXT, fragment,
//         (linkstub_t *) get_reset_linkstub(), pc, LINK_DIRECT);
//#elif defined(LINUX) && defined(HAVE_TLS)
//    /* PR 212570: we need a thread-shared do_syscall for our vsyscall hook */
//    /* PR 361894: we don't support sysenter if no TLS */
//    ASSERT(gencode->do_syscall == NULL);
//    pc = check_size_and_cache_line(gencode, pc);
//    gencode->do_syscall = pc;
//    pc = emit_do_syscall(GLOBAL_DCONTEXT, gencode, pc, gencode->fcache_return,
//                         true/*shared*/, false, &gencode->do_syscall_offs);
//#endif
//
//#ifdef TRACE_HEAD_CACHE_INCR
//    pc = check_size_and_cache_line(gencode, pc);
//    gencode->trace_head_incr = pc;
//    pc = emit_trace_head_incr_shared(GLOBAL_DCONTEXT, pc, gencode->fcache_return);
//#endif
//
//    ASSERT(pc < gencode->commit_end_pc);
//    gencode->gen_end_pc = pc;
//    release_final_page(gencode);
//
//    DOLOG(3, LOG_EMIT, {
//        dump_emitted_routines(GLOBAL_DCONTEXT, GLOBAL,
//                              IF_X64_ELSE(x86_mode ? "thread-shared x86" :
//                                          "thread-shared", "thread-shared"),
//                              gencode, pc);
//    });
//#ifdef INTERNAL
//    if (INTERNAL_OPTION(gendump)) {
//        dump_emitted_routines_to_file(GLOBAL_DCONTEXT, "gencode-shared",
//                                      IF_X64_ELSE(x86_mode ? "thread-shared x86" :
//                                                  "thread-shared", "thread-shared"),
//                                      gencode, pc);
//    }
//#endif
//#ifdef WINDOWS_PC_SAMPLE
//    if (dynamo_options.profile_pcs &&
//        dynamo_options.prof_pcs_gencode >= 2 &&
//        dynamo_options.prof_pcs_gencode <= 32) {
//        gencode->profile =
//            create_profile(gencode->gen_start_pc, pc,
//                           dynamo_options.prof_pcs_gencode, NULL);
//        start_profile(gencode->profile);
//    } else
//        gencode->profile = NULL;
//#endif
//
//    gencode->writable = true;
//    protect_generated_code(gencode, READONLY);
}

void
arch_init()
{
  // COMPLETEDD #284 arch_init
	printf("Starting arch_init\n");
  ASSERT(sizeof(opnd_t) == EXPECTED_SIZEOF_OPND);
  /* ensure our flag sharing is done properly */
  ASSERT(LINK_FINAL_INSTR_SHARED_FLAG < INSTR_FIRST_NON_LINK_SHARED_FLAG);
  ASSERT_TRUNCATE(byte, byte, OPSZ_LAST_ENUM);
  DODEBUG({ reg_check_reg_fixer(); });

  /* Verify that the structures used for a register spill area and to hold IBT
   * table addresses & masks for IBL code are laid out as expected. We expect
   * the spill area to be at offset 0 within the container struct and for the
   * table address/mask pair array to follow immediately after the spill area.
   */
  /* FIXME These can be converted into compile-time checks as follows:
   *
   *    lookup_table_access_t table[
   *       (offsetof(local_state_extended_t, spill_space) == 0 &&
   *        offsetof(local_state_extended_t, table_space) ==
   *           sizeof(spill_state_t)) ? IBL_BRANCH_TYPE_END : -1 ];
   *
   * This isn't self-descriptive, though, so it's not being used right now
   * (xref case 7097).
   */
  ASSERT(offsetof(local_state_extended_t, spill_space) == 0);
  ASSERT(offsetof(local_state_extended_t, table_space) == sizeof(spill_state_t));

  /* Ensure we have no unexpected padding inside structs that include
   * dr_mcontext_t (app_state_at_intercept_t and dcontext_t) */
  ASSERT(offsetof(dr_mcontext_t, pc) + sizeof(byte*) == sizeof(dr_mcontext_t));
  ASSERT(offsetof(app_state_at_intercept_t, mc) ==
         offsetof(app_state_at_intercept_t, app_errno) + sizeof(int)
         IF_X64(+ 4/*padding*/));
  /* Try to catch errors in x86.asm offsets for dcontext_t */
  ASSERT(sizeof(unprotected_context_t) == sizeof(dr_mcontext_t) +
         IF_WINDOWS_ELSE(IF_X64_ELSE(8, 4), 8));

  interp_init();
  int someValue;
  if (someValue = USE_SHARED_GENCODE()) {
  	// INPROCESSS Removed the ability to use USE_SHARED_GENCODE
      /* thread-shared generated code */
      /* Assumption: no single emit uses more than a page.
       * We keep an extra page at all times and release it at the end.
       * FIXME: have heap_mmap not allocate a guard page, and use our
       * extra for that page, to use one fewer total page of address space.
       */
//      ASSERT(GENCODE_COMMIT_SIZE < GENCODE_RESERVE_SIZE);
//
//      shared_gencode_init(IF_X64(false/*x64 mode*/));
  }
  printf("Some Value: %d \n", someValue);
}

void
arch_thread_init(dcontext_t *dcontext)
{
	// INPROCESSS arch_thread_init
	printf("ARCH_THREAD_INIT_UNDER_CONSTRUCTION\n");
    byte *pc;
    generated_code_t *code;
    ibl_branch_type_t branch_type;
    printf("Starting arch_thread_init\n");

    /* Simplest to have a real dcontext for emitting the selfmod code
     * and finding the patch offsets so we do it on 1st thread init */
    static bool selfmod_init = false;
//    if (!selfmod_init) {
//        ASSERT(!dynamo_initialized); /* .data +w */
//        selfmod_init = true;
//        set_selfmod_sandbox_offsets(dcontext);
//    }

    ASSERT_CURIOSITY(proc_is_cache_aligned(get_local_state())
                     IF_WINDOWS(|| DYNAMO_OPTION(tls_align != 0)));

    /* For detach on windows need to use a separate mmap so we can leave this
     * memory around in case of outstanding callbacks when we detach.  Without
     * detach or on linux could just use one of our heaps (which would save
     * a little space, (would then need to coordinate with arch_thread_exit)
     */
    ASSERT(GENCODE_COMMIT_SIZE < GENCODE_RESERVE_SIZE);
    /* case 9474; share allocation unit w/ thread-private stack */
    code = heap_mmap_reserve_post_stack(dcontext,
                                        GENCODE_RESERVE_SIZE, GENCODE_COMMIT_SIZE);
    ASSERT(code != NULL);
    /* FIXME case 6493: if we split private from shared, remove this
     * memset since we will no longer have a bunch of fields we don't use
     */
    memset(code, 0, sizeof(*code));
    /* Generated code immediately follows struct */
    code->gen_start_pc = ((byte *)code) + sizeof(*code);
    code->commit_end_pc = ((byte *)code) + GENCODE_COMMIT_SIZE;
    for (branch_type = IBL_BRANCH_TYPE_START;
         branch_type < IBL_BRANCH_TYPE_END; branch_type++) {
        code->trace_ibl[branch_type].initialized = false;
        code->bb_ibl[branch_type].initialized = false;
        code->coarse_ibl[branch_type].initialized = false;
    }

    dcontext->private_code = (void *) code;

    pc = code->gen_start_pc;
    pc = check_size_and_cache_line(code, pc);
    code->fcache_enter = pc;
//    pc = emit_fcache_enter(dcontext, code, pc);
//    pc = check_size_and_cache_line(code, pc);
//    code->fcache_return = pc;
//    pc = emit_fcache_return(dcontext, code, pc);;
//#ifdef WINDOWS_PC_SAMPLE
//    code->fcache_enter_return_end = pc;
//#endif
//
//    /* Currently all ibl routines target the trace hashtable
//       and we don't yet support basic blocks as targets of an IBL.
//       However, having separate routines at least enables finer control
//       over the indirect exit stubs.
//       This way we have inlined IBL stubs for trace but not in basic blocks.
//
//       TODO: After separating the IBL routines, now we can retarget them to separate
//       hashtables (or alternatively chain several IBL routines together).
//       From trace ib exits we can only go to {traces}, so no change here.
//         (when we exit to a basic block we need to mark as a trace head)
//       From basic block ib exits we should be able to go to {traces + bbs - traceheads}
//          (for the tracehead bbs we actually have to increment counters.
//       From shared_syscall we should be able to go to {traces + bbs}.
//
//       TODO: we also want to have separate routines per indirect branch types to enable
//       the restricted control transfer policies to be efficiently enforced.
//    */
//    if (!DYNAMO_OPTION(disable_traces) && DYNAMO_OPTION(shared_trace_ibl_routine)) {
//        if (!DYNAMO_OPTION(shared_traces)) {
//            /* copy all bookkeeping information from shared_code into thread private
//               needed by get_ibl_routine*() */
//            ibl_branch_type_t branch_type;
//            for (branch_type = IBL_BRANCH_TYPE_START;
//                 branch_type < IBL_BRANCH_TYPE_END; branch_type++) {
//                code->trace_ibl[branch_type] =
//                    SHARED_GENCODE(code->x86_mode)->trace_ibl[branch_type];
//            }
//        } /* FIXME: no private traces supported right now w/ -shared_traces */
//    } else if (PRIVATE_TRACES_ENABLED()) {
//        /* shared_trace_ibl_routine should be false for private (performance test only) */
//        pc = emit_ibl_routines(dcontext, code, pc, code->fcache_return,
//                               IBL_TRACE_PRIVATE, /* source_fragment_type */
//                               DYNAMO_OPTION(shared_trace_ibl_routine), /* thread_shared */
//                               true, /* target_trace_table */
//                               code->trace_ibl);
//    }
//    pc = emit_ibl_routines(dcontext, code, pc, code->fcache_return,
//                           IBL_BB_PRIVATE, /* source_fragment_type */
//                           /* need thread-private for selfmod regardless of sharing */
//                           false, /* thread_shared */
//                           !DYNAMO_OPTION(bb_ibl_targets), /* target_trace_table */
//                           code->bb_ibl);
//#ifdef WINDOWS_PC_SAMPLE
//    code->ibl_routines_end = pc;
//#endif
//
//#if defined(LINUX) && !defined(HAVE_TLS)
//    /* for HAVE_TLS we use the shared version; w/o TLS we don't
//     * make any shared routines (PR 361894)
//     */
//    /* must create before emit_do_clone_syscall() in emit_syscall_routines() */
//    pc = check_size_and_cache_line(code, pc);
//    code->new_thread_dynamo_start = pc;
//    pc = emit_new_thread_dynamo_start(dcontext, pc);
//#endif
//
//#ifdef RETURN_STACK
//    /* unlinked_return comes first */
//    pc = check_size_and_cache_line(code, pc);
//    code->return_lookup = pc;
//    pc = emit_return_lookup(dcontext, pc,
//                            code->indirect_branch_lookup,
//                            code->unlinked_ib_lookup,
//                            &code->unlinked_return);
//#endif
//#ifdef WINDOWS
//    pc = check_size_and_cache_line(code, pc);
//    code->fcache_enter_indirect = pc;
//    pc = emit_fcache_enter_indirect(dcontext, code, pc, code->fcache_return);
//#endif
//    pc = emit_syscall_routines(dcontext, code, pc, false/*thread-private*/);
//#ifdef TRACE_HEAD_CACHE_INCR
//    pc = check_size_and_cache_line(code, pc);
//    code->trace_head_incr = pc;
//    pc = emit_trace_head_incr(dcontext, pc, code->fcache_return);
//#endif
//#ifdef CHECK_RETURNS_SSE2_EMIT
//    /* PR 248210: unsupported feature on x64: need to move to thread-shared gencode
//     * if want to support it.
//     */
//    IF_X64(ASSERT_NOT_IMPLEMENTED(false));
//    pc = check_size_and_cache_line(code, pc);
//    code->pextrw = pc;
//    pc = emit_pextrw(dcontext, pc);
//    pc = check_size_and_cache_line(code, pc);
//    code->pinsrw = pc;
//    pc = emit_pinsrw(dcontext, pc);
//#endif
//    code->reset_exit_stub = pc;
//    /* reset exit stub should look just like a direct exit stub */
//    pc += insert_exit_stub_other_flags(dcontext,
//                                       linkstub_fragment(dcontext, (linkstub_t *)
//                                                         get_reset_linkstub()),
//                                       (linkstub_t *) get_reset_linkstub(),
//                                       pc, LINK_DIRECT);
//    ASSERT(pc < code->commit_end_pc);
//    code->gen_end_pc = pc;
//    release_final_page(code);
//
//    DOLOG(3, LOG_EMIT, {
//        dump_emitted_routines(dcontext, THREAD, "thread-private", code, pc);
//    });
//#ifdef INTERNAL
//    if (INTERNAL_OPTION(gendump)) {
//        dump_emitted_routines_to_file(dcontext, "gencode-private", "thread-private",
//                                      code, pc);
//    }
//#endif
//#ifdef WINDOWS_PC_SAMPLE
//    if (dynamo_options.profile_pcs && dynamo_options.prof_pcs_gencode >= 2 &&
//        dynamo_options.prof_pcs_gencode <= 32) {
//        code->profile = create_profile(code->gen_start_pc, pc,
//                                       dynamo_options.prof_pcs_gencode, NULL);
//        start_profile(code->profile);
//    } else
//        code->profile = NULL;
//#endif
//
//    code->writable = true;
//    /* For SELFPROT_GENCODE we don't make unwritable until after we patch,
//     * though for hotp_only we don't patch.
//     */
//#ifdef HOT_PATCHING_INTERFACE
//    if (DYNAMO_OPTION(hotp_only))
//#endif
//        protect_generated_code(code, READONLY);
}

void
arch_thread_exit(dcontext_t *dcontext _IF_WINDOWS(bool detach_stacked_callbacks))
{
}

void
arch_exit(IF_WINDOWS_ELSE_NP(bool detach_stacked_callbacks, void))
{
}

void
update_generated_hashtable_access(dcontext_t *dcontext)
{
	// COMPLETEDD #407 update_geerated_hashtable_access
	update_indirect_branch_lookup(dcontext);
}

cache_pc
get_unlinked_entry(dcontext_t *dcontext, cache_pc linked_entry)
{
	return 0;
}

static inline
ibl_code_t*
get_ibl_routine_code_internal(dcontext_t *dcontext,
                              ibl_source_fragment_type_t source_fragment_type,
                              ibl_branch_type_t branch_type
                              _IF_X64(gencode_mode_t mode))
{
	// COMPLETEDD #322 get_ibl_routine_code_internal
    switch (source_fragment_type) {
    case IBL_BB_SHARED:
    	  printf("Option 1\n");
        if (!DYNAMO_OPTION(shared_bbs))
            return NULL;
        return &(get_shared_gencode(dcontext _IF_X64(mode))->bb_ibl[branch_type]);
    case IBL_BB_PRIVATE:
  	  printf("Option 2\n");
        return &(get_emitted_routines_code(dcontext _IF_X64(mode))->bb_ibl[branch_type]);
    case IBL_TRACE_SHARED:
        if (!DYNAMO_OPTION(shared_traces))
            return NULL;
        return &(get_shared_gencode(dcontext _IF_X64(mode))->trace_ibl[branch_type]);
    case IBL_TRACE_PRIVATE:
  	  printf("Option 4\n");
        return &(get_emitted_routines_code(dcontext _IF_X64(mode))
                 ->trace_ibl[branch_type]);
    case IBL_COARSE_SHARED:
  	  printf("Option 5\n");
        if (!DYNAMO_OPTION(coarse_units))
            return NULL;
        return &(get_shared_gencode(dcontext _IF_X64(mode))->coarse_ibl[branch_type]);
    default:
        ASSERT_NOT_REACHED();
    }
    ASSERT_NOT_REACHED();
    return NULL;
}

cache_pc
get_ibl_routine_ex(dcontext_t *dcontext, ibl_entry_point_type_t entry_type,
                   ibl_source_fragment_type_t source_fragment_type,
                   ibl_branch_type_t branch_type _IF_X64(gencode_mode_t mode))
{
	// COMPLETEDD #323 get_ibl_routine_ex
    ibl_code_t *ibl_code =
        get_ibl_routine_code_internal(dcontext,
                                      source_fragment_type, branch_type _IF_X64(mode));
    if (ibl_code == NULL || !ibl_code->initialized)
        return NULL;
    switch (entry_type) {
    case IBL_LINKED:
        return (cache_pc) ibl_code->indirect_branch_lookup_routine;
    case IBL_UNLINKED:
        return (cache_pc) ibl_code->unlinked_ibl_entry;
    case IBL_DELETE:
        return (cache_pc) ibl_code->target_delete_entry;
    default:
        ASSERT_NOT_REACHED();
    }
    return NULL;
}

cache_pc
get_ibl_routine(dcontext_t *dcontext, ibl_entry_point_type_t entry_type,
                ibl_source_fragment_type_t source_fragment_type,
                ibl_branch_type_t branch_type)
{
	// COMPLETEDD #324 get_ibl_routine
	printf("Starting get_ibl_routine\n");
    return get_ibl_routine_ex(dcontext, entry_type, source_fragment_type,
                              branch_type _IF_X64(GENCODE_FROM_DCONTEXT));
}
/* Convert FRAG_TABLE_* flags to FRAG_* flags */
/* FIXME This seems more appropriate in fragment.c but since there's no
 * need for the functionality there, we place it here and inline it. We
 * can move it if other pieces need the functionality later.
 */
static inline uint
table_flags_to_frag_flags(dcontext_t *dcontext, ibl_table_t *table)
{
	// COMPLETEDD #319 table_flags_to_frags
    uint flags = 0;
    if (TEST(FRAG_TABLE_TARGET_SHARED, table->table_flags))
        flags |= FRAG_SHARED;
    if (TEST(FRAG_TABLE_TRACE, table->table_flags))
        flags |= FRAG_IS_TRACE;
    /* We want to make sure that any updates to FRAG_TABLE_* flags
     * are reflected in this routine. */
    ASSERT_NOT_IMPLEMENTED(!TESTANY(~(FRAG_TABLE_INCLUSIVE_HIERARCHY |
                                      FRAG_TABLE_IBL_TARGETED |
                                      FRAG_TABLE_TARGET_SHARED |
                                      FRAG_TABLE_SHARED |
                                      FRAG_TABLE_TRACE |
                                      FRAG_TABLE_PERSISTENT |
                                      HASHTABLE_USE_ENTRY_STATS |
                                      HASHTABLE_ALIGN_TABLE),
                                    table->table_flags));
    return flags;
}

ibl_source_fragment_type_t
get_source_fragment_type(dcontext_t *dcontext, uint fragment_flags)
{
	// COMPLETEDD #331 get_source_fragement_type
    if (TEST(FRAG_IS_TRACE, fragment_flags)) {
        return (TEST(FRAG_SHARED, fragment_flags)) ? IBL_TRACE_SHARED : IBL_TRACE_PRIVATE;
    } else if (TEST(FRAG_COARSE_GRAIN, fragment_flags)) {
        ASSERT(TEST(FRAG_SHARED, fragment_flags));
        return IBL_COARSE_SHARED;
    } else {
        return (TEST(FRAG_SHARED, fragment_flags)) ? IBL_BB_SHARED : IBL_BB_PRIVATE;
    }
}

/* Derive the PC of an entry point that aids in atomic hashtable deletion.
 * FIXME: Once we can correlate from what table the fragment is being
 * deleted and therefore type of the corresponding IBL routine, we can
 * widen the interface and be more precise about which entry point
 * is returned, i.e., specify something other than IBL_GENERIC.
 */
cache_pc
get_target_delete_entry_pc(dcontext_t *dcontext, ibl_table_t *table)
{
	// COMPLETEDD #325 get_target_delete_entry_pc
    /*
     * A shared IBL routine makes sure any registers restored on the
     * miss path are all saved in the current dcontext - as well as
     * copying the ECX in both TLS scratch and dcontext, so it is OK
     * to simply return the thread private routine.  We have
     * proven that they are functionally equivalent (all data in the
     * shared lookup is fs indirected to the private dcontext)
     *
     * FIXME: we can in fact use a global delete_pc entry point that
     * is the unlinked path of a shared_ibl_not_found, just like we
     * could share all routines. Since it doesn't matter much for now
     * we can also return the slightly more efficient private
     * ibl_not_found path.
     */
    uint frag_flags = table_flags_to_frag_flags(dcontext, table);

    ASSERT(dcontext != GLOBAL_DCONTEXT);

    return (cache_pc) get_ibl_routine(dcontext, IBL_DELETE,
                                      get_source_fragment_type(dcontext,
                                                               frag_flags),
                                      table->branch_type);
}

void
translation_info_free(dcontext_t *dcontext, translation_info_t *info)
{
}

/* With our weak flushing consistency we must store translation info
 * for any fragment that may outlive its original app code (case
 * 3559).  Here we store actual translation info.  An alternative is
 * to store elided jmp information and a copy of the source memory,
 * but that takes more memory for all but the smallest fragments.  A
 * better alternative is to reliably de-mangle, which would require
 * only elided jmp information.
 */
translation_info_t *
record_translation_info(dcontext_t *dcontext, fragment_t *f, instrlist_t *existing_ilist)
{
	return 0;
}

bool
is_indirect_branch_lookup_routine(dcontext_t *dcontext, cache_pc pc)
{
	return 0;
}

/* exported beyond x86/ */
fcache_enter_func_t
get_fcache_enter_shared_routine(dcontext_t *dcontext)
{
	return 0;
}

/* exported to dispatch.c */
fcache_enter_func_t
get_fcache_enter_private_routine(dcontext_t *dcontext)
{
	return 0;
}

cache_pc
get_do_syscall_entry(dcontext_t *dcontext)
{
	return 0;
}

/* PR 286922: we need an int syscall even when vsyscall is sys{call,enter} */
cache_pc
get_do_int_syscall_entry(dcontext_t *dcontext)
{
	return 0;
}

cache_pc
get_fcache_target(dcontext_t *dcontext)
{
	return 0;
}

cache_pc
get_do_clone_syscall_entry(dcontext_t *dcontext)
{
	return 0;
}

/* For 32-bit linux apps on 64-bit kernels we assume that all syscalls that
 * we use this for are ok w/ int (i.e., we don't need a sys{call,enter} version).
 */
byte *
get_global_do_syscall_entry()
{
	return 0;
}

void
copy_mcontext(dr_mcontext_t *src, dr_mcontext_t *dst)
{
}

int
get_syscall_method(void)
{
	// COMPLETEDD #453 get_syscall_method
	printf("Starting get_syscall_method\n");
  return syscall_method;
}

/* set the fcache target for the next code cache entry */
void
set_fcache_target(dcontext_t *dcontext, cache_pc value)
{
}

/* Does the syscall instruction always return to the invocation point? */
bool
does_syscall_ret_to_callsite(void)
{
	// COMPLETEDD #451 does_syscall_ret_to_callsite
	printf("Starting does_syscall_ret_to_callsite\n");
    return (syscall_method == SYSCALL_METHOD_INT ||
            syscall_method == SYSCALL_METHOD_SYSCALL
            IF_WINDOWS(|| syscall_method == SYSCALL_METHOD_WOW64));
}

bool
is_after_do_syscall_addr(dcontext_t *dcontext, cache_pc pc)
{
	// COMPLETEDD #452 is_after_do_syscall_addr
	printf("Starting is_after_do_syscall_addr\n");
    generated_code_t *code = get_emitted_routines_code(dcontext
                                                       _IF_X64(GENCODE_FROM_DCONTEXT));
    ASSERT(code != NULL);
    return (pc == (cache_pc) (code->do_syscall + code->do_syscall_offs) ||
            pc == (cache_pc) (code->do_int_syscall + code->do_int_syscall_offs)
            IF_VMX86(|| pc == (cache_pc) (code->do_vmkuw_syscall +
                                          code->do_vmkuw_syscall_offs)));
}

static bool
in_generated_shared_routine(dcontext_t *dcontext, cache_pc pc)
{
	// COMPLETEDD #449 in_generated_shared_routine
	printf("Starting in_generated_shared_routine\n");
    if (USE_SHARED_GENCODE()) {
        return (pc >= (cache_pc)(shared_code->gen_start_pc) &&
                pc < (cache_pc)(shared_code->commit_end_pc));
    }
    return false;
}

bool
in_generated_routine(dcontext_t *dcontext, cache_pc pc)
{
	// COMPLETEDD #450 in_generated_routine
	printf("Starting in_generated_routine\n");
  generated_code_t *code = THREAD_GENCODE(dcontext);

  return ((pc >= (cache_pc)(code->gen_start_pc) &&
          pc < (cache_pc)(code->commit_end_pc))
          || in_generated_shared_routine(dcontext, pc));
  /* FIXME: what about inlined IBL stubs */
	return 0;
}

/* Assumes that pc is a pc_recreatable place (i.e. in_fcache(), though could do
 * syscalls with esp, also see FIXME about separate stubs in
 * recreate_app_state_internal()), ASSERTs otherwise.
 * If caller knows which fragment pc belongs to, caller should pass in f
 * to avoid work and avoid lock rank issues as pclookup acquires shared_cache_lock;
 * else, pass in NULL for f.
 * NOTE - If called by a thread other than the tdcontext owner, caller must
 * ensure tdcontext remains valid.  Caller also must ensure that it is safe to
 * allocate memory from tdcontext (for instr routines), i.e. caller owns
 * tdcontext or the owner of tdcontext is suspended.  Also if tdcontext is
 * !couldbelinking then caller must own the thread_initexit_lock in case
 * recreate_fragment_ilist() is called.
 * NOTE - If this function is unable to translate the pc, but the pc is
 * in_fcache() then there is an assert curiosity and the function returns NULL.
 * This can happen only from the pc being in a fragment that is pending
 * deletion (ref case 3559 others).  Most callers don't check the returned
 * value and wouldn't have a way to recover even if they did. FIXME
 */
/* Use THREAD_GET instead of THREAD so log messages go to calling thread */
app_pc
recreate_app_pc(dcontext_t *tdcontext, cache_pc pc, fragment_t *f)
{
	return 0;
}

cache_pc
get_reset_exit_stub(dcontext_t *dcontext)
{
	// COMPLETEDD #458 get_reset_exit_stub
  generated_code_t *code = THREAD_GENCODE(dcontext);
  return (cache_pc) code->reset_exit_stub;
}

bool
is_after_syscall_address(dcontext_t *dcontext, cache_pc pc)
{
	// COMPLETEDD #457 is_after_syscall_address
#ifdef WINDOWS
    if (pc == after_shared_syscall_addr(dcontext))
        return true;
    if (pc == after_do_syscall_addr(dcontext))
        return true;
    return false;
#else
    return is_after_do_syscall_addr(dcontext, pc);
#endif
    /* NOTE - we ignore global_do_syscall since that's only used in special
     * circumstances and is not something the callers (recreate_app_state)
     * really know how to handle. */
}

/* The esp in mcontext must either be valid or NULL (if null will be unable to
 * recreate on XP and 03 at vsyscall_after_syscall and on sygate 2k at after syscall).
 * Returns true if successful.  Whether successful or not, attempts to modify
 * mcontext with recreated state. If just_pc only translates the pc
 * (this is more likely to succeed)
 */
/* Use THREAD_GET instead of THREAD so log messages go to calling thread */
/* Also see NOTEs at recreate_app_state() about lock usage, and lack of full stack
 * translation. */
static recreate_success_t
recreate_app_state_internal(dcontext_t *tdcontext, dr_mcontext_t *mcontext,
                            bool just_pc, fragment_t *owning_f, bool restore_memory)
{
	// INPROCESSS recreate_app_state_internal
	printf("WARNING Starting Recreate_app_state_internal NOT DONE\n");
//    recreate_success_t res = (just_pc ? RECREATE_SUCCESS_PC : RECREATE_SUCCESS_STATE);
//#ifdef WINDOWS
//    if (get_syscall_method() == SYSCALL_METHOD_SYSENTER &&
//        mcontext->pc == vsyscall_after_syscall &&
//        mcontext->xsp != 0) {
//        ASSERT(get_os_version() >= WINDOWS_VERSION_XP);
//        /* case 5441 sygate hack means ret addr to after_syscall will be at
//         * esp+4 (esp will point to ret in ntdll.dll) for sysenter */
//        /* FIXME - should we check that esp is readable? */
//        if (is_after_syscall_address(tdcontext, *(cache_pc *)
//                                     (mcontext->xsp+
//                                      (DYNAMO_OPTION(sygate_sysenter) ? 4 : 0)))) {
//            /* no translation needed, ignoring sysenter stack hacks */
//            LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                "recreate_app no translation needed (at vsyscall)\n");
//            return res;
//        } else {
//            /* this is a dynamo system call! */
//            LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                "recreate_app at dynamo system call\n");
//            return RECREATE_FAILURE;
//        }
//    }
//#else
//    if (get_syscall_method() == SYSCALL_METHOD_SYSENTER &&
//        /* Even when the main syscall method is sysenter, we also have a
//         * do_int_syscall and do_clone_syscall that use int, so check both.
//         * Note that we don't modify the stack, so once we do sysenter syscalls
//         * inlined in the cache (PR 288101) we'll need some mechanism to
//         * distinguish those: but for now if a sysenter instruction is used it
//         * has to be do_syscall since DR's own syscalls are ints.
//         */
//        (mcontext->pc == vsyscall_sysenter_return_pc ||
//         is_after_do_syscall_addr(tdcontext, mcontext->pc))) {
//        LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//            "recreate_app no translation needed (at syscall)\n");
//        return res;
//    }
//#endif
//    else if (is_after_syscall_address(tdcontext, mcontext->pc) &&
//             does_syscall_ret_to_callsite()) {
//            /* suspended inside kernel at syscall
//             * all registers have app values for syscall */
//            LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                "recreate_app pc = after_syscall, translating\n");
//#ifdef WINDOWS
//            if (DYNAMO_OPTION(sygate_int) &&
//                get_syscall_method() == SYSCALL_METHOD_INT) {
//                if ((app_pc)mcontext->xsp == NULL)
//                    return RECREATE_FAILURE;
//                /* dr system calls will have the same after_syscall address when
//                 * sygate hack are in effect so need to check top of stack to see
//                 * if we are returning to dr or do/share syscall (generated
//                 * routines) */
//                if (!in_generated_routine(tdcontext, *(app_pc *)mcontext->xsp)) {
//                    /* this must be a dynamo system call! */
//                    LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                        "recreate_app at dynamo system call\n");
//                    return RECREATE_FAILURE;
//                }
//                ASSERT(*(app_pc *)mcontext->xsp ==
//                       after_do_syscall_code(tdcontext) ||
//                       *(app_pc *)mcontext->xsp ==
//                       after_shared_syscall_code(tdcontext));
//                if (!just_pc) {
//                    /* This is an int system call and since for sygate
//                     * compatibility we redirect those with a call to an ntdll.dll
//                     * int 2e ret 0 we need to pop the stack once to match app. */
//                    mcontext->xsp += XSP_SZ; /* pop the stack */
//                }
//            }
//#endif
//            mcontext->pc = POST_SYSCALL_PC(tdcontext);
//            return res;
//    } else if (mcontext->pc == get_reset_exit_stub(tdcontext)) {
//        LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//            "recreate_app at reset exit stub => using next_tag "PFX"\n",
//            tdcontext->next_tag);
//        /* context is completely native except the pc */
//        mcontext->pc = tdcontext->next_tag;
//        return res;
//    } else if (in_generated_routine(tdcontext, mcontext->pc)) {
//        LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//            "recreate_app state at untranslatable address in "
//            "generated routines for thread "IDFMT"\n", tdcontext->owning_thread);
//        return RECREATE_FAILURE;
//    } else if (in_fcache(mcontext->pc)) {
//        /* FIXME: what if pc is in separate direct stub???
//         * do we have to read the &l from the stub to find linkstub_t and thus
//         * fragment_t owner?
//         */
//        /* NOTE - only at this point is it safe to grab locks other then the
//         * fcache_unit_areas.lock */
//        linkstub_t *l;
//        cache_pc cti_pc;
//        instrlist_t *ilist = NULL;
//        fragment_t *f = owning_f;
//        bool alloc = false;
//#ifdef CLIENT_INTERFACE
//        dr_restore_state_info_t client_info;
//#endif
//        IF_X64(bool old_mode;)
//
//        /* Rather than storing a mapping table, we re-build the fragment
//         * containing the code cache pc whenever we can.  For pending-deletion
//         * fragments we can't do that and have to store the info, due to our
//         * weak consistency flushing where the app code may have changed
//         * before we get here (case 3559).
//         */
//
//        /* Check whether we have a fragment w/ stored translations before
//         * asking to recreate the ilist
//         */
//        if (f == NULL)
//            f = fragment_pclookup_with_linkstubs(tdcontext, mcontext->pc, &alloc);

        /* Whether a bb or trace, this routine will recreate the entire ilist. */
//        if (f == NULL)
//            ilist = recreate_fragment_ilist(tdcontext, mcontext->pc, &f, &alloc,
//                                            true/*mangle*/ _IF_CLIENT(true/*client*/));
//        else if (FRAGMENT_TRANSLATION_INFO(f) == NULL) {
//            /* NULL for pc indicates that f is valid */
//            bool new_alloc;
//            ilist = recreate_fragment_ilist(tdcontext, NULL, &f, &new_alloc,
//                                            true/*mangle*/ _IF_CLIENT(true/*client*/));
//            ASSERT(owning_f == NULL || f == owning_f);
//            ASSERT(!new_alloc);
//        }
//        if (ilist == NULL && (f == NULL || FRAGMENT_TRANSLATION_INFO(f) == NULL)) {
//            /* It is problematic if this routine fails.  Many places assume that
//             * recreate_app_pc() will work.
//             */
//            ASSERT(!INTERNAL_OPTION(safe_translate_flushed));
//            res = RECREATE_FAILURE;
//            goto recreate_app_state_done;
//        }
//
//        LOG(THREAD_GET, LOG_INTERP, 2,
//            "recreate_app : pc is in F%d("PFX")%s\n", f->id, f->tag,
//            ((f->flags & FRAG_IS_TRACE) != 0)?" (trace)":"");
//
//        DOLOG(2, LOG_SYNCH, {
//            if (ilist != NULL) {
//                LOG(THREAD_GET, LOG_SYNCH, 2, "ilist for recreation:\n");
//                instrlist_disassemble(tdcontext, f->tag, ilist, THREAD_GET);
//            }
//        });
//
//        /* if pc is in an exit stub, we find the corresponding exit instr */
//        cti_pc = NULL;
//        for (l = FRAGMENT_EXIT_STUBS(f); l; l = LINKSTUB_NEXT_EXIT(l)) {
//            if (EXIT_HAS_LOCAL_STUB(l->flags, f->flags)) {
//                /* FIXME: as computing the stub pc becomes more expensive,
//                 * should perhaps check fragment_body_end_pc() or something
//                 * that only does one stub check up front, and then find the
//                 * exact stub if pc is beyond the end of the body.
//                 */
//                if (mcontext->pc < EXIT_STUB_PC(tdcontext, f, l))
//                    break;
//                cti_pc = EXIT_CTI_PC(f, l);
//            }
//        }
//        if (cti_pc != NULL) {
//            /* target is inside an exit stub!
//             * new target: the exit cti, not its stub
//             */
//            if (!just_pc) {
//                /* FIXME : translate from exit stub */
//                LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                    "recreate_app_helper -- can't full recreate state, pc "PFX" "
//                    "is in exit stub\n", mcontext->pc);
//                res = RECREATE_SUCCESS_PC; /* failed on full state, but pc good */
//                goto recreate_app_state_done;
//            }
//            LOG(THREAD_GET, LOG_INTERP|LOG_SYNCH, 2,
//                "\ttarget "PFX" is inside an exit stub, looking for its cti "
//                " "PFX"\n", mcontext->pc, cti_pc);
//            mcontext->pc = cti_pc;
//        }
//
//        /* Recreate in same mode as original fragment */
//        IF_X64(old_mode = set_x86_mode(tdcontext, FRAG_IS_32(f->flags)));
//
//        /* now recreate the state */
//#ifdef CLIENT_INTERFACE
//        /* keep a copy of the pre-translation state */
//        client_info.raw_mcontext = *mcontext;
//        client_info.raw_mcontext_valid = true;
//#endif
//        if (ilist == NULL) {
//            ASSERT(f != NULL && FRAGMENT_TRANSLATION_INFO(f) != NULL);
//            ASSERT(!TEST(FRAG_WAS_DELETED, f->flags) ||
//                   INTERNAL_OPTION(safe_translate_flushed));
//            res = recreate_app_state_from_info(tdcontext, FRAGMENT_TRANSLATION_INFO(f),
//                                               (byte *) f->start_pc,
//                                               (byte *) f->start_pc + f->size,
//                                               mcontext, just_pc _IF_DEBUG(f->flags));
//            STATS_INC(recreate_via_stored_info);
//        } else {
//            res = recreate_app_state_from_ilist(tdcontext, ilist, (byte *) f->tag,
//                                                (byte *) FCACHE_ENTRY_PC(f),
//                                                (byte *) f->start_pc + f->size,
//                                                mcontext, just_pc _IF_DEBUG(f->flags));
//            STATS_INC(recreate_via_app_ilist);
//        }
//        IF_X64(set_x86_mode(tdcontext, old_mode));
//
//#ifdef STEAL_REGISTER
//        /* FIXME: conflicts w/ PR 263407 reg spill tracking */
//        ASSERT_NOT_IMPLEMENTED(false && "conflicts w/ reg spill tracking");
//        if (!just_pc) {
//            /* get app's value of edi */
//            mc->xdi = get_mcontext(tdcontext)->xdi;
//        }
//#endif
//#ifdef CLIENT_INTERFACE
//        if (res != RECREATE_FAILURE) {
//            /* PR 214962: if the client has a restore callback, invoke it to
//             * fix up the state (and pc).
//             */
//            client_info.mcontext = mcontext;
//            client_info.fragment_info.tag = (void *) f->tag;
//            client_info.fragment_info.cache_start_pc = FCACHE_ENTRY_PC(f);
//            client_info.fragment_info.is_trace = TEST(FRAG_IS_TRACE, f->flags);
//            client_info.fragment_info.app_code_consistent =
//                !TESTANY(FRAG_WAS_DELETED|FRAG_SELFMOD_SANDBOXED, f->flags);
//            /* i#220/PR 480565: client has option of failing the translation */
//            if (!instrument_restore_state(tdcontext, restore_memory, &client_info))
//                res = RECREATE_FAILURE;
//        }
//#endif
//
//    recreate_app_state_done:
//        /* free the instrlist_t elements */
//        if (ilist != NULL)
//            instrlist_clear_and_destroy(tdcontext, ilist);
//        if (alloc) {
//            ASSERT(f != NULL);
//            fragment_free(tdcontext, f);
//        }
//        return res;
//    } else {
//        /* handle any other cases, in DR etc. */
//        return RECREATE_FAILURE;

}

/* Translates the code cache state in mcontext into what it would look like
 * in the original application.
 * If it fails altogether, returns RECREATE_FAILURE, but still provides
 * a best-effort translation.
 * If it fails to restore the full machine state, but does restore the pc,
 * returns RECREATE_SUCCESS_PC.
 * If it successfully restores the full machine state,
 * returns RECREATE_SUCCESS_STATE.  Only for full success does it
 * consider the restore_memory parameter, which, if true, requests restoration
 * of any memory values that were shifted (primarily due to clients) (otherwise,
 * only the passed-in mcontext is modified). If restore_memory is
 * true, the caller should always relocate the translated thread, as
 * it may not execute properly if left at its current location (it
 * could be in the middle of client code in the cache).
 *
 * FIXME: does not undo stack mangling for sysenter
 */
/* NOTE - Can be called with a thread suspended at an arbitrary place by synch
 * routines so must not call mutex_lock (or call a function that does) unless
 * the synch routines have checked that lock.  Currently only fcache_unit_areas.lock
 * is used (for in_fcache in recreate_app_state_internal())
 * (if in_fcache succeeds then assumes other locks won't be a problem).
 * NOTE - If called by a thread other than the tdcontext owner, caller must
 * ensure tdcontext remains valid.  Caller also must ensure that it is safe to
 * allocate memory from tdcontext (for instr routines), i.e. caller owns
 * tdcontext or the owner of tdcontext is suspended.  Also if tdcontext is
 * !couldbelinking then caller must own the thread_initexit_lock in case
 * recreate_fragment_ilist() is called.  We assume that when tdcontext is
 * not the calling thread, this is a thread synch request, and is NOT from
 * an app fault (PR 267260)!
 */
/* Use THREAD_GET instead of THREAD so log messages go to calling thread */
recreate_success_t
recreate_app_state(dcontext_t *tdcontext, dr_mcontext_t *mcontext,
                   bool restore_memory)
{
	// INPROCESSS recreate_app_state
	printf("WARNING Starting recreate_app_state NOT DONE\n");
//    recreate_success_t res;
//
//#ifdef DEBUG
//    if (stats->loglevel >= 2 && (stats->logmask & LOG_SYNCH) != 0) {
//        LOG(THREAD_GET, LOG_SYNCH, 2,
//            "recreate_app_state -- translating from:\n");
//        dump_mcontext(mcontext, THREAD_GET, DUMP_NOT_XML);
//    }
//#endif
//
//    res = recreate_app_state_internal(tdcontext, mcontext, false, NULL, restore_memory);
//
//#ifdef DEBUG
//    if (res) {
//        if (stats->loglevel >= 2 && (stats->logmask & LOG_SYNCH) != 0) {
//            LOG(THREAD_GET, LOG_SYNCH, 2,
//                "recreate_app_state -- translation is:\n");
//            dump_mcontext(mcontext, THREAD_GET, DUMP_NOT_XML);
//        }
//    } else {
//        LOG(THREAD_GET, LOG_SYNCH, 2,
//            "recreate_app_state -- unable to translate\n");
//    }
//#endif
//
//    return res;
}

/* PR 313715: If we fail to hook the vsyscall page (xref PR 212570, PR 288330)
 * we fall back on int, but we have to tweak syscall param #5 (ebp)
 */
bool
should_syscall_method_be_sysenter(void)
{
	return 0;
}

cache_pc
fcache_return_routine(dcontext_t *dcontext)
{
	return 0;
}

bool
in_context_switch_code(dcontext_t *dcontext, cache_pc pc)
{
	return 0;
}

bool
in_indirect_branch_lookup_code(dcontext_t *dcontext, cache_pc pc)
{
	return 0;
}
