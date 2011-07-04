#include "../globals.h"
#include "../link.h"
#include "../fragment.h"
#include "../fcache.h"
#include "../emit.h"

#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "instrument.h" /* for dr_insert_call() */
#include "proc.h"
#include <string.h> /* for memcpy */
#include "../instrlist.h"
#include "decode.h"
#include "decode_fast.h"
#ifdef DEBUG
#include "disassemble.h"
#endif
#include <limits.h> /* for UCHAR_MAX */
#include "../perscache.h"

#ifdef VMX86_SERVER
# include "vmkuw.h"
#endif

/* Updates an indirect branch exit stub with the latest hashtable mask
 * and hashtable address
 * See also update_indirect_branch_lookup
 */

/* macros shared by fcache_enter and fcache_return
 * in order to generate both thread-private code that uses absolute
 * addressing and thread-shared or dcontext-shared code that uses
 * xdi (and xsi) for addressing.
 * The via_reg macros now auto-magically pick the opnd size from the
 * target register and so work with more than just pointer-sized values.
 */
/* PR 244737: even thread-private fragments use TLS on x64.  We accomplish
 * that at the caller site, so we should never see an "absolute" request.
 */

/* make code more readable by shortening long lines
 * we mark all as meta to avoid client interface asserts
 */
#define POST instrlist_meta_postinsert
#define PRE  instrlist_meta_preinsert
#define APP  instrlist_append

#define RESTORE_FROM_DC(dc, reg, offs) \
    RESTORE_FROM_DC_VIA_REG(absolute, dc, REG_NULL, reg, offs)
void
update_indirect_exit_stub(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
}

int
fragment_prefix_size(uint flags)
{
	return 0;
}

cache_pc
indirect_linkstub_stub_pc(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
	return 0;
}

cache_pc
indirect_linkstub_target(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
	// COMPLETEDD #495 indirect_linkstub_target
	printf("Starting indirect_linkstub_target\n");
  ASSERT(LINKSTUB_INDIRECT(l->flags));
  ASSERT(!TESTANY(LINK_NI_SYSCALL_ALL, l->flags));
#ifdef WINDOWS
  if (EXIT_TARGETS_SHARED_SYSCALL(l->flags)) {
      /* currently this is the only way to distinguish shared_syscall
       * exit from other indirect exits and from other exits in
       * a fragment containing ignorable or non-ignorable syscalls
       */
      ASSERT(TEST(FRAG_HAS_SYSCALL, f->flags));
      return shared_syscall_routine_ex(dcontext
                                       _IF_X64(MODE_OVERRIDE(FRAG_IS_32(f->flags))));
  }
#endif
  if (TEST(FRAG_COARSE_GRAIN, f->flags)) {
      /* Need to target the ibl prefix.  Passing in cti works as well as stub,
       * and avoids a circular dependence where linkstub_unlink_entry_offset()
       * call this routine to get the target and then this routine asks for
       * the stub which calls linkstub_unlink_entry_offset()...
       */
      return get_coarse_ibl_prefix(dcontext, EXIT_CTI_PC(f, l),
                                   extract_branchtype(l->flags));
  } else {
      return get_ibl_routine_ex(dcontext,
                                IF_X64(TEST(LINK_TRACE_CMP, l->flags) ? IBL_TRACE_CMP :)
                                IBL_LINKED,
                                get_source_fragment_type(dcontext, f->flags),
                                extract_branchtype(l->flags)
                                _IF_X64(MODE_OVERRIDE(FRAG_IS_32(f->flags))));
  }
}

/* FIXME: case 10334: pass in info? */
bool
coarse_is_trace_head(cache_pc stub)
{
	// COMPLETEDD #471 coarse_is_trace_head
	printf("Starting coarse_is_trace_head\n");
  if (coarse_is_entrance_stub(stub)) {
      cache_pc tgt = entrance_stub_jmp_target(stub);
      /* FIXME: could see if tgt is a jmp and deref and cmp to
       * trace_head_return_coarse_routine() to avoid the vmvector
       * lookup required to find the prefix
       */
      return tgt == trace_head_return_coarse_prefix(stub, NULL);
  }
  return false;
}

/* Returns whether it had to change page protections */
static bool
patch_coarse_branch(cache_pc stub, cache_pc tgt, bool hot_patch,
                    coarse_info_t *info /*OPTIONAL*/)
{
	// COMPLETEDD #498 patch_coarse_branch
    bool stubs_readonly = false;
    bool stubs_restore = false;
    if (DYNAMO_OPTION(persist_protect_stubs)) {
        if (info == NULL)
            info = get_stub_coarse_info(stub);
        ASSERT(info != NULL);
        if (info->stubs_readonly) {
            stubs_readonly = true;
            stubs_restore = true;
            /* if we don't preserve mapped-in COW state the protection change
             * will fail (case 10570)
             */
            make_copy_on_writable((byte *)PAGE_START(entrance_stub_jmp(stub)),
                                  /* stub jmp can't cross page boundary (can't
                                   * cross cache line in fact) */
                                  PAGE_SIZE);
            if (DYNAMO_OPTION(persist_protect_stubs_limit) > 0) {
                info->stubs_write_count++;
                if (info->stubs_write_count >
                    DYNAMO_OPTION(persist_protect_stubs_limit)) {
                    SYSLOG_INTERNAL_WARNING_ONCE("pcache stubs over write limit");
                    STATS_INC(pcache_unprot_over_limit);
                    stubs_restore = false;
                    info->stubs_readonly = false;
                }
            }
        }
    }
    patch_branch(entrance_stub_jmp(stub), tgt, HOT_PATCHABLE);
    if (stubs_restore)
        make_unwritable((byte *)PAGE_START(entrance_stub_jmp(stub)), PAGE_SIZE);
    return stubs_readonly;
}
/* Passing in stub's info avoids a vmvector lookup */
void
unlink_entrance_stub(dcontext_t *dcontext, cache_pc stub, uint flags,
                     coarse_info_t *info /*OPTIONAL*/)
{
	// COMPLETEDD #499 unlink_entrance_stub
	printf("Starting unlink_enterance_stub\n");
  cache_pc tgt;
  ASSERT(DYNAMO_OPTION(coarse_units));
  ASSERT(coarse_is_entrance_stub(stub));
  ASSERT(self_owns_recursive_lock(&change_linking_lock));
  LOG(THREAD, LOG_LINKS, 5,
      "unlink_entrance_stub "PFX"\n", stub);
  if (TESTANY(FRAG_IS_TRACE_HEAD|FRAG_IS_TRACE, flags))
      tgt = trace_head_return_coarse_prefix(stub, info);
  else
      tgt = fcache_return_coarse_prefix(stub, info);
  if (patch_coarse_branch(stub, tgt, HOT_PATCHABLE, info))
      STATS_INC(pcache_unprot_unlink);
}

/* Passing in stub's info avoids a vmvector lookup */
bool
entrance_stub_linked(cache_pc stub, coarse_info_t *info /*OPTIONAL*/)
{
	// COMPLETEDD #469 entrance_stub_linked
	printf("Starting entrance_stub_linked\n");
  /* entrance stubs are of two types:
   * - targeting trace heads: always point to trace_head_return_coarse,
   *   whether target exists or not, so are always unlinked;
   * - targeting non-trace-heads: if linked, point to fragment; if unlinked,
   *   point to fcache_return_coarse
   */
  cache_pc tgt = entrance_stub_jmp_target(stub);
  /* FIXME: do vmvector just once instead of for each call */
  return (tgt != trace_head_return_coarse_prefix(stub, info) &&
          tgt != fcache_return_coarse_prefix(stub, info));
}

/* The write that inserts the relative target is done atomically so this
 * function is safe with respect to a thread executing the code containing
 * this target, presuming that the code in both the before and after states
 * is valid.
 * For x64 this routine only works for 32-bit reachability.  If further
 * reach is needed the caller must use indirection.  Xref PR 215395.
 */
byte *
insert_relative_target(byte *pc, cache_pc target, bool hot_patch)
{
	// COMPLETEDD #326 insert_relative_target
	printf("Starting insert_relative_target\n");
    /* insert 4-byte pc-relative offset from the beginning of the next instruction
     */
    int value = (int)(ptr_int_t)(target - pc - 4);
    IF_X64(ASSERT(CHECK_TRUNCATE_TYPE_int(target - pc - 4)));
    ATOMIC_4BYTE_WRITE(pc, value, hot_patch);
    pc += 4;
    return pc;
}

/* since we now support branch hints on long cbrs, we need to do a little
 * decoding to find their length
 */
cache_pc
cbr_fallthrough_exit_cti(cache_pc prev_cti_pc)
{
	return 0;
}

/* Return size in bytes required for an exit stub with specified
 * target and FRAG_ flags
 */
int
exit_stub_size(dcontext_t *dcontext, cache_pc target, uint flags)
{
	return 0;
}

/* Our context switch to and from the fragment cache are arranged such
 * that there is no persistent state kept on the dstack, allowing us to
 * start with a clean slate on exiting the cache.  This eliminates the
 * need to protect our dstack from inadvertent or malicious writes.
 *
 * We do not bother to save any DynamoRIO state, even the eflags.  We clear
 * them in fcache_return, assuming that a cleared state is always the
 * proper value (df is never set across the cache, etc.)
 *
 * fcache_enter(dcontext_t *dcontext)
 *   Used by dispatch to begin execution in fcache at dcontext->next_tag

    if (!absolute)
        mov    ARG1,%xdi # dcontext param
      if (TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
        RESTORE_FROM_UPCONTEXT PROT_OFFSET,%xsi
      endif
    endif

        # restore app's error code (32 bits even on x64)
    .ifdef WINDOWS
        RESTORE_FROM_DCONTEXT app_errno_OFFSET,%eax
        mov     %eax, fs:ERRNO_TIB_OFFSET
    .else
        RESTORE_FROM_DCONTEXT app_errno_OFFSET,%eax
        SAVE_TO_DCONTEXT %eax,errno_OFFSET
    .endif

    if (!absolute)
        # put target somewhere we can be absolute about
        RESTORE_FROM_UPCONTEXT next_tag_OFFSET,%xax
      if (shared)
        mov  %xax,fs:xax_OFFSET
      endif
    endif

    if (EXIT_DR_HOOK != NULL && !dcontext->ignore_enterexit)
      if (!absolute)
        push    %xdi
        push    %xsi
      else
        # support for skipping the hook
        RESTORE_FROM_UPCONTEXT ignore_enterexit_OFFSET,%edi
        cmpl    %edi,0
        jnz     post_hook
      endif
        call    EXIT_DR_HOOK # for x64 windows, reserve 32 bytes stack space for call
      if (!absolute)
        pop    %xsi
        pop    %xdi
      endif
    endif

    post_hook:

        # restore the original register state
        RESTORE_FROM_UPCONTEXT xflags_OFFSET,%xax
        push    %xax
        popf            # restore eflags temporarily using dstack
    if preserve_xmm_caller_saved
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+0*16,%xmm0
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+1*16,%xmm1
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+2*16,%xmm2
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+3*16,%xmm3
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+4*16,%xmm4
        RESTORE_FROM_UPCONTEXT xmm_OFFSET+5*16,%xmm5
    endif
    ifdef X64
        RESTORE_FROM_UPCONTEXT r8_OFFSET,%r8
        RESTORE_FROM_UPCONTEXT r9_OFFSET,%r9
        RESTORE_FROM_UPCONTEXT r10_OFFSET,%r10
        RESTORE_FROM_UPCONTEXT r11_OFFSET,%r11
        RESTORE_FROM_UPCONTEXT r12_OFFSET,%r12
        RESTORE_FROM_UPCONTEXT r13_OFFSET,%r13
        RESTORE_FROM_UPCONTEXT r14_OFFSET,%r14
        RESTORE_FROM_UPCONTEXT r15_OFFSET,%r15
    endif
        RESTORE_FROM_UPCONTEXT xax_OFFSET,%xax
        RESTORE_FROM_UPCONTEXT xbx_OFFSET,%xbx
        RESTORE_FROM_UPCONTEXT xcx_OFFSET,%xcx
        RESTORE_FROM_UPCONTEXT xdx_OFFSET,%xdx
    if (absolute || !TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
        RESTORE_FROM_UPCONTEXT xsi_OFFSET,%xsi
    endif
    if (absolute || TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
        RESTORE_FROM_UPCONTEXT xdi_OFFSET,%xdi
    endif
        RESTORE_FROM_UPCONTEXT xbp_OFFSET,%xbp
        RESTORE_FROM_UPCONTEXT xsp_OFFSET,%xsp
    if (!absolute)
      if (TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
        RESTORE_FROM_UPCONTEXT xsi_OFFSET,%xsi
      else
        RESTORE_FROM_UPCONTEXT xdi_OFFSET,%xdi
      endif
    endif

    ifdef X64 and (target is x86 mode)
        # we can't indirect through a register since we couldn't restore
        # the high bits (PR 283152)
        mov gencode-jmp86-value, fs:xbx_OFFSET
        far jmp to next instr, stored w/ 32-bit cs selector in fs:xbx_OFFSET
    endif

        # jump indirect through dcontext->next_tag, set by dispatch()
    if (absolute)
        JUMP_VIA_DCONTEXT next_tag_OFFSET
    else
      if (shared)
        jmp *fs:xax_OFFSET
      else
        JUMP_VIA_DCONTEXT nonswapped_scratch_OFFSET
      endif
    endif

        # now executing in fcache
 */
static byte *
emit_fcache_enter_common(dcontext_t *dcontext, generated_code_t *code, byte *pc,
                         bool absolute, bool shared)
{
	// INPROCESSS emit_fcache_enter_common
	printf("WARNING STARTING EMIT_FCACHE_ENTER_COMMON NOT YET IMPLEMENTED\n");
//    int len;
//    instrlist_t ilist;
//    patch_list_t patch;
//
//    instr_t *post_hook = INSTR_CREATE_label(dcontext);
//
//    init_patch_list(&patch, absolute ? PATCH_TYPE_ABSOLUTE : PATCH_TYPE_INDIRECT_XDI);
//    instrlist_init(&ilist);
//
//
//    if (!absolute) {
//        /* grab gen routine's parameter dcontext and put it into edi */
//        APP(&ilist, INSTR_CREATE_mov_ld(dcontext, opnd_create_reg(REG_XDI), OPND_ARG1));
//        if (TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
//            APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XSI, PROT_OFFS));
//    }
//
//    /* restore app's error code */
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_EAX, APP_ERRNO_OFFSET));
//    APP(&ilist, SAVE_TO_DC(dcontext, REG_EAX, ERRNO_OFFSET));
//
//
//    if (!absolute) {
//        /* put target into special slot that we can be absolute about */
//        APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XAX, NEXT_TAG_OFFSET));
//        if (shared) {
//            APP(&ilist, SAVE_TO_TLS(dcontext, REG_XAX, FCACHE_ENTER_TARGET_SLOT));
//        } else {
//            /* no special scratch slot! */
//            ASSERT_NOT_IMPLEMENTED(false);
//        }
//    }
//
//    if (EXIT_DR_HOOK != NULL) {
//        /* if absolute, don't bother to save any regs around the call */
//        if (!absolute) {
//            /* save xdi and xsi around call.
//             * for x64, they're supposed to be callee-saved on windows,
//             * but not linux (though we could move to r12-r15 on linux
//             * instead of pushing them).
//             */
//            APP(&ilist, INSTR_CREATE_push(dcontext, opnd_create_reg(REG_XDI)));
//            APP(&ilist, INSTR_CREATE_push(dcontext, opnd_create_reg(REG_XSI)));
//        }
//        /* make sure to use dr_insert_call() rather than a raw OP_call instr,
//         * since x64 windows requires 32 bytes of stack space even w/ no args,
//         * and we don't want anyone clobbering our pushed registers!
//         */
//        dr_insert_call((void *)dcontext, &ilist, NULL/*append*/, (void *)EXIT_DR_HOOK, 0);
//        if (!absolute) {
//            /* save edi and esi around call */
//            APP(&ilist, INSTR_CREATE_pop(dcontext, opnd_create_reg(REG_XSI)));
//            APP(&ilist, INSTR_CREATE_pop(dcontext, opnd_create_reg(REG_XDI)));
//        }
//    }
//
//    /* restore the original register state */
//
//    APP(&ilist, post_hook/*label*/);
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XAX, XFLAGS_OFFSET));
//    APP(&ilist, INSTR_CREATE_push(dcontext, opnd_create_reg(REG_XAX)));
//    /* restore eflags temporarily using dstack */
//    APP(&ilist, INSTR_CREATE_RAW_popf(dcontext));
//    if (preserve_xmm_caller_saved()) {
//        /* PR 264138: we must preserve xmm0-5 if on a 64-bit kernel.
//         * Rather than try and optimize we save/restore on every cxt
//         * sw.  The xmm field is aligned, so we can use movdqa/movaps,
//         * though movdqu is stated to be as fast as movdqa when aligned:
//         * but if so, why have two versions?  Is it only loads and not stores
//         * for which that is true?  => PR 266305.
//         * It's not clear that movdqa is any faster (and its opcode is longer):
//         * movdqa and movaps are listed as the same latency and throughput in
//         * the AMD optimization guide.  Yet examples of fast memcpy online seem
//         * to use movdqa when sse2 is available.
//         * Note that mov[au]p[sd] and movdq[au] are functionally equivalent.
//         */
//        int i;
//        uint opcode = (proc_has_feature(FEATURE_SSE2) ? OP_movdqa : OP_movaps);
//        ASSERT(proc_has_feature(FEATURE_SSE));
//        for (i=0; i<NUM_XMM_SAVED; i++) {
//            APP(&ilist, instr_create_1dst_1src
//                (dcontext, opcode, opnd_create_reg(REG_XMM0 + (reg_id_t)i),
//                 OPND_DC_FIELD(absolute, dcontext, OPSZ_16, XMM_OFFSET + i*XMM_REG_SIZE)));
//        }
//    }
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XAX, XAX_OFFSET));
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XBX, XBX_OFFSET));
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XCX, XCX_OFFSET));
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XDX, XDX_OFFSET));
//    /* must restore esi last */
//    if (absolute || !TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
//        APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XSI, XSI_OFFSET));
//    /* must restore edi last */
//    if (absolute || TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
//        APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XDI, XDI_OFFSET));
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XBP, XBP_OFFSET));
//    APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XSP, XSP_OFFSET));
//    /* must restore esi last */
//    if (!absolute) {
//        if (TEST(SELFPROT_DCONTEXT, dynamo_options.protect_mask))
//            APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XSI, XSI_OFFSET));
//        else
//            APP(&ilist, RESTORE_FROM_DC(dcontext, REG_XDI, XDI_OFFSET));
//    }
//
//    /* Jump indirect through next_tag.  Dispatch set this value with
//     * where we want to go next in the fcache_t.
//     */
//    if (absolute) {
//        APP(&ilist, instr_create_jump_via_dcontext(dcontext, NEXT_TAG_OFFSET));
//    } else {
//        if (shared) {
//            /* next_tag placed into tls slot earlier in this routine */
//            APP(&ilist, INSTR_CREATE_jmp_ind(dcontext,
//                                             OPND_TLS_FIELD(FCACHE_ENTER_TARGET_SLOT)));
//
//        } else {
//            /* no special scratch slot! */
//            ASSERT_NOT_IMPLEMENTED(false);
//        }
//    }
//
//    /* now encode the instructions */
//    len = encode_with_patch_list(dcontext, &patch, &ilist, pc);
//    ASSERT(len != 0);
//
//    /* free the instrlist_t elements */
//    instrlist_clear(dcontext, &ilist);
//
//    return pc + len;
    return 0;
}

byte *
emit_fcache_enter_shared(dcontext_t *dcontext, generated_code_t *code, byte *pc)
{
	// INPROCESSS emit_fcache_enter_shared
	printf("Starting emit_fcache_enter_shared\n");
    return emit_fcache_enter_common(dcontext, code, pc,
                                    false/*through xdi*/, true/*shared*/);
}

int insert_exit_stub(dcontext_t *dcontext, fragment_t *f,
                 linkstub_t *l, cache_pc stub_pc)
{
	return 0;
}

/* Passing in stub's info avoids a vmvector lookup */
void
link_entrance_stub(dcontext_t *dcontext, cache_pc stub, cache_pc tgt,
                   bool hot_patch, coarse_info_t *info /*OPTIONAL*/)
{
}

bool
link_direct_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l, fragment_t *targetf,
                 bool hot_patch)
{
	return 0;
}

/* NOTE : for inlined indirect branches linking is !NOT! atomic with respect
 * to a thread executing in the cache unless using the atomic_inlined_linking
 * option (unlike unlinking)
 */
void
link_indirect_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l, bool hot_patch)
{
}

uint
coarse_exit_prefix_size(uint flags)
{
	// INPROCESSS coarse_exit_prefix_size
	printf("WARNING Starting coarse_exit_prefix_size Not Done Returning 0\n");
	return 0;
}

byte *
emit_coarse_exit_prefix(dcontext_t *dcontext, byte *pc, coarse_info_t *info)
{
	return 0;
}


/* based on machine state, returns which of cbr l1 and fall-through l2
 * must have been taken
 */
linkstub_t *
linkstub_cbr_disambiguate(dcontext_t *dcontext, fragment_t *f,
                          linkstub_t *l1, linkstub_t *l2)
                          {
	return 0;
                          }

/* Patch the (direct) branch at branch_pc so it branches to target_pc
 * The write that actually patches the branch is done atomically so this
 * function is safe with respect to a thread executing this branch presuming
 * that both the before and after targets are valid
 */
void
patch_branch(cache_pc branch_pc, cache_pc target_pc, bool hot_patch)
{
	// INPROCESSS patch_branch
	printf("WARNING Starting patch_branch Note DONE\n");
//  cache_pc byte_ptr = exit_cti_disp_pc(branch_pc);
//  insert_relative_target(byte_ptr, target_pc, hot_patch);
}

cache_pc
entrance_stub_target_tag(cache_pc stub)
{
	// COMPLETEDD entrance_stub_target_taq
	printf("Starting entrance_stub_target_tag\n");
  cache_pc jmp = entrance_stub_jmp(stub);

  return *((cache_pc *)(jmp-4));
}

void
unlink_direct_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
}

/* This is an atomic operation with respect to a thread executing in the
 * cache (barring ifdef NATIVE_RETURN), for inlined indirect exits the
 * unlinked path of the ibl routine detects the race condition between the
 * two patching writes and handles it appropriately unless using the
 * atomic_inlined_linking option in which case there is only one patching
 * write (since tail is duplicated) */
void
unlink_indirect_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
}

bool
is_exit_cti_patchable(dcontext_t *dcontext, instr_t *inst, uint frag_flags)
{
	return 0;
}

bool
is_exit_cti_stub_patchable(dcontext_t *dcontext, instr_t *inst, uint frag_flags)
{
	return 0;
}

/* inserts any nop padding needed to ensure patchable branch offsets don't
 * cross cache line boundaries.  If emitting sets the offset field of all
 * instructions, else sets the translation for the added nops (for
 * recreating). If emitting and -pad_jmps_shift_{bb,trace} returns the number
 * of bytes to shift the start_pc by (this avoids putting a nop before the
 * first exit cti) else returns 0. */
uint
nop_pad_ilist(dcontext_t *dcontext, fragment_t *f, instrlist_t *ilist, bool emitting)
{
	return 0;
}

void
insert_fragment_prefix(dcontext_t *dcontext, fragment_t *f)
{
}

/* return startpc shifted by the necessary bytes to pad patchable jmps of the
 * exit stub to proper alignment */
byte *
pad_for_exitstub_alignment(dcontext_t *dcontext, linkstub_t *l, fragment_t *f, byte *startpc)
{
	return 0;
}

int
linkstub_unlink_entry_offset(dcontext_t *dcontext, fragment_t *f, linkstub_t *l)
{
	return 0;
}

/* Returns whether stub is an entrance stub as opposed to a fragment
 * or a coarse indirect stub.  FIXME: if we separate coarse indirect
 * stubs from bodies we'll need to put them somewhere else, or fix up
 * decode_fragment() to be able to distinguish them in some other way
 * like first instruction tls slot.
 */
bool
coarse_is_entrance_stub(cache_pc stub)
{
	// INPROCESSS coarse_is_entrance_stub
//	printf("Starting coarse_is_entrance_stub\n");
//  bool res = false;
//  /* FIXME: case 10334: pass in info and if non-NULL avoid lookup here? */
//  coarse_info_t *info = get_stub_coarse_info(stub);
//  if (info != NULL) {
//      res = ALIGNED(stub, coarse_stub_alignment(info)) &&
//          *entrance_stub_jmp(stub) == JMP_OPCODE;
//      DODEBUG({
//          if (res) {
//              cache_pc tgt = entrance_stub_jmp_target(stub);
//              ASSERT(!in_fcache(stub));
//              ASSERT(tgt == trace_head_return_coarse_prefix(stub, info) ||
//                     tgt == fcache_return_coarse_prefix(stub, info) ||
//                     /* another fragment */
//                     in_fcache(tgt));
//          }
//      });
//  }
//  return res;
	return 0;
}

/* Returns an upper bound on the number of bytes that will be needed to add
 * this fragment to a trace */
uint
extend_trace_pad_bytes(fragment_t *add_frag)
{
	return 0;
}

/* Update info pointer in exit prefixes */
void
patch_coarse_exit_prefix(dcontext_t *dcontext, coarse_info_t *info)
{
}

uint
coarse_indirect_stub_size(coarse_info_t *info)
{
	return 0;
}

byte *
insert_relative_jump(byte *pc, cache_pc target, bool hot_patch)
{
	return 0;
}

cache_pc
entrance_stub_jmp_target(cache_pc stub)
{
	// COMPLETEDD #466 entrnace_stub_jmp_target
  cache_pc jmp = entrance_stub_jmp(stub);
  cache_pc tgt;
  ASSERT(jmp != NULL);
  tgt = (cache_pc) PC_RELATIVE_TARGET(jmp+1);
  ASSERT(*jmp == JMP_OPCODE);
  return tgt;
}

cache_pc
entrance_stub_jmp(cache_pc stub)
{
	// COMPLETEDD #465 entrance_stub_jmp
	// ADDME Been Hardcoded need to remove soon
	printf("Starting entrance_stub_jump\n");

//  return (stub + (STUB_COARSE_DIRECT_SIZE32 - JMP_LONG_LENGTH));
	return stub + 4;
}

/* Patch list support routines */
void
init_patch_list(patch_list_t *patch, patch_list_type_t type)
{
	// COMPLETEDD #368 init_patch_list
    patch->num_relocations = 0;
    ASSERT_TRUNCATE(patch->type, ushort, type);
    patch->type = (ushort) type;
}

byte *
emit_fcache_enter(dcontext_t *dcontext, generated_code_t *code, byte *pc)
{
	// INPROCESSS emit_fcache_enter
    return emit_fcache_enter_common(dcontext, code, pc,
                                    true/*absolute*/, false/*!shared*/);
}

/* updates emitted code according to patch list */
static void
patch_emitted_code(dcontext_t *dcontext, patch_list_t *patch, byte *start_pc)
{
	// COMPLETEDD #410 patch_emitted_code
    uint i;
    /* FIXME: can get this as a patch list entry through indirection */
    per_thread_t *pt = (per_thread_t *) dcontext->fragment_field;
    ASSERT(dcontext != GLOBAL_DCONTEXT && dcontext != NULL);

    LOG(THREAD, LOG_EMIT, 2, "patch_emitted_code start_pc="PFX" pt="PFX"\n",
        start_pc);
    if (patch->type != PATCH_TYPE_ABSOLUTE) {
        LOG(THREAD, LOG_EMIT, 2,
            "patch_emitted_code type=%d indirected, nothing to patch\n", patch->type);
        /* FIXME: propagate the check earlier to save the extraneous calls
           to update_indirect_exit_stub and update_indirect_branch_lookup
        */
        return;
    }
    DOLOG(4, LOG_EMIT, {
        print_patch_list(patch);
    });
    for(i=0; i<patch->num_relocations; i++) {
        byte *pc = start_pc + patch->entry[i].where.offset;
        /* value address, (think for example of pt->trace.hash_mask) */
        ptr_uint_t value;
        char *vaddr = NULL;
        if (TEST(PATCH_PER_THREAD, patch->entry[i].patch_flags)) {
            vaddr = (char *)pt + patch->entry[i].value_location_offset;
        } else if (TEST(PATCH_UNPROT_STAT, patch->entry[i].patch_flags)) {
            /* separate the two parts of the stat */
            uint unprot_offs = (uint) (patch->entry[i].value_location_offset) >> 16;
            uint field_offs = (uint) (patch->entry[i].value_location_offset) & 0xffff;

            vaddr = (*((char **)((char *)pt + unprot_offs))) + field_offs;
            LOG(THREAD, LOG_EMIT, 4,
                "patch_emitted_code [%d] value "PFX" => 0x%x 0x%x => "PFX"\n",
                i, patch->entry[i].value_location_offset, unprot_offs, field_offs, vaddr);
        }
        else
            ASSERT_NOT_REACHED();
        ASSERT(TEST(PATCH_OFFSET_VALID, patch->entry[i].patch_flags));
        ASSERT(!TEST(PATCH_MARKER, patch->entry[i].patch_flags));

        if (!TEST(PATCH_TAKE_ADDRESS, patch->entry[i].patch_flags)) {
            /* use value pointed by computed address */
            if (TEST(PATCH_UINT_SIZED, patch->entry[i].patch_flags))
                value = (ptr_uint_t) *((uint *)vaddr);
            else
                value = *(ptr_uint_t*)vaddr;
        } else {
            ASSERT(!TEST(PATCH_UINT_SIZED, patch->entry[i].patch_flags));
            value = (ptr_uint_t)vaddr;   /* use computed address */
        }

        LOG(THREAD, LOG_EMIT, 4,
            "patch_emitted_code [%d] offset="PFX" patch_flags=%d value_offset="PFX
            " vaddr="PFX" value="PFX"\n", i,
            patch->entry[i].where.offset, patch->entry[i].patch_flags,
            patch->entry[i].value_location_offset, vaddr, value);
        if (TEST(PATCH_UINT_SIZED, patch->entry[i].patch_flags)) {
            IF_X64(ASSERT(CHECK_TRUNCATE_TYPE_uint(value)));
            *((uint*)pc) = (uint) value;
        } else
            *((ptr_uint_t *)pc) = value;
        LOG(THREAD, LOG_EMIT, 4,
            "patch_emitted_code: updated pc *"PFX" = "PFX"\n", pc, value);
    }

    STATS_INC(emit_patched_fragments);
    DOSTATS({
        /* PR 217008: avoid gcc warning from truncation assert in XSTATS_ADD_DC */
        int tmp_num = patch->num_relocations;
        STATS_ADD(emit_patched_relocations, tmp_num);
    });
    LOG(THREAD, LOG_EMIT, 4, "patch_emitted_code done\n");
}

static inline void
update_ibl_routine(dcontext_t *dcontext, ibl_code_t *ibl_code)
{
	// COMPLETEDD #411 update_ibl_routine
	printf("Starting update_ibl_routine\n");
    if (!ibl_code->initialized)
        return;
    patch_emitted_code(dcontext, &ibl_code->ibl_patch,
                       ibl_code->indirect_branch_lookup_routine);
    DOLOG(2, LOG_EMIT, {
        const char *ibl_name;
        const char *ibl_brtype;
        ibl_name = get_ibl_routine_name(dcontext,
                                        ibl_code->indirect_branch_lookup_routine,
                                        &ibl_brtype);
        LOG(THREAD, LOG_EMIT, 2, "Just updated indirect branch lookup\n%s_%s:\n",
            ibl_name, ibl_brtype);
        disassemble_with_annotations(dcontext, &ibl_code->ibl_patch,
                                     ibl_code->indirect_branch_lookup_routine,
                                     ibl_code->indirect_branch_lookup_routine + ibl_code->ibl_routine_length);
    });

    if (ibl_code->ibl_head_is_inlined) {
        patch_emitted_code(dcontext, &ibl_code->ibl_stub_patch, ibl_code->inline_ibl_stub_template);
        DOLOG(2, LOG_EMIT, {
            const char *ibl_name;
            const char *ibl_brtype;
            ibl_name = get_ibl_routine_name(dcontext,
                                            ibl_code->indirect_branch_lookup_routine,
                                            &ibl_brtype);
            LOG(THREAD, LOG_EMIT, 2, "Just updated inlined stub indirect branch lookup\n%s_template_%s:\n",
                ibl_name, ibl_brtype);
            disassemble_with_annotations(dcontext, &ibl_code->ibl_stub_patch, ibl_code->inline_ibl_stub_template,
                                         ibl_code->inline_ibl_stub_template + ibl_code->inline_stub_length);
        });
    }
}

void
update_indirect_branch_lookup(dcontext_t *dcontext)
{
	// COMPLETEDD #412 update_indirect_branch_lookup
	printf("Starting update_indirect_branch_lookup\n");
    generated_code_t *code = THREAD_GENCODE(dcontext);

    ibl_branch_type_t branch_type;
    protect_generated_code(code, WRITABLE);
    for (branch_type = IBL_BRANCH_TYPE_START; branch_type < IBL_BRANCH_TYPE_END; branch_type++) {
        update_ibl_routine(dcontext, &code->bb_ibl[branch_type]);
        if (PRIVATE_TRACES_ENABLED() && !DYNAMO_OPTION(shared_trace_ibl_routine))
            update_ibl_routine(dcontext, &code->trace_ibl[branch_type]);
    }
    protect_generated_code(code, READONLY);
}
