#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_ 1

#include "../globals.h"
#include "../module_shared.h"
#include "arch.h"
#include "instr.h"

/* DR_API EXPORT TOFILE dr_events.h */
/* DR_API EXPORT BEGIN */

/****************************************************************************
 * ROUTINES TO REGISTER EVENT CALLBACKS
 */
/**
 * @file dr_events.h
 * @brief Event callback registration routines.
 */
/* DR_API EXPORT END */

typedef enum {
    /** Emit as normal. */
    DR_EMIT_DEFAULT              =    0,
    /**
     * Store translation information at emit time rather than calling
     * the basic block or trace event later to recreate the
     * information.  Note that even if a standalone basic block has
     * stored translations, if when it is added to a trace it does not
     * request storage (and the trace callback also does not request
     * storage) then the basic block callback may still be called to
     * translate for the trace.
     *
     * \sa #dr_register_bb_event()
     */
    DR_EMIT_STORE_TRANSLATIONS   = 0x01,
} dr_emit_flags_t;

typedef struct _module_segment_data_t {
    app_pc start; /**< Start address of the segment, page-aligned backward. */
    app_pc end;   /**< End address of the segment, page-aligned forward. */
    uint prot;    /**< Protection attributes of the segment */
} module_segment_data_t;

/**
 * Holds information about a loaded module. \note On Linux the start address can be
 * cast to an Elf32_Ehdr or Elf64_Ehdr. \note On Windows the start address can be cast to
 * an IMAGE_DOS_HEADER for use in finding the IMAGE_NT_HEADER and its OptionalHeader.
 * The OptionalHeader can be used to walk the module sections (among other things).
 * See WINNT.H. \note When accessing any memory inside the module (including header fields)
 * user is responsible for guarding against corruption and the possibility of the module
 * being unmapped.
 */
struct _module_data_t {
    union {
        app_pc start; /**< starting address of this module */
        module_handle_t handle; /**< module_handle for use with dr_get_proc_address() */
    } ; /* anonymous union of start address and module handle */
    /**
     * Ending address of this module.  Note that on Linux the module may not
     * be contiguous: there may be gaps containing other objects between start
     * and end.  Use the segments array to examine each mapped region on Linux.
     */
    app_pc end;

    app_pc entry_point; /**< entry point for this module as specified in the headers */

    uint flags; /**< Reserved, set to 0 */

    module_names_t names; /**< struct containing name(s) for this module; use
                           * dr_module_preferred_name() to get the preferred name for
                           * this module */

    char *full_path; /**< full path to the file backing this module */

#ifdef WINDOWS
    version_number_t file_version; /**< file version number from .rsrc section */
    version_number_t product_version; /**< product version number from .rsrc section */
    uint checksum; /**< module checksum from the PE headers */
    uint timestamp; /**< module timestamp from the PE headers */
    size_t module_internal_size; /**< module internal size (from PE headers SizeOfImage) */
#else
    bool contiguous;   /**< whether there are no gaps between segments */
    uint num_segments; /**< number of segments */
    /**
     * Array of num_segments entries, one per segment.  The array is sorted
     * by the start address of each segment.
     */
    module_segment_data_t *segments;
#endif
#ifdef AVOID_API_EXPORT
    /* FIXME: PR 215890: ELF64 size? Anything else? */
    /* We can add additional fields to the end without breaking compatibility */
#endif
};

/* DR_API EXPORT BEGIN */
/****************************************************************************
 * MODULE INFORMATION ROUTINES
 */
 
/** For dr_module_iterator_* interface */
typedef void * dr_module_iterator_t;
/**
 * DR will call the end trace event if it is registered prior to
 * adding each basic block to a trace being generated.  The return
 * value of the event callback should be from the
 * dr_custom_trace_action_t enum.
 *
 * \note DR treats CUSTOM_TRACE_CONTINUE as an advisement only.  Certain
 * fragments are not suitable to be included in a trace and if DR runs
 * into one it will end the trace regardless of what the client returns
 * through the event callback.
 */
typedef enum {
    CUSTOM_TRACE_DR_DECIDES,
    CUSTOM_TRACE_END_NOW,
    CUSTOM_TRACE_CONTINUE
} dr_custom_trace_action_t;

/* DR_API EXPORT END */

void instrument_load_client_libs(void);

DR_API
/**
 * Initialize a new module iterator.  The returned module iterator contains a snapshot
 * of the modules loaded at the time it was created.  Use dr_module_iterator_hasnext()
 * and dr_module_iterator_next() to walk the loaded modules.  Call
 * dr_module_iterator_stop() when finished to release the iterator. \note The iterator
 * does not prevent modules from being loaded or unloaded while the iterator is being
 * walked.
 */
dr_module_iterator_t
dr_module_iterator_start(void);


DR_API
/**
 * Returns true if there is another loaded module in the iterator.
 */
bool
dr_module_iterator_hasnext(dr_module_iterator_t *mi);

void instrument_init(void);

dr_emit_flags_t instrument_trace(dcontext_t *dcontext, app_pc tag, instrlist_t *trace,
                                 bool translating);

void instrument_fork_init(dcontext_t *dcontext);dr_custom_trace_action_t instrument_end_trace(dcontext_t *dcontext, app_pc trace_tag,
    app_pc next_tag);

void instrument_exit(void);

bool dr_thread_exit_hook_exists(void);
bool dr_fragment_deleted_hook_exists(void);

#define CURRENT_API_VERSION VERSION_NUMBER_INTEGER

void instrument_thread_exit_event(dcontext_t *dcontext);

int get_num_client_threads(void);

void instrument_thread_init(dcontext_t *dcontext, bool client_thread, bool valid_mc);

void instrument_thread_exit(dcontext_t *dcontext);

bool dr_exit_hook_exists(void);

void instrument_module_load(module_data_t *data, bool previously_loaded);

DR_API
/**
 * User should call this routine to free the module iterator.
 */
void
dr_module_iterator_stop(dr_module_iterator_t *mi);

void instrument_fragment_deleted(dcontext_t *dcontext, app_pc tag, uint flags);

bool instrument_pre_syscall(dcontext_t *dcontext, int sysnum);

/* This should only be called prior to instrument_init(),
 * since no readers of the client_libs array use synch
 * and since this routine assumes .data is writable.
 */
static void
add_client_lib(char *path, char *id_str, char *options);

bool instrument_invoke_another_syscall(dcontext_t *dcontext);
void instrument_post_syscall(dcontext_t *dcontext, int sysnum);

bool dr_end_trace_hook_exists(void);

dr_custom_trace_action_t instrument_end_trace(dcontext_t *dcontext, app_pc trace_tag,
                                           app_pc next_tag);

bool dr_bb_hook_exists(void);
bool dr_trace_hook_exists(void);
void instrument_module_unload(module_data_t *data);

DR_API
/**
 * Frees a module_data_t returned by dr_module_iterator_next(), dr_lookup_module(),
 * dr_lookup_module_by_name(), or dr_copy_module_data(). \note Should NOT be used with
 * a module_data_t obtained as part of a module load or unload event.
 */
void
dr_free_module_data(module_data_t *data);

module_data_t * copy_module_area_to_module_data(const module_area_t *area);

void instrument_nudge(dcontext_t *dcontext, client_id_t id, uint64 arg);

bool is_in_client_lib(app_pc addr);

void instrument_module_load_trigger(app_pc modbase);

typedef enum {
    /** Deliver signal to the application as normal. */
    DR_SIGNAL_DELIVER,
    /** Suppress signal as though it never happened. */
    DR_SIGNAL_SUPPRESS,
    /**
     * Deliver signal according to the default SIG_DFL action, as would
     * happen if the application had no handler.
     */
    DR_SIGNAL_BYPASS,
    /**
     * Do not deliver the signal.  Instead, redirect control to the
     * application state specified in dr_siginfo_t.mcontext.
     */
    DR_SIGNAL_REDIRECT,
} dr_signal_action_t;

/**
 * Data structure passed with a signal event.  Contains the machine
 * context at the signal interruption point and other signal
 * information.
 */
typedef struct _dr_fault_fragment_info_t {
    /**
     * The tag of the code fragment inside the code cache at the
     * exception/signal/translation interruption point. NULL for
     * interruption not in the code cache.
     */
    void *tag;
    /**
     * The start address of the code fragment inside the code cache at
     * the exception/signal/translation interruption point. NULL for interruption
     * not in the code cache.  Clients are cautioned when examining
     * code cache instructions to not rely on any details of code
     * inserted other than their own.
     */
    byte *cache_start_pc;
    /** Indicates whether the interrupted code fragment is a trace */
    bool is_trace;
    /**
     * Indicates whether the original application code containing the
     * code corresponding to the exception/signal/translation interruption point
     * is guaranteed to still be in the same state it was when the
     * code was placed in the code cache. This guarantee varies
     * depending on the type of cache consistency being used by DR.
     */
    bool app_code_consistent;
} dr_fault_fragment_info_t;

typedef struct _dr_siginfo_t {
    /** The signal number. */
    int sig;
    /** The context of the thread receiving the signal. */
    void *drcontext;
    /** The application machine state at the signal interruption point. */
    dr_mcontext_t mcontext;
    /**
     * The raw pre-translated machine state at the signal interruption
     * point inside the code cache.  NULL for delayable signals.  Clients
     * are cautioned when examining code cache instructions to not rely on
     * any details of code inserted other than their own.
     */
    dr_mcontext_t raw_mcontext;
    /** Whether raw_mcontext is valid. */
    bool raw_mcontext_valid;
    /**
     * For SIGBUS and SIGSEGV, the address whose access caused the signal
     * to be raised (as calculated by DR).
     */
    byte *access_address;
    /**
     * Indicates this signal is blocked.  DR_SIGNAL_BYPASS is not allowed,
     * and a second event will be sent if the signal is later delivered to
     * the application.  Events are only sent for blocked non-delayable signals,
     * not for delayable signals.
     */
    bool blocked;
    /**
     * Information about the code fragment inside the code cache
     * at the signal interruption point.
     */
    dr_fault_fragment_info_t fault_fragment_info;
} dr_siginfo_t;

bool dr_signal_hook_exists(void);
bool hide_tag_from_client(app_pc tag);
dr_signal_action_t instrument_signal(dcontext_t *dcontext, dr_siginfo_t *siginfo);

app_pc get_client_base(client_id_t client_id);

DR_API
/**
 * Decodes and then prints the instruction at address \p pc to file \p outfile.
 * Prior to the instruction the address is printed if \p show_pc and the raw
 * bytes are printed if \p show_bytes.
 * The default is to use AT&T-style syntax, unless the \ref op_syntax_intel
 * "-syntax_intel" runtime option is specified.
 * Returns the address of the subsequent instruction, or NULL if the instruction
 * at \p pc is invalid.
 */
byte *
disassemble_with_info(dcontext_t *dcontext, byte *pc, file_t outfile,
                      bool show_pc, bool show_bytes);


/**
 * Data structure passed to a restore_state_ex event handler (see
 * dr_register_restore_state_ex_event()).  Contains the machine
 * context at the translation point and other translation
 * information.
 */
typedef struct _dr_restore_state_info_t {
    /** The application machine state at the translation point. */
    dr_mcontext_t *mcontext;
    /** Whether raw_mcontext is valid. */
    bool raw_mcontext_valid;
    /**
     * The raw pre-translated machine state at the translation
     * interruption point inside the code cache.  Clients are
     * cautioned when examining code cache instructions to not rely on
     * any details of code inserted other than their own.
     */
    dr_mcontext_t raw_mcontext;
    /**
     * Information about the code fragment inside the code cache
     * at the translation interruption point.
     */
    dr_fault_fragment_info_t fragment_info;
} dr_restore_state_info_t;
/* DR_API EXPORT END */
#endif
