/* **********************************************************
 * Copyright (c) 2002-2009 VMware, Inc.  All rights reserved.
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

#ifndef _DR_DEFINES_H_
#define _DR_DEFINES_H_ 1

/****************************************************************************
 * GENERAL TYPEDEFS AND DEFINES
 */

/**
 * @file dr_defines.h
 * @brief Basic defines and type definitions.
 */

/* A client's target operating system and architecture must be specified. */
#if (!defined(LINUX) && !defined(WINDOWS)) || (defined(LINUX) && defined(WINDOWS))
# error Target operating system unspecified: must define either WINDOWS xor LINUX
#endif

#if (!defined(X86_64) && !defined(X86_32)) || (defined(X86_64) && defined(X86_32))
# error Target architecture unspecified: must define either X86_64 xor X86_32
#endif

#if defined(X86_64) && !defined(X64)
# define X64
#endif

#ifdef WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <winbase.h>
#else
#  include <stdio.h>
#  include <stdlib.h>
#endif

#ifdef WINDOWS
/* allow nameless struct/union */
#  pragma warning(disable: 4201)
#endif

#ifdef WINDOWS
#  define DR_EXPORT __declspec(dllexport)
#  define LINK_ONCE __declspec(selectany)
#  define ALIGN_VAR(x) __declspec(align(x))
#  define inline __inline
#  define INLINE_FORCED __forceinline
#else
/* We assume gcc is being used.  If the client is using -fvisibility
 * (in gcc >= 3.4) to not export symbols by default, setting
 * USE_VISIBILITY_ATTRIBUTES will properly export.
 */
#  ifdef USE_VISIBILITY_ATTRIBUTES
#    define DR_EXPORT __attribute__ ((visibility ("default")))
#  else
#    define DR_EXPORT
#  endif
#  define LINK_ONCE __attribute__ ((weak))
#  define ALIGN_VAR(x) __attribute__ ((aligned (x)))
#  define inline __inline__
#  define INLINE_FORCED inline
#endif

/** Cross-platform maximum file path length. */
#define MAXIMUM_PATH      260 


#ifndef NULL
#  define NULL (0)
#endif

#ifndef __cplusplus
#  ifndef DR_DO_NOT_DEFINE_bool
typedef int bool;
#  endif
#  define true  (1)
#  define false (0)
#endif

#ifndef DR_DO_NOT_DEFINE_uint
typedef unsigned int uint;
#endif
#ifndef DR_DO_NOT_DEFINE_ushort
typedef unsigned short ushort;
#endif
#ifndef DR_DO_NOT_DEFINE_byte
typedef unsigned char byte;
#endif
#ifndef DR_DO_NOT_DEFINE_sbyte
typedef signed char sbyte;
#endif
typedef byte * app_pc;

typedef void (*generic_func_t) ();

#ifdef WINDOWS
#  ifndef DR_DO_NOT_DEFINE_uint64
typedef unsigned __int64 uint64;
#  endif
#  ifndef DR_DO_NOT_DEFINE_int64
typedef __int64 int64;
#  endif
#  ifdef X64
typedef __int64 ssize_t;
#  else
typedef int ssize_t;
#  endif
#  define INT64_FORMAT "I64"
#else /* Linux */
#  ifdef X64
#    ifndef DR_DO_NOT_DEFINE_uint64
typedef unsigned long int uint64;
#    endif
#    ifndef DR_DO_NOT_DEFINE_int64
typedef long int int64;
#    endif
#    define INT64_FORMAT "l"
#  else
#    ifndef DR_DO_NOT_DEFINE_uint64
typedef unsigned long long int uint64;
#    endif
#    ifndef DR_DO_NOT_DEFINE_int64
typedef long long int int64;
#    endif
#    define INT64_FORMAT "ll"
#  endif
#endif

/* a register value: could be of any type; size is what matters. */
#ifdef X64
typedef uint64 reg_t;
#else
typedef uint reg_t;
#endif
/* integer whose size is based on pointers: ptr diff, mask, etc. */
typedef reg_t ptr_uint_t;
#ifdef X64
typedef int64 ptr_int_t;
#else
typedef int ptr_int_t;
#endif
/* for memory region sizes, use size_t */

/**
 * Application offset from module base.
 * PE32+ modules are limited to 2GB, but not ELF x64 med/large code model.
 */
typedef size_t app_rva_t;

#ifdef WINDOWS
typedef ptr_uint_t thread_id_t;
typedef ptr_uint_t process_id_t;
#else /* Linux */
typedef pid_t thread_id_t;
typedef pid_t process_id_t;
#endif

#ifdef WINDOWS
/* since a FILE cannot be used outside of the DLL it was created in,
 * we have to use HANDLE on Windows
 * we hide the distinction behind the file_t type
 */
typedef HANDLE file_t;
/** The sentinel value for an invalid file_t. */
#  define INVALID_FILE INVALID_HANDLE_VALUE
/* dr_get_stdout_file and dr_get_stderr_file return errors as 
 * INVALID_HANDLE_VALUE.  We leave INVALID_HANDLE_VALUE as is, 
 * since it equals INVALID_FILE
 */
/** The file_t value for standard output. */
#  define STDOUT (dr_get_stdout_file())
/** The file_t value for standard error. */
#  define STDERR (dr_get_stderr_file())
/** The file_t value for standard input. */
#  define STDIN  (dr_get_stdin_file())
#endif

#ifdef LINUX
typedef int file_t;
/** The sentinel value for an invalid file_t. */
#  define INVALID_FILE -1
/** Allow use of stdout after the application closes it. */
extern file_t our_stdout;
/** Allow use of stderr after the application closes it. */
extern file_t our_stderr;
/** Allow use of stdin after the application closes it. */
extern file_t our_stdin;
/** The file_t value for standard output. */
#  define STDOUT (our_stdout == INVALID_FILE ? stdout->_fileno : our_stdout)
/** The file_t value for standard error. */
#  define STDERR (our_stderr == INVALID_FILE ? stderr->_fileno : our_stderr)
/** The file_t value for standard error. */
#  define STDIN  (our_stdin == INVALID_FILE ? stdin->_fileno : our_stdin)
#endif

/**
 * ID used to uniquely identify a client.  This value is set at
 * client registration and passed to the client in dr_init().
 */
typedef uint client_id_t;

/**
 * Internal structure of opnd_t is below abstraction layer.
 * But compiler needs to know field sizes to copy it around
 */
typedef struct {
#ifdef X64
    uint black_box_uint;
    uint64 black_box_uint64;
#else
    uint black_box_uint[3];
#endif
} opnd_t;

/**
 * Internal structure of instr_t is below abstraction layer, but we
 * provide its size so that it can be used in stack variables
 * instead of always allocated on the heap.
 */
typedef struct {
#ifdef X64
    uint black_box_uint[26];
#else
    uint black_box_uint[16];
#endif
} instr_t;

# define IN /* marks input param */
# define OUT /* marks output param */
# define INOUT /* marks input+output param */


#ifdef X64
# define IF_X64(x) x
# define IF_X64_ELSE(x, y) x
# define IF_X64_(x) x,
# define _IF_X64(x) , x
# define IF_NOT_X64(x)
# define _IF_NOT_X64(x)
#else
# define IF_X64(x)
# define IF_X64_ELSE(x, y) y
# define IF_X64_(x)
# define _IF_X64(x)
# define IF_NOT_X64(x) x
# define _IF_NOT_X64(x) , x
#endif

#define UINT64_FORMAT_CODE INT64_FORMAT"u"
#define INT64_FORMAT_CODE INT64_FORMAT"d"
#define UINT64_FORMAT_STRING "%"UINT64_FORMAT_CODE
#define INT64_FORMAT_STRING "%"INT64_FORMAT_CODE
#define HEX64_FORMAT_STRING "%"INT64_FORMAT"x"
#define ZHEX64_FORMAT_STRING "%016"INT64_FORMAT"x"
#define ZHEX32_FORMAT_STRING "%08x"
#define HEX32_FORMAT_STRING "%x"
/* Convenience defines for cross-platform printing */
#ifdef X64
# define PFMT ZHEX64_FORMAT_STRING
# define PIFMT HEX64_FORMAT_STRING
# define SZFMT INT64_FORMAT_STRING
#else
# define PFMT ZHEX32_FORMAT_STRING
# define PIFMT HEX32_FORMAT_STRING
# define SZFMT "%d"
#endif

#define PFX "0x"PFMT
#define PIFX "0x"PIFMT

/** 128-bit XMM register. */
typedef union _dr_xmm_t {
    uint64 u64[2]; /**< Representation as 2 64-bit integers. */
    uint   u32[4]; /**< Representation as 4 32-bit integers. */
    byte   u8[16]; /**< Representation as 8 8-bit integers. */
    reg_t  reg[IF_X64_ELSE(2,4)]; /**< Representation as 2 or 4 registers. */
} dr_xmm_t;

#ifdef X64
# ifdef WINDOWS
#  define NUM_XMM_SLOTS 6 /** Number of xmm reg slots in dr_mcontext_t */ /* xmm0-5 */
# else
#  define NUM_XMM_SLOTS 16 /** Number of xmm reg slots in dr_mcontext_t */ /* xmm0-15 */
# endif
#else
# define NUM_XMM_SLOTS 8 /** Number of xmm reg slots in dr_mcontext_t */ /* xmm0-7 */
#endif

/**
 * Machine context structure.
 */
#ifndef ARM
typedef struct _dr_mcontext_t {
    union {
        reg_t xdi; /**< platform-independent name for full rdi/edi register */
        reg_t IF_X64_ELSE(rdi, edi); /**< platform-dependent name for rdi/edi register */
    }; /* anonymous union of alternative names for rdi/edi register */
    union {
        reg_t xsi; /**< platform-independent name for full rsi/esi register */
        reg_t IF_X64_ELSE(rsi, esi); /**< platform-dependent name for rsi/esi register */
    }; /* anonymous union of alternative names for rsi/esi register */
    union {
        reg_t xbp; /**< platform-independent name for full rbp/ebp register */
        reg_t IF_X64_ELSE(rbp, ebp); /**< platform-dependent name for rbp/ebp register */
    }; /* anonymous union of alternative names for rbp/ebp register */
    union {
        reg_t xsp; /**< platform-independent name for full rsp/esp register */
        reg_t IF_X64_ELSE(rsp, esp); /**< platform-dependent name for rsp/esp register */
    }; /* anonymous union of alternative names for rsp/esp register */
    union {
        reg_t xbx; /**< platform-independent name for full rbx/ebx register */
        reg_t IF_X64_ELSE(rbx, ebx); /**< platform-dependent name for rbx/ebx register */
    }; /* anonymous union of alternative names for rbx/ebx register */
    union {
        reg_t xdx; /**< platform-independent name for full rdx/edx register */
        reg_t IF_X64_ELSE(rdx, edx); /**< platform-dependent name for rdx/edx register */
    }; /* anonymous union of alternative names for rdx/edx register */
    union {
        reg_t xcx; /**< platform-independent name for full rcx/ecx register */
        reg_t IF_X64_ELSE(rcx, ecx); /**< platform-dependent name for rcx/ecx register */
    }; /* anonymous union of alternative names for rcx/ecx register */
    union {
        reg_t xax; /**< platform-independent name for full rax/eax register */
        reg_t IF_X64_ELSE(rax, eax); /**< platform-dependent name for rax/eax register */
    }; /* anonymous union of alternative names for rax/eax register */
#ifdef X64
    reg_t r8;  /**< r8 register. \note For 64-bit DR builds only. */
    reg_t r9;  /**< r9 register. \note For 64-bit DR builds only. */
    reg_t r10; /**< r10 register. \note For 64-bit DR builds only. */
    reg_t r11; /**< r11 register. \note For 64-bit DR builds only. */
    reg_t r12; /**< r12 register. \note For 64-bit DR builds only. */
    reg_t r13; /**< r13 register. \note For 64-bit DR builds only. */
    reg_t r14; /**< r14 register. \note For 64-bit DR builds only. */
    reg_t r15; /**< r15 register. \note For 64-bit DR builds only. */
#endif
    /**
     * The SSE registers xmm0-xmm5 (-xmm15 on Linux) are volatile
     * (caller-saved) for 64-bit and WOW64, and are actually zeroed out on
     * Windows system calls.  These fields are ignored for 32-bit processes
     * that are not WOW64, or if the underlying processor does not support
     * SSE.  Use dr_mcontext_xmm_fields_valid() to determine whether the
     * fields are valid.
     */
    dr_xmm_t xmm[NUM_XMM_SLOTS];
    union {
        reg_t xflags; /**< platform-independent name for full rflags/eflags register */
        reg_t IF_X64_ELSE(rflags, eflags); /**< platform-dependent name for
                                                rflags/eflags register */
    }; /* anonymous union of alternative names for rflags/eflags register */
    /*
     * Anonymous union of alternative names for the program counter / 
     * instruction pointer (eip/rip).  This field is not always set or 
     * read by all API routines.
     */ 
    union {
        byte *xip; /**< platform-independent name for full rip/eip register */
        byte *pc; /**< platform-independent alt name for full rip/eip register */
        byte *IF_X64_ELSE(rip, eip); /**< platform-dependent name for rip/eip register */
    };
} dr_mcontext_t;
#else
typedef struct _dr_mcontext_t {
		reg_t r0;
		reg_t r1;
		reg_t r2;
		reg_t r3;
		reg_t r4;
		reg_t r5;
		reg_t r6;
		reg_t r7;
		reg_t r8;
		reg_t r9;
		reg_t r10;
		reg_t r11;
		reg_t r12;
		reg_t r13;
		reg_t r14;
		byte * pc;
		reg_t apsr;
} dr_mcontext_t;
#endif


typedef struct _instr_list_t instrlist_t;
typedef struct _module_data_t module_data_t;


/**
 * Structure written by dr_get_time() to specify the current time. 
 */
typedef struct {
    uint year;         /**< */
    uint month;        /**< */
    uint day_of_week;  /**< */
    uint day;          /**< */
    uint hour;         /**< */
    uint minute;       /**< */
    uint second;       /**< */
    uint milliseconds; /**< */
} dr_time_t;



/**
 * Returns the length of \p instr.
 * As a side effect, if instr_ok_to_mangle(instr) and \p instr's raw bits
 * are invalid, encodes \p instr into bytes allocated with
 * instr_allocate_raw_bits(), after which instr is marked as having
 * valid raw bits.
 */
int
instr_length(void *drcontext, instr_t *instr);

/**
 * Returns the next instr_t in the instrlist_t that contains \p instr.
 * \note The next pointer for an instr_t is inside the instr_t data
 * structure itself, making it impossible to have on instr_t in
 * two different InstrLists (but removing the need for an extra data
 * structure for each element of the instrlist_t).
 */
instr_t*
instr_get_next(instr_t *instr);

/** Return the taken target pc of the (direct branch) instruction. */
app_pc
instr_get_branch_target_pc(instr_t *cti_instr);

/** Returns the previous instr_t in the instrlist_t that contains \p instr. */
instr_t*
instr_get_prev(instr_t *instr);

/** Set the taken target pc of the (direct branch) instruction. */
void
instr_set_branch_target_pc(instr_t *cti_instr, app_pc pc);

/** Return true iff \p instr is to be emitted into the cache. */
bool
instr_ok_to_emit(instr_t *instr);

/**
 * Encodes \p instr into the memory at \p pc.
 * Uses the x86/x64 mode stored in instr, not the mode of the current thread.
 * Returns the pc after the encoded instr, or NULL if the encoding failed.
 * If instr is a cti with an instr_t target, the note fields of instr and
 * of the target must be set with the respective offsets of each instr_t!
 * (instrlist_encode does this automatically, if the target is in the list).
 */
byte *
instr_encode(void *drcontext, instr_t *instr, byte *pc);

/** Sets the user-controlled note field in \p instr to \p value. */
void
instr_set_note(instr_t *instr, void *value);

/**
 * Performs instr_free() and then deallocates the thread-local heap
 * storage for \p instr.
 */
void
instr_destroy(void *drcontext, instr_t *instr);

/** Sets the next field of \p instr to point to \p next. */
void
instr_set_next(instr_t *instr, instr_t *next);

/** Sets the prev field of \p instr to point to \p prev. */
void
instr_set_prev(instr_t *instr, instr_t *prev);

/**
 * Gets the value of the user-controlled note field in \p instr.
 * \note Important: is also used when emitting for targets that are other
 * instructions, so make sure to clear or set appropriately the note field
 * prior to emitting.
 */
void *
instr_get_note(instr_t *instr);

/**
 * Returns an initialized instr_t allocated on the thread-local heap.
 * Sets the x86/x64 mode of the returned instr_t to the mode of dcontext.
 */
instr_t*
instr_create(void *drcontext);

/**
 * Performs both instr_free() and instr_init().
 * \p instr must have been initialized.
 */
void
instr_reset(void *drcontext, instr_t *instr);

/**
 * Deallocates all memory that was allocated by \p instr.  This
 * includes raw bytes allocated by instr_allocate_raw_bits() and
 * operands allocated by instr_set_num_opnds().  Does not deallocate
 * the storage for \p instr itself.
 */
void
instr_free(void *drcontext, instr_t *instr);

/** Initializes \p instr.
 * Sets the x86/x64 mode of \p instr to the mode of dcontext.
 */
void
instr_init(void *drcontext, instr_t *instr);

/**
 * Return true iff \p instr is not a meta-instruction
 * (see instr_set_ok_to_mangle() for more information).
 */
bool
instr_ok_to_mangle(instr_t *instr);

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


/** Returns the cache line size in bytes of the processor. */
size_t
proc_get_cache_line_size(void);

/** Returns one of the VENDOR_ constants. */
uint
proc_get_vendor(void);

/** Returns n >= \p sz such that n is a multiple of the cache line size. */
ptr_uint_t
proc_bump_to_end_of_cache_line(ptr_uint_t sz);
/* Remember that we add extended family to family as Intel suggests */
#define FAMILY_LLANO        18 /**< proc_get_family() processor family: AMD Llano */
#define FAMILY_ITANIUM_2_DC 17 /**< proc_get_family() processor family: Itanium 2 DC */
#define FAMILY_K8_MOBILE    17 /**< proc_get_family() processor family: AMD K8 Mobile */
#define FAMILY_ITANIUM_2    16 /**< proc_get_family() processor family: Itanium 2 */
#define FAMILY_K8L          16 /**< proc_get_family() processor family: AMD K8L */
#define FAMILY_K8           15 /**< proc_get_family() processor family: AMD K8 */
#define FAMILY_PENTIUM_4    15 /**< proc_get_family() processor family: Pentium 4 */
#define FAMILY_P4           15 /**< proc_get_family() processor family: P4 family */
#define FAMILY_ITANIUM       7 /**< proc_get_family() processor family: Itanium */
/* Pentium Pro, Pentium II, Pentium III, Athlon, Pentium M, Core, Core 2, Core i7 */
#define FAMILY_P6            6 /**< proc_get_family() processor family: P6 family */
#define FAMILY_CORE_I7       6 /**< proc_get_family() processor family: Core i7 */
#define FAMILY_CORE_2        6 /**< proc_get_family() processor family: Core 2 */
#define FAMILY_CORE          6 /**< proc_get_family() processor family: Core */
#define FAMILY_PENTIUM_M     6 /**< proc_get_family() processor family: Pentium M */
#define FAMILY_PENTIUM_3     6 /**< proc_get_family() processor family: Pentium 3 */
#define FAMILY_PENTIUM_2     6 /**< proc_get_family() processor family: Pentium 2 */
#define FAMILY_PENTIUM_PRO   6 /**< proc_get_family() processor family: Pentium Pro */
#define FAMILY_ATHLON        6 /**< proc_get_family() processor family: Athlon */
#define FAMILY_K7            6 /**< proc_get_family() processor family: AMD K7 */
/* Pentium (586) */
#define FAMILY_P5            5 /**< proc_get_family() processor family: P5 family */
#define FAMILY_PENTIUM       5 /**< proc_get_family() processor family: Pentium */
#define FAMILY_K6            5 /**< proc_get_family() processor family: K6 */
#define FAMILY_K5            5 /**< proc_get_family() processor family: K5 */
/* 486 */
#define FAMILY_486           4 /**< proc_get_family() processor family: 486 */

/* We do not enumerate all models; just relevant ones needed to distinguish
 * major processors in the same family.
 */
#define MODEL_I7_WESTMERE_EX  47 /**< proc_get_model(): Core i7 Westmere Ex */
#define MODEL_I7_WESTMERE     44 /**< proc_get_model(): Core i7 Westmere */
#define MODEL_I7_CLARKDALE    37 /**< proc_get_model(): Core i7 Clarkdale/Arrandale */
#define MODEL_I7_HAVENDALE    31 /**< proc_get_model(): Core i7 Havendale/Auburndale */
#define MODEL_I7_CLARKSFIELD  30 /**< proc_get_model(): Core i7 Clarksfield/Lynnfield */
#define MODEL_ATOM            28 /**< proc_get_model(): Atom */
#define MODEL_I7_GAINESTOWN   26 /**< proc_get_model(): Core i7 Gainestown */
#define MODEL_CORE_PENRYN     23 /**< proc_get_model(): Core 2 Penryn */
#define MODEL_CORE_2          15 /**< proc_get_model(): Core 2 Merom/Conroe */
#define MODEL_CORE            14 /**< proc_get_model(): Core Yonah */
#define MODEL_PENTIUM_M       13 /**< proc_get_model(): Pentium M 2MB L2 */
#define MODEL_PENTIUM_M_1MB    9 /**< proc_get_model(): Pentium M 1MB L2 */

/**
 * Struct to hold all 4 32-bit feature values returned by cpuid.
 * Used by proc_get_all_feature_bits().
 */
typedef struct {
    uint flags_edx;             /**< feature flags stored in edx */
    uint flags_ecx;             /**< feature flags stored in ecx */
    uint ext_flags_edx;         /**< extended feature flags stored in edx */
    uint ext_flags_ecx;         /**< extended feature flags stored in ecx */
} features_t;

/**
 * Feature bits returned by cpuid.  Pass one of these values to proc_has_feature() to
 * determine whether the underlying processor has the feature.
 */
typedef enum {
    /* features returned in edx */
    FEATURE_FPU =       0,              /**< Floating-point unit on chip */
    FEATURE_VME =       1,              /**< Virtual Mode Extension */
    FEATURE_DE =        2,              /**< Debugging Extension */
    FEATURE_PSE =       3,              /**< Page Size Extension */
    FEATURE_TSC =       4,              /**< Time-Stamp Counter */
    FEATURE_MSR =       5,              /**< Model Specific Registers */
    FEATURE_PAE =       6,              /**< Physical Address Extension */
    FEATURE_MCE =       7,              /**< Machine Check Exception */
    FEATURE_CX8 =       8,              /**< CMPXCHG8 Instruction Supported */
    FEATURE_APIC =      9,              /**< On-chip APIC Hardware Supported */
    FEATURE_SEP =       11,             /**< Fast System Call */
    FEATURE_MTRR =      12,             /**< Memory Type Range Registers */
    FEATURE_PGE =       13,             /**< Page Global Enable */
    FEATURE_MCA =       14,             /**< Machine Check Architecture */
    FEATURE_CMOV =      15,             /**< Conditional Move Instruction */
    FEATURE_PAT =       16,             /**< Page Attribute Table */
    FEATURE_PSE_36 =    17,             /**< 36-bit Page Size Extension */
    FEATURE_PSN =       18,             /**< Processor serial # present & enabled */
    FEATURE_CLFSH =     19,             /**< CLFLUSH Instruction supported */
    FEATURE_DS =        21,             /**< Debug Store */
    FEATURE_ACPI =      22,             /**< Thermal monitor & SCC supported */
    FEATURE_MMX =       23,             /**< MMX technology supported */
    FEATURE_FXSR =      24,             /**< Fast FP save and restore */
    FEATURE_SSE =       25,             /**< SSE Extensions supported */
    FEATURE_SSE2 =      26,             /**< SSE2 Extensions supported */
    FEATURE_SS =        27,             /**< Self-snoop */
    FEATURE_HTT =       28,             /**< Hyper-threading Technology */
    FEATURE_TM =        29,             /**< Thermal Monitor supported */
    FEATURE_IA64 =      30,             /**< IA64 Capabilities */
    FEATURE_PBE =       31,             /**< Pending Break Enable */
    /* features returned in ecx */
    FEATURE_SSE3 =      0 + 32,         /**< SSE3 Extensions supported */
    FEATURE_MONITOR =   3 + 32,         /**< MONITOR/MWAIT instructions supported */
    FEATURE_DS_CPL =    4 + 32,         /**< CPL Qualified Debug Store */
    FEATURE_VMX =       5 + 32,         /**< Virtual Machine Extensions */
    FEATURE_EST =       7 + 32,         /**< Enhanced Speedstep Technology */
    FEATURE_TM2 =       8 + 32,         /**< Thermal Monitor 2 */
    FEATURE_SSSE3 =     9 + 32,         /**< SSSE3 Extensions supported */
    FEATURE_CID =       10 + 32,        /**< Context ID */
    FEATURE_CX16 =      13 + 32,        /**< CMPXCHG16B instruction supported */
    FEATURE_xPTR =      14 + 32,        /**< Send Task Priority Messages */
    FEATURE_SSE41 =     19 + 32,        /**< SSE4.1 Extensions supported */
    FEATURE_SSE42 =     20 + 32,        /**< SSE4.2 Extensions supported */
    /* extended features returned in edx */
    FEATURE_SYSCALL =   11 + 64,        /**< SYSCALL/SYSRET instructions supported */
    FEATURE_XD_Bit =    20 + 64,        /**< Execution Disable bit */
    FEATURE_EM64T =     29 + 64,        /**< Extended Memory 64 Technology */
    /* extended features returned in ecx */
    FEATURE_LAHF =      0 + 96          /**< LAHF/SAHF available in 64-bit mode */
} feature_bit_t;
#else
typedef enum {
	NO_FEATURE
} feature_bit_t;
#endif

/*
 * Due to the fact that the way to find out processor information in the ARM processor
 * has to be done in privilaged mode I have just hard coded a Cortex A8 Processor in
 * for now.
 */
static void
get_processor_specific_info(void);


/** Tests if processor has selected feature. */
bool
proc_has_feature(feature_bit_t feature);
#endif /* _PROC_H_ */


#endif /* _DR_DEFINES_H_ */
