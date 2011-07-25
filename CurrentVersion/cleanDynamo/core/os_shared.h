/* **********************************************************
 * Copyright (c) 2003-2009 VMware, Inc.  All rights reserved.
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

/*
 * os_shared.h - shared declarations for os facilities from linux/win32 
 */

#ifndef OS_SHARED_H
#define OS_SHARED_H

enum {VM_ALLOCATION_BOUNDARY = 64*1024}; /* 64KB allocation size for Windows */

struct _local_state_t; /* in arch_exports.h */

/* in os.c */
void os_init(void);
/* called on detach and on process exit in debug builds */
void os_slow_exit(void);
/* called on detach and on process exit in both debug and release builds */
void os_fast_exit(void);

void os_tls_init(void);
/* Frees local_state.  If the calling thread is exiting (i.e.,
 * !other_thread) then also frees kernel resources for the calling
 * thread; if other_thread then that may not be possible.
 */
void os_tls_exit(struct _local_state_t *local_state, bool other_thread);
void os_thread_init(dcontext_t *dcontext);
void os_thread_exit(dcontext_t *dcontext);
void os_thread_under_dynamo(dcontext_t *dcontext);
void os_thread_not_under_dynamo(dcontext_t *dcontext);

void os_heap_init(void);
void os_heap_exit(void);

/* os provided heap routines */
/* caller is required to handle thread synchronization and to update dynamo vm areas.
 * size must be PAGE_SIZE-aligned.
 * returns NULL if fails to allocate memory!
 * N.B.: only heap.c should call these routines, as it contains the logic to
 * handle memory allocation failure!
 *
 * error_code is set to 0 on success
 * on failure it is set to one of the other enum values below or an OS specific status
 * code used only for reporting
 */
enum {
    HEAP_ERROR_SUCCESS = 0,
    /* os_heap_reserve_in_region() only, couldn't find a place to reserve within region */
    HEAP_ERROR_CANT_RESERVE_IN_REGION = 1,
    /* os_heap_reserve() only, Linux only, mmap failed to reserve at preferred address */ 
    HEAP_ERROR_NOT_AT_PREFERRED = 2,
};
typedef uint heap_error_code_t;

/* Reserve size bytes of virtual address space in one piece without committing swap
 * space for it.  If preferred is non-NULL then memory will be reserved at that address
 * only (if size bytes are unavailable at preferred then the allocation will fail). 
 * The executable flag is for platforms that divide executable from data memory.
 */
void *os_heap_reserve(void *preferred, size_t size, heap_error_code_t *error_code,
                      bool executable);
/* Reserve size bytes of virtual address space in one piece entirely within the
 * address range specified without committing swap space for it. 
 * The executable flag is for platforms that divide executable from data memory.
 */
void *os_heap_reserve_in_region(void *start, void *end, size_t size,
                                heap_error_code_t *error_code, bool executable);
/* commit previously reserved pages, returns false when out of memory */
bool os_heap_commit(void *p, size_t size, uint prot, heap_error_code_t *error_code);
/* decommit previously committed page, so it is reserved for future reuse */
void os_heap_decommit(void *p, size_t size, heap_error_code_t *error_code);
/* frees size bytes starting at address p (note - on windows the entire allocation
 * containing p is freed and size is ignored) */
void os_heap_free(void *p, size_t size, heap_error_code_t *error_code);

/* prognosticate whether systemwide memory pressure based on
 * last_error_code and systemwide omens 
 *
 * Note it doesn't answer if the hog is current process, nor whether
 * it is our own fault in current process.
 */
bool os_heap_systemwide_overcommit(heap_error_code_t last_error_code);

bool os_heap_get_commit_limit(size_t *commit_used, size_t *commit_limit);

thread_id_t get_thread_id(void);
process_id_t get_process_id(void);
void thread_yield(void);
void thread_sleep(uint64 milliseconds);
bool thread_suspend(thread_record_t *tr);
bool thread_resume(thread_record_t *tr);
bool thread_terminate(thread_record_t *tr);

bool is_thread_currently_native(thread_record_t *tr);

/* If state beyond that in our dr_mcontext_t is needed, os-specific routines
 * must be used.  These only deal with dr_mcontext_t state.
 */
bool thread_get_mcontext(thread_record_t *tr, dr_mcontext_t *mc);
bool thread_set_mcontext(thread_record_t *tr, dr_mcontext_t *mc);

/* Takes an os-specific context. Does not return. */
void thread_set_self_context(void *cxt);
/* Only sets the dr_mcontext_t state.  Does not return. */
void thread_set_self_mcontext(dr_mcontext_t *mc);

dcontext_t *get_thread_private_dcontext(void);
void set_thread_private_dcontext(dcontext_t *dcontext);

/* converts a local_state_t offset to a segment offset */
ushort os_tls_offset(ushort tls_offs);

struct _local_state_t; /* in arch_exports.h */
struct _local_state_extended_t; /* in arch_exports.h */
struct _local_state_t *get_local_state(void);
struct _local_state_extended_t *get_local_state_extended(void);

#ifdef X86
/* Returns POINTER_MAX on failure.
 * Assumes that cs, ss, ds, and es are flat.
 */
byte *
get_segment_base(uint seg);
#endif

#ifdef CLIENT_INTERFACE
/* Allocates num_slots tls slots aligned with alignment align */
bool
os_tls_calloc(OUT uint *offset, uint num_slots, uint alignment);

bool
os_tls_cfree(uint offset, uint num_slots);
#endif

bool pre_system_call(dcontext_t *dcontext);
void post_system_call(dcontext_t *dcontext);

char *get_application_pid(void);
char *get_application_name(void);
const char *get_application_short_name(void);
char *get_computer_name(void); /* implemented on win32 only, in eventlog.c */

int get_num_processors(void);

/* Terminate types - the best choice here is TERMINATE_PROCESS, no cleanup*/
typedef enum {
    TERMINATE_PROCESS = 0x1,
    TERMINATE_THREAD  = 0x2, /* terminate thread (and process if last) */
    /* Use of TERMINATE_THREAD mode is dangerous, 
       and can result in the following problems (see MSDN):
       * If the target thread owns a critical section, 
       the critical section will not be released.
       * If the target thread is allocating memory from the heap, 
       the heap lock will not be released.
       * If the target thread is executing certain kernel32 calls when it is terminated, 
       the kernel32 state for the thread's process could be inconsistent.
       * If the target thread is manipulating the global state of a shared DLL, 
       the state of the DLL could be destroyed, affecting other users of the DLL.
    */
    TERMINATE_CLEANUP = 0x4 /* cleanup our state before issuing terminal syscall */
} terminate_flags_t;

void os_terminate(dcontext_t *dcontext, terminate_flags_t flags);

typedef enum {
    ILLEGAL_INSTRUCTION_EXCEPTION,
    UNREADABLE_MEMORY_EXECUTION_EXCEPTION,
    IN_PAGE_ERROR_EXCEPTION,
} exception_type_t;

void os_forge_exception(app_pc exception_address, exception_type_t type);

/* events for dumpcore_mask */
/* NOTE with DUMPCORE_DEADLOCK and DUMPCORE_ASSERT you will get 2 dumps for
 * rank order violations in debug builds */
enum {
    DUMPCORE_INTERNAL_EXCEPTION = 0x0001,
    DUMPCORE_SECURITY_VIOLATION = 0x0002,
    DUMPCORE_DEADLOCK           = 0x0004,
    DUMPCORE_ASSERTION          = 0x0008,
    DUMPCORE_FATAL_USAGE_ERROR  = 0x0010,
    DUMPCORE_UNSUPPORTED_APP    = 0x0020,
    DUMPCORE_TIMEOUT            = 0x0040,
    DUMPCORE_CURIOSITY          = 0x0080,
#ifdef HOT_PATCHING_INTERFACE       /* Part of fix for 5357 & 5988. */
    /* Errors and exceptions in hot patches are treated the same.  Case 5696. */
    DUMPCORE_HOTP_FAILURE       = 0x0100,
#endif
    DUMPCORE_OUT_OF_MEM         = 0x0200,
    DUMPCORE_OUT_OF_MEM_SILENT  = 0x0400, /* not on by default in DEBUG */
#ifdef LINUX
    DUMPCORE_INCLUDE_STACKDUMP  = 0x0800,
    DUMPCORE_WAIT_FOR_DEBUGGER  = 0x1000, /* not on by default in DEBUG */
#endif
#ifdef HOT_PATCHING_INTERFACE       /* Part of fix for 5988. */
    DUMPCORE_HOTP_DETECTION     = 0x2000, /* not on by default in DEBUG */
    DUMPCORE_HOTP_PROTECTION    = 0x4000, /* not on by default in DEBUG */
#endif
#ifdef CLIENT_INTERFACE
    DUMPCORE_DR_ABORT           = 0x8000,
#endif
    /* all exception cases here are off by default since we expect them to
     * usually be the app's fault (or normal behavior for the app) */
    DUMPCORE_FORGE_ILLEGAL_INST = 0x10000,
    DUMPCORE_FORGE_UNREAD_EXEC  = 0x20000, /* not including -throw_exception */
    /* Note this is ALL app exceptions (including those the app may expect and
     * handle without issue) except those created via RaiseException (note
     * dr forged exceptions use the eqv. of RaiseException).  Would be nice to
     * have a flag for just unhandled app exceptions but that's harder to
     * implement. */
    DUMPCORE_APP_EXCEPTION      = 0x40000,
    DUMPCORE_TRY_EXCEPT         = 0x80000, /* even when we do have a handler */

#ifdef LINUX
    DUMPCORE_OPTION_PAUSE       = DUMPCORE_WAIT_FOR_DEBUGGER  |
                                  DUMPCORE_INTERNAL_EXCEPTION |
                                  DUMPCORE_SECURITY_VIOLATION |
                                  DUMPCORE_DEADLOCK           |
                                  DUMPCORE_ASSERTION          |
                                  DUMPCORE_FATAL_USAGE_ERROR  |
                                  DUMPCORE_UNSUPPORTED_APP    |
                                  DUMPCORE_TIMEOUT            |
                                  DUMPCORE_CURIOSITY          |
# ifdef CLIENT_INTERFACE
                                  DUMPCORE_DR_ABORT           |
# endif
                                  DUMPCORE_OUT_OF_MEM         |
                                  DUMPCORE_OUT_OF_MEM_SILENT,
#endif
};
void os_dump_core(const char *msg);

int os_timeout(int time_in_milliseconds);

void os_syslog(syslog_event_type_t priority, uint message_id, 
               uint substitutions_num, va_list args);

#if defined(CLIENT_INTERFACE) || defined(HOT_PATCHING_INTERFACE)
/* Note that this is NOT identical to module_handle_t: on Linux this
 * is a pointer to a loader data structure and NOT the base address
 * (xref PR 366195).
 */
typedef void * shlib_handle_t;
typedef void (*shlib_routine_ptr_t)();

shlib_handle_t load_shared_library(char *name);
#endif

#if defined(CLIENT_INTERFACE)
shlib_routine_ptr_t lookup_library_routine(shlib_handle_t lib, char *name);
void unload_shared_library(shlib_handle_t lib);
void shared_library_error(char *buf, int maxlen);
/* addr is any pointer known to lie within the library */
bool shared_library_bounds(IN shlib_handle_t lib, IN byte *addr,
                           OUT byte **start, OUT byte **end);
#endif

/* DR_API EXPORT TOFILE dr_tools.h */
/* DR_API EXPORT BEGIN */
/**************************************************
 * MEMORY INFORMATION TYPES
 */

#define DR_MEMPROT_NONE  0x00 /**< No read, write, or execute privileges. */
#define DR_MEMPROT_READ  0x01 /**< Read privileges. */
#define DR_MEMPROT_WRITE 0x02 /**< Write privileges. */
#define DR_MEMPROT_EXEC  0x04 /**< Execute privileges. */

/**
 * Flags describing memory used by dr_query_memory_ex().
 */
typedef enum {
    DR_MEMTYPE_FREE,  /**< No memory is allocated here */
    DR_MEMTYPE_IMAGE, /**< An executable file is mapped here */
    DR_MEMTYPE_DATA,  /**< Some other data is allocated here */
} dr_mem_type_t;

/**
 * Describes a memory region.  Used by dr_query_memory_ex().
 */
typedef struct _dr_mem_info_t {
    /** Starting address of memory region */
    byte          *base_pc;
    /** Size of region */
    size_t        size;
    /** Protection of region (DR_MEMPROT_* flags) */
    uint          prot;
    /** Type of region */
    dr_mem_type_t type;
} dr_mem_info_t;

/* DR_API EXPORT END */

#define MEMPROT_NONE  DR_MEMPROT_NONE
#define MEMPROT_READ  DR_MEMPROT_READ
#define MEMPROT_WRITE DR_MEMPROT_WRITE
#define MEMPROT_EXEC  DR_MEMPROT_EXEC
bool get_memory_info(const byte *pc, byte **base_pc, size_t *size, uint *prot);
bool query_memory_ex(const byte *pc, OUT dr_mem_info_t *info);
#ifdef LINUX
bool get_memory_info_from_os(const byte *pc, byte **base_pc, size_t *size, uint *prot);
bool query_memory_ex_from_os(const byte *pc, OUT dr_mem_info_t *info);
#endif

bool get_stack_bounds(dcontext_t *dcontext, byte **base, byte **top);

bool is_readable_without_exception(const byte *pc, size_t size);
bool is_readable_without_exception_query_os(byte *pc, size_t size);
bool safe_read(const void *base, size_t size, void *out_buf);
bool safe_read_ex(const void *base, size_t size, void *out_buf, size_t *bytes_read);
bool safe_write_ex(void *base, size_t size, const void *in_buf, size_t *bytes_written);
bool is_user_address(byte *pc);
/* returns osprot flags preserving all native protection flags except
 * for RWX, which are replaced according to memprot */
uint
osprot_replace_memprot(uint old_osprot, uint memprot);

/* returns false if out of memory */
bool set_protection(byte *pc, size_t size, uint prot);
/* Change protections on memory region starting at pc of length size 
 * (padded to page boundaries).  This method is meant to be used on DR memory 
 * as part of protect from app and is safe with respect to stats and changing
 * the protection of the data segment. */
/* returns false if out of memory */
bool change_protection(byte *pc, size_t size, bool writable);
#ifdef WINDOWS
/* makes pc:pc+size (page_padded) writable preserving other flags 
 * returns false if out of memory
 */
bool make_hookable(byte *pc, size_t size, bool *changed_prot);
/* if changed_prot makes pc:pc+size (page padded) unwritable preserving 
 * other flags */
void make_unhookable(byte *pc, size_t size, bool changed_prot);
#endif
/* requires that pc is page aligned and size is multiple of the page size
 * and marks that memory writable, preserves other flags,
 * returns false if out of memory
 */
bool make_writable(byte *pc, size_t size);
/* requires that pc is page aligned and size is multiple of the page size
 * and marks that memory NOT writable, preserves other flags */
void make_unwritable(byte *pc, size_t size);
/* like make_writable but adds COW (note: only usable if allocated COW) */
bool make_copy_on_writable(byte *pc, size_t size);

/***************************************************************************
 * SELF_PROTECTION
 */

/* Values for protect_mask to specify what is write-protected from malicious or
 * inadvertent modification by the application.
 * DATA_CXTSW and GLOBAL are done on each context switch
 * the rest are on-demand:
 * DATASEGMENT, DATA_FREQ, and GENCODE only on the rare occasions when we write to them
 * CACHE only when emitting or {,un}linking
 * LOCAL only on path in DR that needs to write to local
 */
enum {
    /* Options specifying protection of our DR dll data sections: */
    /* .data == variables written only at init or exit time or rarely in between */
    SELFPROT_DATA_RARE   = 0x001,
    /* .fspdata == frequently written enough that we separate from .data.
     * FIXME case 8073: currently these are unprotected on every cxt switch
     */
    SELFPROT_DATA_FREQ   = 0x002,
    /* .cspdata == so frequently written that to protect them requires unprotecting
     * every context switch.
     */
    SELFPROT_DATA_CXTSW  = 0x004,

    /* if GLOBAL && !DCONTEXT, entire dcontext is unprotected, rest of
     *   global allocs are protected;
     * if GLOBAL && DCONTEXT, cache-written fields of dcontext are unprotected,
     *   rest are protected;
     * if !GLOBAL, DCONTEXT should not be used
     */
    SELFPROT_GLOBAL      = 0x008,
    SELFPROT_DCONTEXT    = 0x010, /* means we split out unprotected_context_t -- 
                                   * no actual protection unless SELFPROT_GLOBAL */
    SELFPROT_LOCAL       = 0x020,
    SELFPROT_CACHE       = 0x040, /* FIXME: thread-safe NYI when doing all units */
    SELFPROT_STACK       = 0x080, /* essentially always on with clean-dstack dispatch()
                                   * design, leaving as a bit in case we do more later */
    /* protect our generated thread-shared and thread-private code */
    SELFPROT_GENCODE     = 0x100,
    /* FIXME: TEB page on Win32 
     * Other global structs, like thread-local callbacks on Win32?
     * PEB page?
     */
    /* options that require action on every context switch
     * FIXME: global heap used to be much rarer before shared
     * fragments, only containing "important" data, which is why we
     * un-protected on every context switch.  We should re-think that
     * now that most things are shared.
     */
    SELFPROT_ON_CXT_SWITCH = (SELFPROT_DATA_CXTSW | SELFPROT_GLOBAL
                              /* FIXME case 8073: this is only temporary until
                               * we finish implementing .fspdata unprots */
                              | SELFPROT_DATA_FREQ),
    SELFPROT_ANY_DATA_SECTION = (SELFPROT_DATA_RARE | SELFPROT_DATA_FREQ |
                                 SELFPROT_DATA_CXTSW),
};

/* Values to refer to individual data sections.  The order is not
 * important here.
 */
enum {
    DATASEC_NEVER_PROT = 0,
    DATASEC_RARELY_PROT,
    DATASEC_FREQ_PROT,
    DATASEC_CXTSW_PROT,
    DATASEC_NUM,
};

/* in dynamo.c */
extern const uint DATASEC_SELFPROT[];
extern const char * const DATASEC_NAMES[];
/* see dynamo.c for why this is not an array */
extern const uint datasec_writable_neverprot;
extern uint datasec_writable_rareprot;
extern uint datasec_writable_freqprot;
extern uint datasec_writable_cxtswprot;

/* this is a uint not a bool */
#define DATASEC_WRITABLE(which) \
    ((which) == DATASEC_RARELY_PROT ? datasec_writable_rareprot : \
     ((which) == DATASEC_CXTSW_PROT ? datasec_writable_cxtswprot : \
      ((which) == DATASEC_FREQ_PROT ? datasec_writable_freqprot : \
       datasec_writable_neverprot)))

/* these must be plain literals since we need these in pragmas/attributes */
#define NEVER_PROTECTED_SECTION  ".nspdata"
#define RARELY_PROTECTED_SECTION ".data"
#define FREQ_PROTECTED_SECTION   ".fspdata"
#define CXTSW_PROTECTED_SECTION  ".cspdata"

/* note that asserting !protected is safe, but asserting protection
 * is racy as another thread could be in an unprot window.
 * see also check_should_be_protected().
 */
#define DATASEC_PROTECTED(which) (DATASEC_WRITABLE(which) == 0)

/* User MUST supply an init value, or else cl will leave in .bss (will show up
 * at end of .data), which is why we hardcode the = here!
 * We use varargs to allow commas in the init if a struct.
 */
#define DECLARE_FREQPROT_VAR(var, ...)                        \
    START_DATA_SECTION(FREQ_PROTECTED_SECTION, "w")           \
    var VAR_IN_SECTION(FREQ_PROTECTED_SECTION) = __VA_ARGS__; \
    END_DATA_SECTION() 

#define DECLARE_CXTSWPROT_VAR(var, ...)                        \
    START_DATA_SECTION(CXTSW_PROTECTED_SECTION, "w")           \
    var VAR_IN_SECTION(CXTSW_PROTECTED_SECTION) = __VA_ARGS__; \
    END_DATA_SECTION() 

#define DECLARE_NEVERPROT_VAR(var, ...)                        \
    START_DATA_SECTION(NEVER_PROTECTED_SECTION, "w")           \
    var VAR_IN_SECTION(NEVER_PROTECTED_SECTION) = __VA_ARGS__; \
    END_DATA_SECTION() 

#define SELF_PROTECT_ON_CXT_SWITCH \
    (TESTANY(SELFPROT_ON_CXT_SWITCH, DYNAMO_OPTION(protect_mask)) \
     || INTERNAL_OPTION(single_privileged_thread))

/* macros to put mask check outside of function, for efficiency */
#define SELF_PROTECT_LOCAL(dc, w) do { \
    if (TEST(SELFPROT_LOCAL, dynamo_options.protect_mask)) \
        protect_local_heap(dc, w);                         \
} while (0);

#define SELF_PROTECT_GLOBAL(w) do { \
    if (TEST(SELFPROT_GLOBAL, dynamo_options.protect_mask)) \
        protect_global_heap(w);                             \
} while (0);

#define ASSERT_LOCAL_HEAP_PROTECTED(dcontext) \
    ASSERT(!TEST(SELFPROT_LOCAL, dynamo_options.protect_mask) || \
           local_heap_protected(dcontext))
#define ASSERT_LOCAL_HEAP_UNPROTECTED(dcontext) \
    ASSERT(!TEST(SELFPROT_LOCAL, dynamo_options.protect_mask) || \
           !local_heap_protected(dcontext))

#define SELF_PROTECT_DATASEC(which) do { \
    if (TEST(DATASEC_SELFPROT[which], DYNAMO_OPTION(protect_mask))) \
        protect_data_section(which, READONLY);    \
} while (0);
#define SELF_UNPROTECT_DATASEC(which) do { \
    if (TEST(DATASEC_SELFPROT[which], DYNAMO_OPTION(protect_mask))) \
        protect_data_section(which, WRITABLE);      \
} while (0);

/***************************************************************************/

#ifdef DEBUG
void mem_stats_snapshot(void);
#endif

app_pc get_dynamorio_dll_start(void);
app_pc get_dynamorio_dll_preferred_base(void);

bool is_in_dynamo_dll(app_pc pc);
int find_dynamo_library_vm_areas(void);
int find_executable_vm_areas(void);

/* all_memory_areas is linux only, dummy on win32 */
void all_memory_areas_lock(void);
void all_memory_areas_unlock(void);
/* pass -1 if type is unchanged */
void update_all_memory_areas(app_pc start, app_pc end, uint prot, int type);
bool remove_from_all_memory_areas(app_pc start, app_pc end);

/* file operations */
/* defaults to read only access, if write is not set ignores others */
#define OS_OPEN_READ        0x01
#define OS_OPEN_WRITE       0x02
#define OS_OPEN_APPEND      0x04
#define OS_OPEN_REQUIRE_NEW 0x08
#define OS_EXECUTE          0x10 /* only used on win32, currently */
#define OS_SHARE_DELETE     0x20 /* only used on win32, currently */
#define OS_OPEN_FORCE_OWNER 0x40 /* only used on win32, currently */
#define OS_OPEN_ALLOW_LARGE 0x80 /* only used on linux32, currently */
/* always use OS_OPEN_REQUIRE_NEW when asking for OS_OPEN_WRITE, in
 * order to avoid hard link or symbolic link attacks if the file is in
 * a world writable locations and the process may have high
 * privileges.
 */
file_t os_open(const char *fname, int os_open_flags);
file_t os_open_directory(const char *fname, int os_open_flags);
/* FIXME : os_file_exists always returns true on linux */
bool os_file_exists(const char *fname, bool is_dir);
bool os_get_file_size(const char *file, uint64 *size); /* NYI on Linux */
bool os_get_file_size_by_handle(file_t fd, uint64 *size);

typedef enum {
    CREATE_DIR_ALLOW_EXISTING = 0x0,
    /* should always use CREATE_DIR_REQUIRE_NEW, see reference in
     * OS_OPEN_REQUIRE_NEW - to avoid symlink attacks (although only
     * an issue when files we create in these directories have
     * predictible names - case 9138)
     */
    CREATE_DIR_REQUIRE_NEW    = 0x1,
    CREATE_DIR_FORCE_OWNER    = 0x2,
} create_directory_flags_t;

bool os_create_dir(const char *fname, create_directory_flags_t create_dir_flags);
void os_close(file_t f);
/* returns number of bytes written, negative if failure */
ssize_t os_write(file_t f, const void *buf, size_t count);
/* returns number of bytes read, negative if failure */
ssize_t os_read(file_t f, void *buf, size_t count);
void os_flush(file_t f);

/* For use with os_file_seek(), specifies the origin at which to apply the offset
 * NOTE - keep in synch with DR_SEEK_* in insturment.h and SEEK_* from Linux headers */
#define OS_SEEK_SET 0  /* start of file */
#define OS_SEEK_CUR 1  /* current file position */
#define OS_SEEK_END 2  /* end of file */
/* seek the current file position to offset bytes from origin, return true if successful */
bool os_seek(file_t f, int64 offset, int origin);
/* return the current file position, -1 on failure */
int64 os_tell(file_t f);

bool os_delete_file(const char *file_name);
bool os_delete_mapped_file(const char *filename);
bool os_rename_file(const char *orig_name, const char *new_name, bool replace);
/* These routines do not update dynamo_areas; use the non-os_-prefixed versions */
/* File mapping on Linux uses a 32-bit offset, with mmap2 considering
 * it a multiple of the page size in order to have a larger reach; on
 * Windows we have a 64-bit offset.  We go with the widest here, a
 * 64-bit offset, to avoid limiting Windows.
 */
/* When fixed==true, DR tries to allocate memory from the address specified 
 * by addr. 
 * In Linux, it has the same semantic as mmap system call with MAP_FIXED flags, 
 * If the memory region specified by addr and size overlaps pages of 
 * any existing mapping(s), then the overlapped part of the existing mapping(s)
 * will be discarded. If the specified address cannot be used, it will fail
 * and returns NULL.
 * XXX: in Windows, fixed argument is currently ignored (PR 214077 / case 9642),
 * and handling it is covered by PR 214097.
 */
byte *os_map_file(file_t f, size_t *size INOUT, uint64 offs, app_pc addr,
                  uint prot, bool copy_on_write, bool image, bool fixed);
bool os_unmap_file(byte *map, size_t size);
/* unlike set_protection, os_set_protection does not update 
 * the allmem info in Linux. */
bool os_set_protection(byte *pc, size_t length, uint prot/*MEMPROT_*/);

bool
os_current_user_directory(char *directory_prefix /* INOUT */,
                          uint directory_size,
                          bool create);
bool
os_validate_user_owned(file_t file_or_directory_handle);


bool
os_get_disk_free_space(/*IN*/ file_t file_handle,
                       /*OUT*/ uint64 *AvailableQuotaBytes /*OPTIONAL*/,
                       /*OUT*/ uint64 *TotalQuotaBytes /*OPTIONAL*/,
                       /*OUT*/ uint64 *TotalVolumeBytes /*OPTIONAL*/);

#ifdef PROFILE_RDTSC
extern uint kilo_hertz;
#endif

/* Despite the name, this enum is used for all types of critical events:
es
 * asserts and crashes for all builds, and security violations for PROGRAM_SHEPHERDING builds
 * When a security_violation is being reported, enum value must be negative! 
 */
typedef enum {
#ifdef PROGRAM_SHEPHERDING
    STACK_EXECUTION_VIOLATION       = -1,
    HEAP_EXECUTION_VIOLATION        = -2,
    RETURN_TARGET_VIOLATION         = -3, 
    RETURN_DIRECT_RCT_VIOLATION     = -4, /* NYI DIRECT_CALL_CHECK */
    INDIRECT_CALL_RCT_VIOLATION     = -5,
    INDIRECT_JUMP_RCT_VIOLATION     = -6,
# ifdef HOT_PATCHING_INTERFACE
    HOT_PATCH_DETECTOR_VIOLATION    = -7,
    HOT_PATCH_PROTECTOR_VIOLATION   = -8,
    /* Errors and exceptions in hot patches are treated the same.  Case 5696. */
    HOT_PATCH_FAILURE               = -9,
# endif
    /* internal */
    ATTACK_SIMULATION_VIOLATION     = -10,
    ATTACK_SIM_NUDGE_VIOLATION      = -11,
#endif
    /* not really PROGRAM_SHEPHERDING */
    ASLR_TARGET_VIOLATION           = -12,
#ifdef GBOP
    GBOP_SOURCE_VIOLATION           = -13,
#endif /* GBOP */
#ifdef PROCESS_CONTROL
    PROCESS_CONTROL_VIOLATION       = -14,      /* case 8594 */
#endif
    APC_THREAD_SHELLCODE_VIOLATION  = -15, /* note still presented externally as .B */
    /* Not a valid violation; used for initializing security_violation_t vars. */
    INVALID_VIOLATION               = 0,
#ifdef PROGRAM_SHEPHERDING
    /* Add new violation types above this line as consecutive negative
     * numbers, and update get_security_violation_name() for the
     * appropriate letter obfuscation */
    ALLOWING_OK                     =  1,
    ALLOWING_BAD                    =  2,    
#endif
    NO_VIOLATION_BAD_INTERNAL_STATE =  3,
    NO_VIOLATION_OK_INTERNAL_STATE  =  4
} security_violation_t;

/*  in diagnost.c */
void report_diagnostics(const char *message, const char *name,
                        security_violation_t violation_type);
void append_diagnostics(file_t diagnostics_file, const char *message, const char *name,
                        security_violation_t violation_type);
void diagnost_exit(void);
bool check_for_unsupported_modules(void);

#ifdef RETURN_AFTER_CALL
typedef enum {
    INITIAL_STACK_EMPTY              = 0,
    INITIAL_STACK_BOTTOM_REACHED     = 1,
    INITIAL_STACK_BOTTOM_NOT_REACHED = 2
} initial_call_stack_status_t;

initial_call_stack_status_t at_initial_stack_bottom(dcontext_t *dcontext, app_pc target_pc);
bool at_known_exception(dcontext_t *dcontext, app_pc target_pc, app_pc source_fragment);
#endif

/* contended path of mutex operations */
void mutex_wait_contended_lock(mutex_t *lock);
void mutex_notify_released_lock(mutex_t *lock);
/* contended path of rwlock operations */
void rwlock_wait_contended_writer(read_write_lock_t *rwlock);
void rwlock_notify_writer(read_write_lock_t *rwlock);
void rwlock_wait_contended_reader(read_write_lock_t *rwlock);
void rwlock_notify_readers(read_write_lock_t *rwlock);

/* The event interface */

/* An event object. */
#ifdef WINDOWS
typedef HANDLE event_t;
#else
typedef struct linux_event_t *event_t;
#endif

event_t create_event(void);
void destroy_event(event_t e);
void signal_event(event_t e);
void reset_event(event_t e);
void wait_for_event(event_t e);

/* get current timer frequency in KHz */
/* NOTE: Keep in mind that with voltage scaling in power saving mode
 * this may not be a reliable way to obtain time information */
timestamp_t
get_timer_frequency(void);

/* On Linux, returns the number of seconds since the Epoch (Jan 1, 1970).
 * On Windows, returns the number of seconds since Jan 1, 1600 (this is
 * the current UTC time).
 */
uint
query_time_seconds(void);

/* On Linux, returns the number of milliseconds since the Epoch (Jan 1, 1970).
 * On Windows, returns the number of milliseconds since Jan 1, 1600 (this is
 * the current UTC time).
 */
uint64
query_time_millis(void);

/* gives a good but not necessarily crypto-strength random seed */
uint
os_random_seed(void);

/* module map/unmap processing relevant to the RCT policies */
/* cf. rct_analyze_module_at_violation() which can be called lazily */
void
rct_process_module_mmap(app_pc module_base, size_t module_size, 
                        bool add, bool already_relocated);

/* module boundary and code section analysis for RCT policies */
/* FIXME: should be better abstracted by calling a routine that
 * enumerates code sections, while keeping the general driver in rct.c
 */
bool
rct_analyze_module_at_violation(dcontext_t *dcontext, app_pc target_pc);
bool
aslr_is_possible_attack(app_pc target);
app_pc
aslr_possible_preferred_address(app_pc target_addr);

#ifdef WINDOWS
/* dispatch places next pc in asynch_target and clears it after syscall handling
 * completes, so a zero value means shared syscall was used and the next pc
 * is in the esi slot. If asynch_target == BACK_TO_NATIVE_AFTER_SYSCALL then the
 * thread is native at an intercepted syscall and the real post syscall target is in
 * native_exec_postsyscall. */
# define POST_SYSCALL_PC(dc)                                                      \
     ((dc)->asynch_target == NULL ?                                               \
      ASSERT(DYNAMO_OPTION(shared_syscalls)), (app_pc) get_mcontext((dc))->xsi :  \
      ((dc)->asynch_target == BACK_TO_NATIVE_AFTER_SYSCALL ?                      \
       (dc)->native_exec_postsyscall : (dc)->asynch_target))
#else
/* on linux asynch target always holds the post syscall pc */
# define POST_SYSCALL_PC(dc) ((dc)->asynch_target)
#endif

/* This has been exposed from win32 module so that hotp_only_gateway can 
 * return the right return code.
 */
typedef enum {
    AFTER_INTERCEPT_LET_GO,
    AFTER_INTERCEPT_LET_GO_ALT_DYN, /* alternate direct execution target,
                                     * usable only with
                                     * AFTER_INTERCEPT_DYNAMIC_DECISION,
                                     * not by itself */
    AFTER_INTERCEPT_TAKE_OVER,
    AFTER_INTERCEPT_DYNAMIC_DECISION, /* handler returns one of prev values */

    /* handler is expected to restore original instructions */
    AFTER_INTERCEPT_TAKE_OVER_SINGLE_SHOT, /* static only with alternative target */
} after_intercept_action_t;

/* Argument structure for intercept function; contains app state at the point
 * our intercept routine will take over.  Fix for case 7597.
 *
 * -- CAUTION -- the number, order and size of fields in this structure are
 * assumed in emit_intercept_code().  Changing anything here is likely to cause
 * DR hooks & hotp_only to fail.
 */
typedef struct {
    void *callee_arg;     /* Argument passed to the intercept routine. */
    app_pc start_pc; /* optimization: could use mc.pc instead */
    int app_errno;
#ifdef X64
    /* 4 bytes of padding since dr_mcontext_t is 8-byte aligned, but it's handled
     * naturally in emit_intercept_code() since pushes are all 8 bytes */ 
#endif
    dr_mcontext_t mc;
} app_state_at_intercept_t;

/* note that only points intercepted with DYNAMIC_DECISION (currently only
 * [un]load_dll) care about the return value */ 
typedef after_intercept_action_t intercept_function_t(app_state_at_intercept_t *args);

/* opcodes and encoding constants that we sometimes directly use */
#ifdef X86
/* FIXME: if we add more opcodes maybe should be exported by x86/ */
enum {
    JMP_REL32_OPCODE  = 0xe9,
    JMP_REL32_SIZE    = 5,      /* size in bytes of 32-bit rel jmp */
    CALL_REL32_OPCODE = 0xe8,
# ifdef X64
    JMP_ABS_IND64_OPCODE    = 0xff,
    JMP_ABS_IND64_SIZE      = 6,    /* size in bytes of a 64-bit abs ind jmp */
    JMP_ABS_MEM_IND64_MODRM = 0x25,
# endif
};
#endif /* X86 */

#ifdef WINDOWS
/* used for option early_inject_location */
/* use 0 to supply an arbitrary address via -early_inject_address */
/* NOTE - these enum values are passed between processes, don't change/reuse
 * any existing values. */ 
/* NtMapViewOfSection doesn't work as a place to LoadDll (ldr can't handle the
 * re-entrancy), but might be a good place to takeover if we remote mapped in
 * and needed ntdll initialized (first MapView is for kernel32.dll fom what
 * I've seen, though beware the LdrLists aren't consistent at this point). */
/* As currently implemented KiUserException location doesn't work, but we might
 * be able to find a better exception location (maybe point the image's import
 * section to invalid memory instead? TODO try). */
/* see early_inject_init() in os.c for more notes on best location for each
 * os version */
/* NOTE - be carefull changing this enum as it's shared across proccesses. 
 * Also values <= LdrDefault are assumed to need address computation while
 * those > are assumed to not need it. */
#define INJECT_LOCATION_IS_LDR(loc) (loc <= INJECT_LOCATION_LdrDefault)
#define INJECT_LOCATION_IS_LDR_NON_DEFAULT(loc) (loc < INJECT_LOCATION_LdrDefault)
enum {
    INJECT_LOCATION_Invalid          = -100, /* invalid sentinel */
    INJECT_LOCATION_LdrpLoadDll      = 0, /* Good for XP, 2k3 */
    INJECT_LOCATION_LdrpLoadImportModule = 1, /* Good for 2k */
    INJECT_LOCATION_LdrCustom        = 2, /* Specify custom address */
    INJECT_LOCATION_LdrLoadDll       = 3, /* Good for 2k3 */
    INJECT_LOCATION_LdrDefault       = 4, /* Pick best location based on OS */
    /* For NT the best location is LdrpLoadImportModule, but we can't find that
     * automatically on NT, so LdrDefault for NT uses -early_inject_address if
     * specified or else disables early injection (xref 7806). */
    /* Beyond this point not expected to need address determination */
    INJECT_LOCATION_KiUserApc        = 5, /* NYI as must use map which is NYI */
    INJECT_LOCATION_KiUserException  = 6, /* No good, Ldr init issues */
    INJECT_LOCATION_MAX = INJECT_LOCATION_KiUserException,
};
#endif

void take_over_primary_thread(void);

#ifdef HOT_PATCHING_INTERFACE
void *
get_drmarker_hotp_policy_status_table(void);
void
set_drmarker_hotp_policy_status_table(void *new_table);

/* Used for -hotp_only; can be exposed if needed elsewhere later on. */
byte *
hook_text(byte *hook_code_buf, const app_pc image_addr, 
          intercept_function_t hook_func, const void *callee_arg, 
          const after_intercept_action_t action_after,
          const bool abort_if_hooked, const bool ignore_cti,
          byte **app_code_copy_p, byte **alt_exit_cti_p);
void
unhook_text(byte *hook_code_buf, app_pc image_addr);
void
insert_jmp_at_tramp_entry(byte *trampoline, byte *target);
#endif /* HOT_PATCHING_INTERFACE */

/* checks for compatibility OS specific options, returns true if
 * modified the value of any options to make them compatible
 */
bool
os_check_option_compatibility(void);

/* Introduced as part of PR 250294 - 64-bit hook reachability. */
#define LANDING_PAD_AREA_SIZE   64*1024
#ifdef X64
/* 8 bytes for the 64-bit abs addr, 6 for abs ind jmp to the trampoline and 5
 * for return jmp back to the instruction after the hook. */
# define LANDING_PAD_SIZE    19
#else
/* 5 bytes each for the two relative jumps (one to the trampoline and the 
 * other back to instruction after hook. */
# define LANDING_PAD_SIZE    10
#endif
byte *alloc_landing_pad(app_pc addr_to_hook);
void landing_pads_to_executable_areas(bool add);

/* in loader_shared.c */
#ifdef WINDOWS
app_pc load_private_library(const char *filename);
bool unload_private_library(app_pc modbase);
void loader_init(void);
void loader_exit(void);
void loader_thread_init(dcontext_t *dcontext);
void loader_thread_exit(dcontext_t *dcontext);
bool in_private_library(app_pc pc);
#endif /* WINDOWS */

#endif /* OS_SHARED_H */