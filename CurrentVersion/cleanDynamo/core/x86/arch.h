#ifndef X86_ARCH_H
#define X86_ARCH_H

#include <stddef.h> /* for offsetof */
#include "instr.h" /* for reg_id_t */

void mangle(dcontext_t *dcontext, instrlist_t *ilist, uint flags,
            bool mangle_calls, bool record_translation);

void update_indirect_branch_lookup(dcontext_t *dcontext);

/* Xref the partially overlapping CONTEXT_PRESERVE_XMM */
static inline bool
preserve_xmm_caller_saved(void)
{
	return 0;
}


cache_pc fcache_return_routine(dcontext_t *dcontext);

typedef enum {
    IBL_UNLINKED,
    IBL_DELETE,
    IBL_LINKED,
    IBL_TEMPLATE, /* a template is presumed to be always linked */
    IBL_LINK_STATE_END
} ibl_entry_point_type_t;

/* PR 244737: thread-private uses shared gencode on x64 */
#define USE_SHARED_GENCODE_ALWAYS() IF_X64_ELSE(true, false)
/* PR 212570: on linux we need a thread-shared do_syscall for our vsyscall hook,
 * if we have TLS and support sysenter (PR 361894)
 */
#define USE_SHARED_GENCODE()                                         \
    (USE_SHARED_GENCODE_ALWAYS() || IF_LINUX(IF_HAVE_TLS_ELSE(true, false) ||) \
     SHARED_FRAGMENTS_ENABLED() || DYNAMO_OPTION(shared_trace_ibl_routine))

/* A simple linker to give us indirection for patching after relocating structures */
typedef struct patch_entry_t {
    union {
        instr_t *instr;       /* used before instructions are encoded */
        size_t   offset;      /* offset in instruction stream */
    } where;
    ptr_uint_t value_location_offset; /* location containing value to be updated */
                 /* offset from dcontext->fragment_field (usually pt->trace.field),
                  * or an absolute address */
    ushort patch_flags; /* whether to use the address of location or its value */
    short  instr_offset; /* desired offset within instruction,
                            negative offsets are from end of instruction */
} patch_entry_t;

enum {
    MAX_PATCH_ENTRIES =
#ifdef HASHTABLE_STATISTICS
6 +     /* will need more only for statistics */
#endif
    7, /* we use 5 normally, 7 w/ -atomic_inlined_linking and inlining */
    /* Patch entry flags */
    /* Patch offset entries for dynamic updates from input variables */
    PATCH_TAKE_ADDRESS      = 0x01, /* use computed address if set, value at address otherwise */
    PATCH_PER_THREAD        = 0x02, /* address is relative to the per_thread_t thread local field */
    PATCH_UNPROT_STAT       = 0x04, /* address is (unprot_ht_statistics_t offs << 16) | (stats offs) */
    /* Patch offset markers update an output variable in encode_with_patch_list */
    PATCH_MARKER            = 0x08, /* if set use only as a static marker */
    PATCH_ASSEMBLE_ABSOLUTE = 0x10, /* if set retrieve an absolute pc into given target address,
                                      otherwise relative to start pc */
    PATCH_OFFSET_VALID      = 0x20, /* if set use patch_entry_t.where.offset;
                                     * else patch_entry_t.where.instr */
    PATCH_UINT_SIZED        = 0x40, /* if set value is uint-sized; else pointer-sized */
};

/* we should allow for all {{bb,trace} x {ret,ind call, ind jmp} x {shared, private}} */
/* combinations of routines which are in turn  x {unlinked, linked} */
typedef enum {
    /* FIXME: have a separate flag for private vs shared */
    IBL_BB_SHARED,
    IBL_SOURCE_TYPE_START = IBL_BB_SHARED,
    IBL_TRACE_SHARED,
    IBL_BB_PRIVATE,
    IBL_TRACE_PRIVATE,
    IBL_COARSE_SHARED, /* no coarse private, for now */
    IBL_SOURCE_TYPE_END
} ibl_source_fragment_type_t;

typedef struct patch_list_t {
    ushort num_relocations;
    ushort /* patch_list_type_t */ type;
    patch_entry_t entry[MAX_PATCH_ENTRIES];
} patch_list_t;

typedef enum {
    PATCH_TYPE_ABSOLUTE     = 0x0, /* link with absolute address, updated dynamically */
} patch_list_type_t;

void
set_selfmod_sandbox_offsets(dcontext_t *dcontext);



# define THREAD_GENCODE(dc) get_emitted_routines_code(dc)

void
init_patch_list(patch_list_t *patch, patch_list_type_t type);

/* Defines book-keeping structures needed for an indirect branch lookup routine */
typedef struct ibl_code_t {
    bool initialized:1; /* currently only used for ibl routines */
    bool thread_shared_routine:1;
    bool ibl_head_is_inlined:1;
    byte *indirect_branch_lookup_routine;

    byte *unlinked_ibl_entry;
    byte *target_delete_entry;
    uint ibl_routine_length;
    /* offsets into ibl routine */
    patch_list_t ibl_patch;
    ibl_branch_type_t branch_type;
    ibl_source_fragment_type_t source_fragment_type;

    /* bookkeeping for the inlined ibl stub template, if inlining */
    byte *inline_ibl_stub_template;
    patch_list_t ibl_stub_patch;
    uint inline_stub_length;
    /* for atomic_inlined_linking we store the linkstub twice so need to update
     * two offsets */
    uint inline_linkstub_first_offs;
    uint inline_linkstub_second_offs;
    uint inline_unlink_offs;
    uint inline_linkedjmp_offs;
    uint inline_unlinkedjmp_offs;

} ibl_code_t;
/* Each thread needs its own copy of these routines, but not all
 * routines here are created in a thread-private: we could save space
 * by splitting into two separate structs.
 *
 * On x64, we only have thread-shared generated routines,
 * including do_syscall and shared_syscall and detach's post-syscall
 * continuation (PR 244737).
 */
typedef struct _generated_code_t {
    byte *fcache_enter;
    byte *fcache_return;

    ibl_code_t trace_ibl[IBL_BRANCH_TYPE_END];
    ibl_code_t bb_ibl[IBL_BRANCH_TYPE_END];
    ibl_code_t coarse_ibl[IBL_BRANCH_TYPE_END];

#ifdef RETURN_STACK
    byte *return_lookup;
    byte *unlinked_return;
#endif

    byte *do_syscall;
    uint do_syscall_offs; /* offs of pc after actual syscall instr */

    /* PR 286922: we both need an int and a sys{call,enter} do-syscall for
     * 32-bit apps on 64-bit kernels.  do_syscall is whatever is in
     * vsyscall, while do_int_syscall is hardcoded to use OP_int.
     */
    byte *do_int_syscall;
    uint do_int_syscall_offs; /* offs of pc after actual syscall instr */
    byte *do_clone_syscall;
    uint do_clone_syscall_offs; /* offs of pc after actual syscall instr */

    /* PR 212290: can't be static code in x86.asm since it can't be PIC */
    byte *new_thread_dynamo_start;
#ifdef TRACE_HEAD_CACHE_INCR
    byte *trace_head_incr;
#endif
#ifdef CHECK_RETURNS_SSE2
    byte *pextrw;
    byte *pinsrw;
#endif
    /* For control redirection from a syscall.
     * We could make this shared-only and save some space, if we
     * generated a shared fcache_return in all-private-fragment configs.
     */
    byte *reset_exit_stub;

    /* Coarse-grain fragments don't have linkstubs and need custom routines.
     * Direct exits use entrance stubs that record the target app pc,
     * while coarse indirect stubs record the source cache cti.
     */
    /* FIXME: these two return routines are only needed in the global struct */
    byte *fcache_return_coarse;
    byte *trace_head_return_coarse;

    bool writable;

    /* We store the start of the generated code for simplicity even
     * though it is always right after this struct; if we really need
     * to shrink 4 bytes we can remove this field and replace w/
     * ((char *)TPC_ptr) + sizeof(generated_code_t)
     */
    byte *gen_start_pc; /* start of generated code */
    byte *gen_end_pc; /* end of generated code */
    byte *commit_end_pc; /* end of committed region */
    /* generated code follows, ends at gen_end_pc < commit_end_pc */
} generated_code_t;

extern generated_code_t *shared_code;
/* thread-shared generated code */
byte * emit_fcache_enter_shared(dcontext_t *dcontext, generated_code_t *code, byte *pc);

static inline generated_code_t *
get_shared_gencode(dcontext_t *dcontext _IF_X64(gencode_mode_t mode))
{
	// COMPLETEDD #320 get_shared_gencode
    return shared_code;
}

void protect_generated_code(generated_code_t *code, bool writable);
/* returns the thread private code or GLOBAL thread shared code */
static inline generated_code_t *
get_emitted_routines_code(dcontext_t *dcontext)
{
	// COMPLETEDD #408 get_emitted_routines_code
	printf("Starting get_emitted_routines_code\n");
    generated_code_t *code;
    /* This routine exists only because GLOBAL_DCONTEXT is not a real dcontext
     * structure. Still, useful to wrap all references to private_code. */
    /* PR 244737: thread-private uses only shared gencode on x64 */
    /* PR 253431: to distinguish shared x86 gencode from x64 gencode, a dcontext
     * must be passed in; use get_shared_gencode() for x64 builds */
    if (USE_SHARED_GENCODE_ALWAYS() ||
        (USE_SHARED_GENCODE() && dcontext == GLOBAL_DCONTEXT)) {
        code = get_shared_gencode(dcontext _IF_X64(mode));
    } else {
        ASSERT(dcontext != GLOBAL_DCONTEXT);
        /* NOTE thread private code entry points may also refer to shared
         * routines */
        code = (generated_code_t *) dcontext->private_code;
    }
    return code;
}

cache_pc get_ibl_routine_ex(dcontext_t *dcontext, ibl_entry_point_type_t entry_type,
                            ibl_source_fragment_type_t source_fragment_type,
                            ibl_branch_type_t branch_type _IF_X64(gencode_mode_t mode));

ibl_source_fragment_type_t
get_source_fragment_type(dcontext_t *dcontext, uint fragment_flags);
#endif /* X86_ARCH_H */
