#ifndef _ARCH_EXPORTS_H_
#define _ARCH_EXPORTS_H_ 1

#define REL32_REACHABLE_OFFS(offs) 0

typedef struct dr_jmp_buf_t {
} dr_jmp_buf_t;

typedef struct _local_state_t {
} local_state_t;

// Have no idea what this does so going to set it to 4 like everything else
#ifndef ARM
#define SIZE32_MOV_PTR_IMM_TO_TLS    10
#else
#define SIZE32_MOV_PTR_IMM_TO_TLS		 4;
#endif
#ifdef ARM
unsigned int SET_FLAG(unsigned int);
#endif
#ifndef ARM
# define ATOMIC_INC_suffix(suffix, var) \
    __asm__ __volatile__("lock inc" suffix " %0" : "=m" (var) : : "memory")
# define ATOMIC_INC_int(var) ATOMIC_INC_suffix("l", var)
#endif

#ifndef ARM
# define ATOMIC_COMPARE_EXCHANGE_suffix(suffix, var, compare, exchange) \
   __asm__ __volatile__ ("lock cmpxchg" suffix " %2,%0"         \
                         : "=m" (var)                           \
                         : "a" (compare), "r" (exchange)        \
                         : "memory")
#endif

#ifndef ARM
void our_cpuid(int res[4], int eax);
bool cpuid_supported(void);
#endif

#ifndef ARM
/* PR 264138: we must preserve xmm0-5 if on a 64-bit kernel */
#define XMM_REG_SIZE  16
#define XMM_SLOTS_SIZE  (NUM_XMM_SLOTS*XMM_REG_SIZE)
#define XMM_SAVED_SIZE  (NUM_XMM_SAVED*XMM_REG_SIZE)
#define XMM_ALIGN     16
#endif

# define ATOMIC_COMPARE_EXCHANGE ATOMIC_COMPARE_EXCHANGE_int

#ifdef ARM
void ATOMIC_DEC_int(volatile int * addressOfValue);
void ATOMIC_ADD_int(volatile int * operand1, int operand2);
#endif

int ATOMIC_COMPARE_EXCHANGE_int(volatile int var, int compare, int exchange);

#ifndef ARM
typedef enum {
    IBL_NONE = -1,
    /* N.B.: order determines which table is on 2nd cache line in local_state_t */
    IBL_RETURN = 0, /* returns lookup routine has stricter requirements */
    IBL_BRANCH_TYPE_START = IBL_RETURN,
    IBL_INDCALL,
    IBL_INDJMP,
    IBL_GENERIC           = IBL_INDJMP, /* currently least restrictive */
    /* can double if a generic lookup is needed
       FIXME: remove this and add names for specific needs */
    IBL_SHARED_SYSCALL    = IBL_GENERIC,
    IBL_BRANCH_TYPE_END
} ibl_branch_type_t;
#else
typedef enum {
	IBL_NONE = -1,
	IBL_RETURN = 0,
  IBL_BRANCH_TYPE_START = IBL_RETURN,
  IBL_INDCALL,
  IBL_INDJMP,
	IBL_BRANCH_TYPE_END
} ibl_branch_type_t;
#endif
#ifndef ARM
# define ATOMIC_COMPARE_EXCHANGE_int(var, compare, exchange) \
    ATOMIC_COMPARE_EXCHANGE_suffix("l", var, compare, exchange)
#endif
/* Method of performing system call.
 * We assume that only one method is in use, except for 32-bit applications
 * on 64-bit linux kernels, which use both sys{enter,call} on the vsyscall
 * page and inlined int (PR 286922).
 * For these apps, DR itself and global_do_syscall use int, but we
 * have both a do_syscall for the vsyscall and a separate do_int_syscall
 * (we can't use the vsyscall for some system calls like clone: we could
 * potentially use int for everything if we fixed up the syscall args).
 * The method set in that case is the vsyscall method.
 */
enum {
    SYSCALL_METHOD_UNINITIALIZED,
    SYSCALL_METHOD_INT,
    SYSCALL_METHOD_SYSENTER,
    SYSCALL_METHOD_SYSCALL,
};

#ifndef ARM
# define ATOMIC_4BYTE_WRITE(target, value, hot_patch) do {           \
    /* test that we aren't crossing a cache line boundary */         \
    DODEBUG({                                                        \
        ASSERT(sizeof(value) == 4);                                  \
        if (hot_patch && CROSSES_ALIGNMENT(target, 4, proc_get_cache_line_size())) { \
            STATS_INC(unaligned_patches);                            \
            ASSERT(!DYNAMO_OPTION(pad_jmps));                        \
        }                                                            \
    });                                                              \
    __asm__ __volatile__("xchgl (%0), %1" : : "r" (target), "r" (value) : "memory"); \
  } while (0)
#else
# define ATOMIC_4BYTE_WRITE(target, value, hot_patch) do {           \
    /* test that we aren't crossing a cache line boundary */         \
    DODEBUG({                                                        \
        ASSERT(sizeof(value) == 4);                                  \
        if (hot_patch && CROSSES_ALIGNMENT(target, 4, proc_get_cache_line_size())) { \
            STATS_INC(unaligned_patches);                            \
            ASSERT(!DYNAMO_OPTION(pad_jmps));                        \
        }                                                            \
    });                                                              \
    atomic_4byte_write_asm(target, value);													 \
    } while (0)

void atomic_4byte_write_asm(byte * destination, int value);

#endif

#define IBL_HASH_FUNC_OFFSET_MAX 3

typedef struct {
    app_pc region_start;
    app_pc region_end;
    app_pc start_pc;
    app_pc min_pc;
    app_pc max_pc;
    app_pc bb_end;
    bool contiguous;
    bool overlap;
} overlap_info_t;

/* for use with patch_branch and insert_relative target */
enum {
    NOT_HOT_PATCHABLE=false,
    HOT_PATCHABLE=true
};

// For now just rewrite this all with 4 as the length. Its just easier
#ifndef ARM
enum {
    MAX_INSTR_LENGTH = 17,
    /* size of 32-bit-offset jcc instr, assuming it has no
     * jcc branch hint!
     */
    CBR_LONG_LENGTH  = 6,
    JMP_LONG_LENGTH  = 5,
    JMP_SHORT_LENGTH = 2,
    CBR_SHORT_REWRITE_LENGTH = 9, /* FIXME: use this in mangle.c */
    RET_0_LENGTH     = 1,
    PUSH_IMM32_LENGTH = 5,

    /* size of 32-bit call and jmp instructions w/o prefixes. */
    CTI_IND1_LENGTH    = 2, /* FF D6             call esi                      */
    CTI_IND2_LENGTH    = 3, /* FF 14 9E          call dword ptr [esi+ebx*4]    */
    CTI_IND3_LENGTH    = 4, /* FF 54 B3 08       call dword ptr [ebx+esi*4+8]  */
    CTI_DIRECT_LENGTH  = 5, /* E8 9A 0E 00 00    call 7C8024CB                 */
    CTI_IAT_LENGTH     = 6, /* FF 15 38 10 80 7C call dword ptr ds:[7C801038h] */
    CTI_FAR_ABS_LENGTH = 7, /* 9A 1B 07 00 34 39 call 0739:3400071B            */
                            /* 07                                              */

    INT_LENGTH = 2,
    SYSCALL_LENGTH = 2,
    SYSENTER_LENGTH = 2,
};
#else
//enum {
//    MAX_INSTR_LENGTH = 4,
//    /* size of 32-bit-offset jcc instr, assuming it has no
//     * jcc branch hint!
//     */
//    CBR_LONG_LENGTH  = 4,
//    JMP_LONG_LENGTH  = 4,
//    JMP_SHORT_LENGTH = 4,
//    CBR_SHORT_REWRITE_LENGTH = 4, /* FIXME: use this in mangle.c */
//    RET_0_LENGTH     = 4,
//    PUSH_IMM32_LENGTH = 4,
//
//    /* size of 32-bit call and jmp instructions w/o prefixes. */
//    CTI_IND1_LENGTH    = 4, /* FF D6             call esi                      */
//    CTI_IND2_LENGTH    = 4, /* FF 14 9E          call dword ptr [esi+ebx*4]    */
//    CTI_IND3_LENGTH    = 4, /* FF 54 B3 08       call dword ptr [ebx+esi*4+8]  */
//    CTI_DIRECT_LENGTH  = 4, /* E8 9A 0E 00 00    call 7C8024CB                 */
//    CTI_IAT_LENGTH     = 4, /* FF 15 38 10 80 7C call dword ptr ds:[7C801038h] */
//    CTI_FAR_ABS_LENGTH = 4, /* 9A 1B 07 00 34 39 call 0739:3400071B            */
//                            /* 07                                              */
//
//    INT_LENGTH = 4,
//    SYSCALL_LENGTH = 4,
//    SYSENTER_LENGTH = 4,
//};
#define 	CBR_LONG_LENGTH 	4
#define 	JMP_LONG_LENGTH		4
#define 	JMP_SHORT_LENGTH 	4
#define 	MAX_INSTR_LENGTH 	4
#define 	CBR_SHORT_REWRITE_LENGTH	4
#define		RET_0_LENGTH			4
#define		PUSH_IMM32_LENGTH	4
#define		CTI_IND1_LENGTH		4
#define		CTI_IND2_LENGTH		4
#define		CTI_IND3_LENGTH		4
#define		CTI_DIRECT_LENGTH	4
#define 	CTI_IAT_LENGTH		4
#define 	CTI_FAR_ABS_LENGTH			4
#define		INT_LENGTH				4
#define		SYSCALL_LENGTH		4
#define		SYSENTER_LENGTH		4
#endif

enum {
	Z_FLAG	=	0x40000000
};
struct _ibl_table_t; /* in fragment.h */

#define STUB_COARSE_DIRECT_SIZE32  (SIZE32_MOV_PTR_IMM_TO_TLS + JMP_LONG_LENGTH)
ptr_int_t dynamorio_syscall(uint sysnum, uint num_args, ...);

/* Translation table that records info for translating cache pc to app
 * pc without reading app memory (used when it is unsafe to do so).
 * The table records only translations at change points, so the
 * recreater must interpolate between them, using either a stride of 0
 * if the previous translation entry is marked "identical" or a stride
 * equal to the instruction length as we decode from the cache if the
 * previous entry is !identical=="contiguous".
 */
typedef struct _translation_info_t {
} translation_info_t;

void translation_info_free(dcontext_t *tdcontext, translation_info_t *info);
/* in proc.c -- everything in proc.h is exported so just include it here */
#include "proc.h"

void arch_init(void);
void arch_thread_init(dcontext_t *dcontext);
void arch_thread_exit(dcontext_t *dcontext _IF_WINDOWS(bool detach_stacked_callbacks));
void arch_exit(IF_WINDOWS_ELSE_NP(bool detach_stacked_callbacks, void));
void update_indirect_exit_stub(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);

#ifdef ARM
void ATOMIC_MAX_int(int * firstValue, int secondValue);
#endif
/* atomically adds value to memory location var and returns the sum */
static inline int atomic_add_exchange_int(volatile int *var, int value)
{
	return 0;
}

#ifndef ARM
#define ATOMIC_MAX(type, maxvar, curvar) ATOMIC_MAX_##type(type, maxvar, curvar)
#else
#define ATOMIC_MAX(type, maxvar, curvar) ATOMIC_MAX_##type(maxvar, curvar)
#endif

#ifndef ARM
#define ATOMIC_MAX_int(type, maxvar, curvar) do {               \
        type atomic_max__maxval;                                \
        type atomic_max__curval = (curvar);                     \
        ASSERT(sizeof(int) == sizeof(maxvar));                  \
        ASSERT(sizeof(type) == sizeof(maxvar));                 \
        ASSERT(sizeof(type) == sizeof(curvar));                 \
        do {                                                    \
            atomic_max__maxval = (maxvar);                      \
        } while (atomic_max__maxval < atomic_max__curval &&     \
                 !atomic_compare_exchange_int((int*)&(maxvar),      \
                                              atomic_max__maxval,   \
                                              atomic_max__curval)); \
    } while (0)
#endif

#define ATOMIC_INC(type, var) ATOMIC_INC_##type(var)
#ifndef ARM
# define ATOMIC_DEC_suffix(suffix, var) \
    __asm__ __volatile__("lock dec" suffix " %0" : "=m" (var) : : "memory")
# define ATOMIC_DEC_int(var) ATOMIC_DEC_suffix("l", var)
#endif

#define ATOMIC_DEC(type, var) ATOMIC_DEC_##type(var)
#define RDTSC_LL(llval)
#ifndef ARM
# define GET_STACK_PTR(var) asm("mov %%"IF_X64_ELSE("rsp","esp")", %0" : "=m"(var))
#else
unsigned char * get_stack_ptr();
#endif
#define DIRECT_EXIT_STUB_SIZE(flags) 0
#define STUB_COARSE_DIRECT_SIZE(flags) 0
/* writes debugbreaks into the address range */
#define SET_TO_DEBUG(addr, size)
#define STATS_PAD_JMPS_ADD
#ifdef ARM
#define SET_IF_NOT_ZERO(flag) !SET_FLAG(Z_FLAG)
#else
# define SET_FLAG(cc, flag) __asm__ __volatile__("set"#cc " %0" :"=qm" (flag) )
# define SET_IF_NOT_ZERO(flag) SET_FLAG(nz, flag)
#endif

cache_pc
indirect_linkstub_stub_pc(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);
cache_pc
indirect_linkstub_target(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);

void hashlookup_null_handler(void);

int insert_exit_stub(dcontext_t *dcontext, fragment_t *f,
                     linkstub_t *l, cache_pc stub_pc);

void
link_entrance_stub(dcontext_t *dcontext, cache_pc stub, cache_pc tgt,
                   bool hot_patch, coarse_info_t *info /*OPTIONAL*/);

bool link_direct_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l,
                      fragment_t *targetf, bool hot_patch);
void link_indirect_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l,
                        bool hot_patch);

typedef linkstub_t * (* fcache_enter_func_t) (dcontext_t *dcontext);

uint
coarse_exit_prefix_size(uint flags);

fragment_t *
build_basic_block_fragment(dcontext_t *dcontext, app_pc start_pc,
                           uint initial_flags, bool linked, bool visible
                           _IF_CLIENT(bool for_trace)
                           _IF_CLIENT(instrlist_t **unmangled_ilist));

byte *
emit_coarse_exit_prefix(dcontext_t *dcontext, byte *pc, coarse_info_t *info);

void patch_branch(cache_pc branch_pc, cache_pc target_pc, bool hot_patch);

cache_pc
entrance_stub_target_tag(cache_pc stub);

void unlink_direct_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);

void unlink_indirect_exit(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);

// ADDME NOT sure what this does
typedef struct _spill_state_t {

#ifndef ARM
    /* Four registers are used in the indirect branch lookup routines */
    reg_t xax, xbx, xcx, xdx;    /* general-purpose registers */
#else
    /* Don't really know what these are for but the comment says general purpose registers */
    reg_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11;
#endif
    /* FIXME: move this below the tables to fit more on cache line */
    dcontext_t *dcontext;
} spill_state_t;

/* Scratch space and state required to be easily accessible from
 * in-cache indirect branch lookup routines, store in thread-local storage.
 * Goal is to get it all in one cache line: currently, though, it is
 * 2 lines on 32-byte-line machines, with the call* and jmp* tables and the
 * hashtable stats spilling onto the 2nd line.
 * Even all on one line, shared ibl has a load vs private ibl's hardcoded immed...
 *
 * FIXME: to avoid splitting the mcontext for now these scratch
 * fs:slots are used in fcache, but copied to the
 * mcontext on transitions.  see case 3701
 */
typedef struct _lookup_table_access_t {
    ptr_uint_t hash_mask;
    struct _fragment_entry_t *lookuptable;
} lookup_table_access_t;

// ADDME Again not sure why we need this. Need to understand TLS better
typedef struct _table_stat_state_t {
    /* Organized in mask-table pairs to get both fields for a particular table
     * on the same cache line.
     */
    /* FIXME We can play w/ordering these fields differently or if TLS space is
     * crunched keeping a subset of them in TLS.
     * For example, the ret_trace & indcall_trace tables could be heavily used
     * but if the indjmp table isn't, it might make sense to put the ret_bb
     * table's fields into TLS since ret_bb is likely to be the most heavily
     * used for BB2BB IBL.
     */
    lookup_table_access_t table[IBL_BRANCH_TYPE_END];
    /* FIXME: should allocate this separately so that release and
     * DEBUG builds have the same layout especially when backward
     * aligned entry */
#ifdef HASHTABLE_STATISTICS
    uint stats;
#endif
} table_stat_state_t;

// ADDME Added a spill state and a table state into local state ectended t even though
// I dont think we need them
typedef struct _local_state_extended_t {
    spill_state_t spill_space;
    table_stat_state_t table_space;
} local_state_extended_t;

cache_pc
get_target_delete_entry_pc(dcontext_t *dcontext,
                           struct _ibl_table_t *table);

instrlist_t *
decode_fragment_exact(dcontext_t *dcontext, fragment_t *f, byte *buf,
                      /*IN/OUT*/uint *bufsz, uint target_flags,
                      /*OUT*/uint *dir_exits, /*OUT*/uint *indir_exits);

instrlist_t * build_app_bb_ilist(dcontext_t *dcontext, byte *start_pc, file_t outf);

DR_API
/**
 * Returns the length of \p instr.
 * As a side effect, if instr_ok_to_mangle(instr) and \p instr's raw bits
 * are invalid, encodes \p instr into bytes allocated with
 * instr_allocate_raw_bits(), after which instr is marked as having
 * valid raw bits.
 */
int
instr_length(dcontext_t *dcontext, instr_t *instr);

translation_info_t *record_translation_info(dcontext_t *dcontext, fragment_t *f,
                                            instrlist_t *ilist);

int fragment_prefix_size(uint flags);

void finalize_selfmod_sandbox(dcontext_t *dcontext, fragment_t *f);

void shift_ctis_in_fragment(dcontext_t *dcontext, fragment_t *f, ssize_t shift,
                            cache_pc start, cache_pc end, size_t old_size);

bool app_bb_overlaps(dcontext_t *dcontext, byte *start_pc, uint flags,
                       byte *region_start, byte *region_end, overlap_info_t *info_res);

DR_API
/**
 * Returns the next instr_t in the instrlist_t that contains \p instr.
 * \note The next pointer for an instr_t is inside the instr_t data
 * structure itself, making it impossible to have on instr_t in
 * two different InstrLists (but removing the need for an extra data
 * structure for each element of the instrlist_t).
 */
instr_t*
instr_get_next(instr_t *instr);

cache_pc
entrance_stub_jmp_target(cache_pc stub);

void disassemble_fragment_body(dcontext_t *dcontext, fragment_t *f, file_t outfile);

bool
coarse_is_trace_head(cache_pc stub);

void
unlink_entrance_stub(dcontext_t *dcontext, cache_pc stub, uint flags,
                     coarse_info_t *info /*OPTIONAL*/);

bool
entrance_stub_linked(cache_pc stub, coarse_info_t *info /*OPTIONAL*/);

void disassemble_app_bb(dcontext_t *dcontext, app_pc tag, file_t outfile);

bool mangle_syscall_code(dcontext_t *dcontext, fragment_t *f, byte *pc, bool skip);

/* if hot_patch is true:
 *   The write that inserts the relative target is done atomically so this
 *   function is safe with respect to a thread executing the code containing
 *   this target, presuming that the code in both the before and after states
 *   is valid
 */
byte *
insert_relative_target(byte *pc, cache_pc target, bool hot_patch);

cache_pc
cbr_fallthrough_exit_cti(cache_pc prev_cti_pc);

int exit_stub_size(dcontext_t *dcontext, cache_pc target, uint flags);

bool is_indirect_branch_lookup_routine(dcontext_t *dcontext, cache_pc pc);

void update_generated_hashtable_access(dcontext_t *dcontext);

fcache_enter_func_t get_fcache_enter_shared_routine(dcontext_t *dcontext);

fcache_enter_func_t get_fcache_enter_private_routine(dcontext_t *dcontext);

/* based on machine state, returns which of l1 and l2 must have been taken */
linkstub_t *
linkstub_cbr_disambiguate(dcontext_t *dcontext, fragment_t *f,
                          linkstub_t *l1, linkstub_t *l2);

/* convert link flags to ibl_branch_type_t */
static inline ibl_branch_type_t
extract_branchtype(ushort linkstub_flags)
{
	return 0;
}

cache_pc get_do_syscall_entry(dcontext_t *dcontext);

cache_pc get_do_int_syscall_entry(dcontext_t *dcontext);

cache_pc get_fcache_target(dcontext_t *dcontext);

cache_pc get_do_clone_syscall_entry(dcontext_t *dcontext);

byte * get_global_do_syscall_entry(void);

void copy_mcontext(dr_mcontext_t *src, dr_mcontext_t *dst);

/* in x86.asm */
void call_switch_stack(dcontext_t *dcontext, byte *stack, void (*func) (dcontext_t *),
                       bool free_initstack, bool return_on_return);

int get_syscall_method(void);

void set_fcache_target(dcontext_t *dcontext, cache_pc value);

bool
instr_is_exit_cti(instr_t *instr);

int instr_exit_branch_type(instr_t *instr);

DR_API
/** Return the taken target pc of the (direct branch) instruction. */
app_pc
instr_get_branch_target_pc(instr_t *cti_instr);

DR_API
/** Returns the previous instr_t in the instrlist_t that contains \p instr. */
instr_t*
instr_get_prev(instr_t *instr);

DR_API
/** Set the taken target pc of the (direct branch) instruction. */
void
instr_set_branch_target_pc(instr_t *cti_instr, app_pc pc);

DR_API
/** Return true iff \p instr is to be emitted into the cache. */
bool
instr_ok_to_emit(instr_t *instr);

DR_API
/**
 * Encodes \p instr into the memory at \p pc.
 * Uses the x86/x64 mode stored in instr, not the mode of the current thread.
 * Returns the pc after the encoded instr, or NULL if the encoding failed.
 * If instr is a cti with an instr_t target, the note fields of instr and
 * of the target must be set with the respective offsets of each instr_t!
 * (instrlist_encode does this automatically, if the target is in the list).
 */
byte *
instr_encode(dcontext_t *dcontext, instr_t *instr, byte *pc);

DR_API
/** Sets the user-controlled note field in \p instr to \p value. */
void
instr_set_note(instr_t *instr, void *value);

app_pc find_app_bb_end(dcontext_t *dcontext, byte *start_pc, uint flags);

cache_pc get_unlinked_entry(dcontext_t *dcontext, cache_pc linked_entry);

bool
is_exit_cti_patchable(dcontext_t *dcontext, instr_t *inst, uint frag_flags);

/* evaluates to true if region crosses at most 1 padding boundary */
#define WITHIN_PAD_REGION(lower, upper) 0

#ifdef ARM
void ATOMIC_INC_int(volatile int * addressofInt);
#endif

/* offset of the patchable region from the end of a cti */
#define CTI_PATCH_OFFSET 0

/* the most bytes we'll need to shift a patchable location for -pad_jmps */
#define MAX_PAD_SIZE 0

bool
is_exit_cti_stub_patchable(dcontext_t *dcontext, instr_t *inst,
                           uint frag_flags);

uint
nop_pad_ilist(dcontext_t *dcontext, fragment_t *f, instrlist_t *ilist, bool emitting);

void insert_fragment_prefix(dcontext_t *dcontext, fragment_t *f);

byte *
pad_for_exitstub_alignment(dcontext_t *dcontext, linkstub_t *l, fragment_t *f,
                           byte *startpc);

int
linkstub_unlink_entry_offset(dcontext_t *dcontext, fragment_t *f, linkstub_t *l);

#ifndef ARM
/* writes nops into the address range */
#define SET_TO_NOPS(addr, size) memset(addr, 0x90, size)
#else
#define SET_TO_NOPS(addr, size) memset(addr, 0x0320F000, size);
#endif

#ifndef ARM
# define SPINLOCK_PAUSE()   __asm__ __volatile__("pause")
#else
# define SPINLOCK_PAUSE()
#endif
/* Atomically increments *var by 1
 * Returns true if the resulting value is zero, otherwise returns false
 */
static inline bool atomic_inc_and_test(volatile int *var)
{
	/* Atomically increments *var by 1
	 * Returns true if the resulting value is zero, otherwise returns false
	 */
//	printf("Starting atomic_inc_and_test\n");
#ifndef ARM
  unsigned char c;
#else
  unsigned int c;
#endif

#ifndef ARM
	ATOMIC_INC(int, *var);
#else
	ATOMIC_INC(int, var);
#endif
	/* flags should be set according to resulting value, now we convert that back to C */
#ifdef ARM
	c = SET_IF_NOT_ZERO();
#else
	SET_IF_NOT_ZERO(c);
#endif
	/* FIXME: we add an extra memory reference to a local,
	although we could put the return value in EAX ourselves */
//	printf("Endinf atomic_inc_and_test\n");
	return c == 0;
}



/* returns true if var was equal to compare, and now is equal to exchange,
   otherwise returns false
 */
static inline bool atomic_compare_exchange_int(volatile int *var,
                                               int compare, int exchange)
{
	// COMPLETEDD #436 atomic_compare_exchange_int
//	  printf("Starting atomic_compare_exchange_int\n");
#ifndef ARM
  unsigned char c;
#else
  unsigned int c;
#endif
    ATOMIC_COMPARE_EXCHANGE(var, compare, exchange);
    /* ZF is set if matched, all other flags are as if a normal compare happened */
    /* we convert ZF value back to C */
#ifdef ARM
	c = SET_IF_NOT_ZERO();
#else
	SET_IF_NOT_ZERO(c);
#endif
    /* FIXME: we add an extra memory reference to a local,
       although we could put the return value in EAX ourselves */
//	  printf("Ending atomic_compare_exchange_int\n");
    return c == 0;
}

/* Atomically decrements *var by 1
 * Returns true if the resulting value is zero, otherwise returns false
 */
static inline bool atomic_dec_becomes_zero(volatile int *var)
{
	// COMPLETEDD #63 atomic_dec_becomes_zero
	printf("Starting atomic_dec_becomes_zero \n");
#ifndef ARM
  unsigned char c;
#else
  unsigned int c;
#endif

#ifndef ARM
    ATOMIC_DEC(int, *var);
#else
    ATOMIC_DEC(int, var);
#endif
    /* result should be set according to value after change, now we convert that back to C */
#ifdef ARM
	c = SET_IF_NOT_ZERO();
#else
	SET_IF_NOT_ZERO(c);
#endif
    /* FIXME: we add an extra memory reference to a local,
       although we could put the return value in EAX ourselves */
    return c == 0;
}

# define GET_FRAME_PTR(var) var = 0
#define atomic_compare_exchange atomic_compare_exchange_int

#define DR_SETJMP(buf) (dr_setjmp(buf))

int dr_setjmp(dr_jmp_buf_t *buf);

static inline bool atomic_dec_and_test(volatile int *var)
{
#ifndef ARM
  unsigned char c;
#else
  unsigned int c;
#endif
  printf("Starting atomic_dec_and_test\n");
  printf("Var = %x\n", *var);
#ifndef ARM
  ATOMIC_DEC(int, *var);
#else
  ATOMIC_DEC(int, var);
#endif
  /* flags should be set according to resulting value, now we convert that back to C */
//  printf("Var = %x\n", *var);
  #ifdef ARM
  c = SET_IF_NOT_ZERO();
  #else
  SET_IF_NOT_ZERO(c);
  #endif
  /* FIXME: we add an extra memory reference to a local,
     although we could put the return value in EAX ourselves */

  printf("Ending atomic_dec_and_test C Equals %x \n", c);
  return c == 0;
}

DR_API
/**
 * Performs instr_free() and then deallocates the thread-local heap
 * storage for \p instr.
 */
void
instr_destroy(dcontext_t *dcontext, instr_t *instr);

DR_API
/** Sets the next field of \p instr to point to \p next. */
void
instr_set_next(instr_t *instr, instr_t *next);

DR_API
/** Sets the prev field of \p instr to point to \p prev. */
void
instr_set_prev(instr_t *instr, instr_t *prev);

DR_API
/**
 * Gets the value of the user-controlled note field in \p instr.
 * \note Important: is also used when emitting for targets that are other
 * instructions, so make sure to clear or set appropriately the note field
 * prior to emitting.
 */
void *
instr_get_note(instr_t *instr);

bool
coarse_is_entrance_stub(cache_pc stub);

void instr_shift_raw_bits(instr_t *instr, ssize_t offs);

uint
extend_trace_pad_bytes(fragment_t *add_frag);

/* An upper bound on instructions added to a bb when added to a trace,
 * which is of course highest for the case of indirect branch mangling.
 * Normal lea, jecxz, lea is 14, NATIVE_RETURN can get above 20,
 * but this should cover everything, fine to be well above, this is
 * only used to keep below the maximum trace size for the next bb,
 * we calculate the exact size in fixup_last_cti().
 *
 * For x64 we have to increase this (PR 333576 hit this):
 *    +19   L4  65 48 89 0c 25 10 00 mov    %rcx -> %gs:0x10
 *              00 00
 *    +28   L4  48 8b c8             mov    %rax -> %rcx
 *    +31   L4  e9 1b e2 f6 ff       jmp    $0x00000000406536e0 <shared_bb_ibl_indjmp>
 *    (+36)
 *   =>
 *    +120  L0  65 48 89 0c 25 10 00 mov    %rcx -> %gs:0x10
 *              00 00
 *    +129  L0  48 8b c8             mov    %rax -> %rcx
 *    +132  L3  65 48 a3 00 00 00 00 mov    %rax -> %gs:0x00
 *              00 00 00 00
 *    +143  L3  48 b8 23 24 93 28 00 mov    $0x0000000028932423 -> %rax
 *              00 00 00
 *    +153  L3  65 48 a3 08 00 00 00 mov    %rax -> %gs:0x08
 *              00 00 00 00
 *    +164  L3  9f                   lahf   -> %ah
 *    +165  L3  0f 90 c0             seto   -> %al
 *    +168  L3  65 48 3b 0c 25 08 00 cmp    %rcx %gs:0x08
 *              00 00
 *    +177  L4  0f 85 a9 d7 f6 ff    jnz    $0x000000004065312f <shared_trace_cmp_indjmp>
 *    +183  L3  65 48 8b 0c 25 10 00 mov    %gs:0x10 -> %rcx
 *              00 00
 *    +192  L3  04 7f                add    $0x7f %al -> %al
 *    +194  L3  9e                   sahf   %ah
 *    +195  L3  65 48 a1 00 00 00 00 mov    %gs:0x00 -> %rax
 *              00 00 00 00
 *    +206
 *
 *    (36-19)=17 vs (206-120)=86 => 69 bytes.  was 65 bytes prior to PR 209709!
 *    usually 3 bytes smaller since don't need to restore eflags.
 */
#define TRACE_CTI_MANGLE_SIZE_UPPER_BOUND 0

int append_trace_speculate_last_ibl(dcontext_t *dcontext, instrlist_t *trace,
                                    app_pc speculate_next_tag, bool record_translation);

uint extend_trace(dcontext_t *dcontext, fragment_t *f, linkstub_t *prev_l);

void
bb_build_abort(dcontext_t *dcontext, bool clean_vmarea);

/* Update info pointer in exit prefixes */
void
patch_coarse_exit_prefix(dcontext_t *dcontext, coarse_info_t *info);

DR_API
/**
 * Returns an initialized instr_t allocated on the thread-local heap.
 * Sets the x86/x64 mode of the returned instr_t to the mode of dcontext.
 */
instr_t*
instr_create(dcontext_t *dcontext);

DR_API
/**
 * Performs both instr_free() and instr_init().
 * \p instr must have been initialized.
 */
void
instr_reset(dcontext_t *dcontext, instr_t *instr);

uint
coarse_indirect_stub_size(coarse_info_t *info);

byte *
insert_relative_jump(byte *pc, cache_pc target, bool hot_patch);

cache_pc
entrance_stub_jmp_target(cache_pc stub);

cache_pc
entrance_stub_jmp(cache_pc stub);

/* Does the syscall instruction always return to the invocation point? */
bool does_syscall_ret_to_callsite(void);

bool is_after_do_syscall_addr(dcontext_t *dcontext, cache_pc pc);
bool in_generated_routine(dcontext_t *dcontext, cache_pc pc);

typedef enum {
    RECREATE_FAILURE,
    RECREATE_SUCCESS_PC,
    RECREATE_SUCCESS_STATE,
} recreate_success_t;

/* state translation for faults and thread relocation */
app_pc recreate_app_pc(dcontext_t *tdcontext, cache_pc pc, fragment_t *f);

cache_pc get_reset_exit_stub(dcontext_t *dcontext);

recreate_success_t
recreate_app_state(dcontext_t *tdcontext, dr_mcontext_t *mcontext, bool restore_memory);

# define ATOMIC_ADD(type, var, val) ATOMIC_ADD_##type(var, val)
#ifndef ARM
# define ATOMIC_ADD_int(var, val) ATOMIC_ADD_suffix("l", var, val)
# define ATOMIC_ADD_suffix(suffix, var, value)                 \
   __asm__ __volatile__("lock add" suffix " %1, %0"            \
                        : "=m" (var) : "ri" (value) : "memory")
#endif
#define TLS_DCONTEXT_SLOT        0

thread_id_t dynamorio_clone(uint flags, byte *newsp, void *ptid, void *tls,
                            void *ctid, void (*func)(void));

bool should_syscall_method_be_sysenter(void);

void cleanup_and_terminate(dcontext_t *dcontext, int sysnum,
                           ptr_uint_t sys_arg1, ptr_uint_t sys_arg2, bool exitproc);

void dynamorio_nonrt_sigreturn(void);

DR_API
/**
 * Deallocates all memory that was allocated by \p instr.  This
 * includes raw bytes allocated by instr_allocate_raw_bits() and
 * operands allocated by instr_set_num_opnds().  Does not deallocate
 * the storage for \p instr itself.
 */
void
instr_free(dcontext_t *dcontext, instr_t *instr);

void dynamorio_sigreturn(void);

#define DR_LONGJMP(buf, val)
/* dumps callstack for current pc and ebp */
void dump_dr_callstack(file_t outfile);

DR_API
/** Initializes \p instr.
 * Sets the x86/x64 mode of \p instr to the mode of dcontext.
 */
void
instr_init(dcontext_t *dcontext, instr_t *instr);

/* This completely optimizable routine is the only place where we
 * allow a data pointer to be converted to a function pointer to allow
 * better type-checking for the rest of our C code
 */
/* on x86 function pointers and data pointers are interchangeable */

static inline
generic_func_t
convert_data_to_function(void *data_ptr)
{
    return (generic_func_t)data_ptr;
}

bool in_context_switch_code(dcontext_t *dcontext, cache_pc pc);

bool in_indirect_branch_lookup_code(dcontext_t *dcontext, cache_pc pc);

DR_API
/**
 * Return true iff \p instr is not a meta-instruction
 * (see instr_set_ok_to_mangle() for more information).
 */
bool
instr_ok_to_mangle(instr_t *instr);

DR_API
/**
 * Sets \p instr to "ok to mangle" if \p val is true and "not ok to
 * mangle" if \p val is false.  An instruction that is "not ok to
 * mangle" is treated by DR as a "meta-instruction", distinct from
 * normal application instructions, and is not mangled in any way.
 * This is necessary to have DR not create an exit stub for a direct
 * jump.  All non-meta instructions that are added to basic blocks or
 * traces should have their translation fields set (via
 * #instr_set_translation(), or the convenience routine
 * #instr_set_meta_no_translation()) when recreating state at a fault;
 * meta instructions should not fault and are not considered
 * application instructions but rather added instrumentation code (see
 * #dr_register_bb_event() for further information on recreating).
 *
 * \note For meta-instructions that can fault but only when accessing
 * client memory and that never access application memory, the
 * "meta-instruction that can fault" property can be set via
 * #instr_set_meta_may_fault to avoid incurring the cost of added
 * sandboxing checks that look for changes to application code.
 */
void
instr_set_ok_to_mangle(instr_t *instr, bool val);
#endif
