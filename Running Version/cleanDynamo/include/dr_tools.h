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

#ifndef _DR_TOOLS_H_
#define _DR_TOOLS_H_ 1

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



/**************************************************
 * MODULE INFORMATION TYPES
 */

/**
 * Type used for dr_get_proc_address().  This can be obtained from the
 * #_module_data_t structure.  It is equivalent to the base address of
 * the module on both Windows and Linux.
 */
typedef void * module_handle_t;

#ifdef WINDOWS

#define MODULE_FILE_VERSION_INVALID ULLONG_MAX

/**
 * Used to hold .rsrc section version number information. This number is usually
 * presented as p1.p2.p3.p4 by PE parsing tools.
 */
typedef union _version_number_t {
    uint64 version;  /**< Representation as a 64-bit integer. */
    struct {
        uint ms;     /**< */
        uint ls;     /**< */
    } version_uint;  /**< Representation as 2 32-bit integers. */
    struct {
        ushort p2;   /**< */
        ushort p1;   /**< */
        ushort p4;   /**< */
        ushort p3;   /**< */
    } version_parts; /**< Representation as 4 16-bit integers. */
} version_number_t;

#endif



/**
 * Holds the names of a module.  This structure contains multiple
 * fields corresponding to different sources of a module name.  Note
 * that some of these names may not exist for certain modules.  It is
 * highly likely, however, that at least one name is available.  Use
 * dr_module_preferred_name() on the parent _module_data_t to get the
 * preferred name of the module.
 */
typedef struct _module_names_t {
    const char *module_name; /**< On windows this name comes from the PE header exports
                              * section (NULL if the module has no exports section).  On
                              * Linux the name will come from the ELF DYNAMIC program 
                              * header (NULL if the module has no SONAME entry). */
    const char *file_name; /**< The file name used to load this module. Note - on Windows
                            * this is not always available. */
#ifdef WINDOWS
    const char *exe_name; /**< If this module is the main executable of this process then
                           * this is the executable name used to launch the process (NULL
                           * for all other modules). */
    const char *rsrc_name; /**< The internal name given to the module in its resource
                            * section. Will be NULL if the module has no resource section
                            * or doesn't set this field within it. */
#else /* LINUX */
    uint64 inode; /**< The inode of the module file mapped in. */
#endif
} module_names_t;




/**
 * Frees a module_data_t returned by dr_module_iterator_next(), dr_lookup_module(),
 * dr_lookup_module_by_name(), or dr_copy_module_data(). \note Should NOT be used with
 * a module_data_t obtained as part of a module load or unload event.
 */
void
dr_free_module_data(module_data_t *data);

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
disassemble_with_info(void *drcontext, byte *pc, file_t outfile,
                      bool show_pc, bool show_bytes);


/**
 * Returns true iff \p instr is a conditional branch: OP_jcc, OP_jcc_short,
 * OP_loop*, or OP_jecxz.
 */
bool
instr_is_cbr(instr_t *instr);

/**
 * Sets the translation pointer for \p instr, used to recreate the
 * application address corresponding to this instruction.  When adding
 * or modifying instructions that are to be considered application
 * instructions (i.e., non meta-instructions: see
 * #instr_ok_to_mangle), the translation should always be set.  Pick
 * the application address that if executed will be equivalent to
 * restarting \p instr.
 * Returns the supplied \p instr (for easy chaining).  Use
 * #instr_get_app_pc to see the current value of the translation.
 */
instr_t *
instr_set_translation(instr_t *instr, app_pc addr);

/**
 * Returns a copy of \p orig with separately allocated memory for
 * operands and raw bytes if they were present in \p orig.
 */
instr_t *
instr_clone(void *drcontext, instr_t *orig);

/**
 * Returns \p instr's source operand at position \p pos (0-based).
 */
opnd_t
instr_get_src(instr_t *instr, uint pos);

/** Returns true iff \p opnd is a (near or far) instr_t pointer address operand. */
bool
opnd_is_instr(opnd_t opnd);

/** Assumes \p opnd is an instr_t pointer, returns its value. */
instr_t*
opnd_get_instr(opnd_t opnd);

/** Returns true iff \p opnd is a far instr_t pointer address operand. */
bool
opnd_is_far_instr(opnd_t opnd);

/**
 * Sets \p instr's source operand at position \p pos to be \p opnd.
 * Also calls instr_set_raw_bits_valid(\p instr, false) and
 * instr_set_operands_valid(\p instr, true).
 */
void
instr_set_src(instr_t *instr, uint pos, opnd_t opnd);

/**
 * Assumes \p opnd is a far program address.
 * Returns \p opnd's segment, a segment selector (not a SEG_ constant).
 */
ushort
opnd_get_segment_selector(opnd_t opnd);

/**
 * Returns a far instr_t pointer address with value \p seg_selector:instr.
 * \p seg_selector is a segment selector, not a SEG_ constant.
 */
opnd_t
opnd_create_far_instr(ushort seg_selector, instr_t *instr);

/** Returns an instr_t pointer address with value \p instr. */
opnd_t
opnd_create_instr(instr_t *instr);

/**
 * Assumes that \p cti_instr is a control transfer instruction
 * Returns the first source operand of \p cti_instr (its target).
 */
opnd_t
instr_get_target(instr_t *cti_instr);

/** Assumes \p opnd is a (near or far) program address, returns its value. */
app_pc
opnd_get_pc(opnd_t opnd);

/**
 * Calculates the size, in bytes, of the memory read or write of
 * the instr at \p pc.  If the instruction is a repeating string instruction,
 * considers only one iteration.
 * Returns the pc of the following instruction.
 * If the instruction at \p pc does not reference memory, or is invalid,
 * returns NULL.
 */
app_pc
decode_memory_reference_size(void *drcontext, app_pc pc, uint *size_in_bytes);

/**
 * Returns true iff \p instr's opcode is valid.
 * If the opcode is ever set to other than OP_INVALID or OP_UNDECODED it is assumed
 * to be valid.  However, calling instr_get_opcode() will attempt to
 * decode a valid opcode, hence the purpose of this routine.
 */
bool
instr_opcode_valid(instr_t *instr);

/**
 * Returns true iff \p instr is a control transfer instruction of any kind
 * This includes OP_jcc, OP_jcc_short, OP_loop*, OP_jecxz, OP_call*, and OP_jmp*.
 */
bool
instr_is_cti(instr_t *instr);

/**
 * Convenience routine that returns an initialized instr_t allocated on the
 * thread-local heap with opcode \p opcode and no sources or destinations.
 */
instr_t *
instr_create_0dst_0src(void *drcontext, int opcode);

/** Returns \p instr's opcode (an OP_ constant). */
int
instr_get_opcode(instr_t *instr);

/**
 * Performs address calculation in the same manner as
 * instr_compute_address() but handles multiple memory operands.  The
 * \p index parameter should be initially set to 0 and then
 * incremented with each successive call until this routine returns
 * false, which indicates that there are no more memory operands.  The
 * address of each is computed in the same manner as
 * instr_compute_address() and returned in \p addr; whether it is a
 * write is returned in \p is_write.  Either or both OUT variables can
 * be NULL.
 */
bool
instr_compute_address_ex(instr_t *instr, dr_mcontext_t *mc, uint index,
                         OUT app_pc *addr, OUT bool *write);

/** Returns true iff \p instr is an "undefined" instruction (ud2) */
bool
instr_is_undefined(instr_t *instr);

/**
 * Returns true iff \p instr's opcode is NOT OP_INVALID.
 * Not to be confused with an invalid opcode, which can be OP_INVALID or
 * OP_UNDECODED.  OP_INVALID means an instruction with no valid fields:
 * raw bits (may exist but do not correspond to a valid instr), opcode,
 * eflags, or operands.  It could be an uninitialized
 * instruction or the result of decoding an invalid sequence of bytes.
 */
bool
instr_valid(instr_t *instr);

/**
 * Returns true iff \p instr is used to implement system calls: OP_int with a
 * source operand of 0x80 on linux or 0x2e on windows, or OP_sysenter,
 * or OP_syscall, or #instr_is_wow64_syscall() for WOW64.
 */
bool
instr_is_syscall(instr_t *instr);

/** Returns true iff \p instr's raw bits are a valid encoding of instr. */
bool
instr_raw_bits_valid(instr_t *instr);

/****************************************************************************
 * BINARY TRACE DUMP FORMAT
 */

/**<pre>
 * Binary trace dump format:
 * the file starts with a tracedump_file_header_t
 * then, for each trace:
     struct _tracedump_trace_header
     if num_bbs > 0 # tracedump_origins
       foreach bb:
           app_pc tag;
           int bb_code_size;
           byte code[bb_code_size];
     endif
     foreach exit:
       struct _tracedump_stub_data
       if linkcount_size > 0
         linkcount_type_t count; # sizeof == linkcount_size
       endif
       if separate from body
       (i.e., exit_stub < cache_start_pc || exit_stub >= cache_start_pc+code_size):
           byte stub_code[15]; # all separate stubs are 15
       endif
     endfor
     byte code[code_size];
 * if the -tracedump_threshold option (deprecated) was specified:
     int num_below_treshold
     linkcount_type_t count_below_threshold
   endif
</pre>
 */
typedef struct _tracedump_file_header_t {
    int version;           /**< The DynamoRIO version that created the file. */
    bool x64;              /**< Whether a 64-bit DynamoRIO library created the file. */
    int linkcount_size;    /**< Size of the linkcount (linkcounts are deprecated). */
} tracedump_file_header_t;

/** Header for an individual trace in a binary trace dump file. */
typedef struct _tracedump_trace_header_t {
    int frag_id;           /**< Identifier for the trace. */
    app_pc tag;            /**< Application address for start of trace. */
    app_pc cache_start_pc; /**< Code cache address of start of trace. */
    int entry_offs;        /**< Offset into trace of normal entry. */
    int num_exits;         /**< Number of exits from the trace. */
    int code_size;         /**< Length of the trace in the code cache. */
    uint num_bbs;          /**< Number of constituent basic blocks making up the trace. */
    bool x64;              /**< Whether the trace contains 64-bit code. */
} tracedump_trace_header_t;

/** Size of tag + bb_code_size fields for each bb. */
#define BB_ORIGIN_HEADER_SIZE (sizeof(app_pc)+sizeof(int))

/**< tracedump_stub_data_t.stub_size will not exceed this value. */
#define SEPARATE_STUB_MAX_SIZE IF_X64_ELSE(23, 15)

/** The format of a stub in a trace dump file. */
typedef struct _tracedump_stub_data {
    int cti_offs;   /**< Offset from the start of the fragment. */
    /* stub_pc is an absolute address, since can be separate from body. */
    app_pc stub_pc; /**< Code cache address of the stub. */
    app_pc target;  /**< Target of the stub. */
    bool linked;    /**< Whether the stub is linked to its target. */
    int stub_size;  /**< Length of stub_code array */
    /****** the rest of the fields are optional and may not be present! ******/
    union {
        uint count32; /**< 32-bit exit execution count. */
        uint64 count64; /**< 64-bit exit execution count. */
    } count; /**< Which field is present depends on the first entry in
              * the file, which indicates the linkcount size. */
    /** Code for exit stubs.  Only present if:
     *   stub_pc < cache_start_pc ||
     *   stub_pc >= cache_start_pc+code_size). 
     * The actual size of the array varies and is indicated by the stub_size field.
     */
    byte stub_code[1/*variable-sized*/];
} tracedump_stub_data_t;

/** The last offset into tracedump_stub_data_t of always-present fields. */
#define STUB_DATA_FIXED_SIZE (offsetof(tracedump_stub_data_t, count))

/****************************************************************************/



#endif /* _DR_TOOLS_H_ */
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



/**************************************************
 * MODULE INFORMATION TYPES
 */

/**
 * Type used for dr_get_proc_address().  This can be obtained from the
 * #_module_data_t structure.  It is equivalent to the base address of
 * the module on both Windows and Linux.
 */
typedef void * module_handle_t;

#ifdef WINDOWS

#define MODULE_FILE_VERSION_INVALID ULLONG_MAX

/**
 * Used to hold .rsrc section version number information. This number is usually
 * presented as p1.p2.p3.p4 by PE parsing tools.
 */
typedef union _version_number_t {
    uint64 version;  /**< Representation as a 64-bit integer. */
    struct {
        uint ms;     /**< */
        uint ls;     /**< */
    } version_uint;  /**< Representation as 2 32-bit integers. */
    struct {
        ushort p2;   /**< */
        ushort p1;   /**< */
        ushort p4;   /**< */
        ushort p3;   /**< */
    } version_parts; /**< Representation as 4 16-bit integers. */
} version_number_t;

#endif



/**
 * Holds the names of a module.  This structure contains multiple
 * fields corresponding to different sources of a module name.  Note
 * that some of these names may not exist for certain modules.  It is
 * highly likely, however, that at least one name is available.  Use
 * dr_module_preferred_name() on the parent _module_data_t to get the
 * preferred name of the module.
 */
typedef struct _module_names_t {
    const char *module_name; /**< On windows this name comes from the PE header exports
                              * section (NULL if the module has no exports section).  On
                              * Linux the name will come from the ELF DYNAMIC program 
                              * header (NULL if the module has no SONAME entry). */
    const char *file_name; /**< The file name used to load this module. Note - on Windows
                            * this is not always available. */
#ifdef WINDOWS
    const char *exe_name; /**< If this module is the main executable of this process then
                           * this is the executable name used to launch the process (NULL
                           * for all other modules). */
    const char *rsrc_name; /**< The internal name given to the module in its resource
                            * section. Will be NULL if the module has no resource section
                            * or doesn't set this field within it. */
#else /* LINUX */
    uint64 inode; /**< The inode of the module file mapped in. */
#endif
} module_names_t;




/**
 * Frees a module_data_t returned by dr_module_iterator_next(), dr_lookup_module(),
 * dr_lookup_module_by_name(), or dr_copy_module_data(). \note Should NOT be used with
 * a module_data_t obtained as part of a module load or unload event.
 */
void
dr_free_module_data(module_data_t *data);

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
disassemble_with_info(void *drcontext, byte *pc, file_t outfile,
                      bool show_pc, bool show_bytes);


/**
 * Returns true iff \p instr is a conditional branch: OP_jcc, OP_jcc_short,
 * OP_loop*, or OP_jecxz.
 */
bool
instr_is_cbr(instr_t *instr);

/**
 * Sets the translation pointer for \p instr, used to recreate the
 * application address corresponding to this instruction.  When adding
 * or modifying instructions that are to be considered application
 * instructions (i.e., non meta-instructions: see
 * #instr_ok_to_mangle), the translation should always be set.  Pick
 * the application address that if executed will be equivalent to
 * restarting \p instr.
 * Returns the supplied \p instr (for easy chaining).  Use
 * #instr_get_app_pc to see the current value of the translation.
 */
instr_t *
instr_set_translation(instr_t *instr, app_pc addr);

/**
 * Returns a copy of \p orig with separately allocated memory for
 * operands and raw bytes if they were present in \p orig.
 */
instr_t *
instr_clone(void *drcontext, instr_t *orig);

/**
 * Returns \p instr's source operand at position \p pos (0-based).
 */
opnd_t
instr_get_src(instr_t *instr, uint pos);

/** Returns true iff \p opnd is a (near or far) instr_t pointer address operand. */
bool
opnd_is_instr(opnd_t opnd);

/** Assumes \p opnd is an instr_t pointer, returns its value. */
instr_t*
opnd_get_instr(opnd_t opnd);

/** Returns true iff \p opnd is a far instr_t pointer address operand. */
bool
opnd_is_far_instr(opnd_t opnd);

/**
 * Sets \p instr's source operand at position \p pos to be \p opnd.
 * Also calls instr_set_raw_bits_valid(\p instr, false) and
 * instr_set_operands_valid(\p instr, true).
 */
void
instr_set_src(instr_t *instr, uint pos, opnd_t opnd);

/**
 * Assumes \p opnd is a far program address.
 * Returns \p opnd's segment, a segment selector (not a SEG_ constant).
 */
ushort
opnd_get_segment_selector(opnd_t opnd);

/**
 * Returns a far instr_t pointer address with value \p seg_selector:instr.
 * \p seg_selector is a segment selector, not a SEG_ constant.
 */
opnd_t
opnd_create_far_instr(ushort seg_selector, instr_t *instr);

/** Returns an instr_t pointer address with value \p instr. */
opnd_t
opnd_create_instr(instr_t *instr);

/**
 * Assumes that \p cti_instr is a control transfer instruction
 * Returns the first source operand of \p cti_instr (its target).
 */
opnd_t
instr_get_target(instr_t *cti_instr);

/** Assumes \p opnd is a (near or far) program address, returns its value. */
app_pc
opnd_get_pc(opnd_t opnd);

/**
 * Calculates the size, in bytes, of the memory read or write of
 * the instr at \p pc.  If the instruction is a repeating string instruction,
 * considers only one iteration.
 * Returns the pc of the following instruction.
 * If the instruction at \p pc does not reference memory, or is invalid,
 * returns NULL.
 */
app_pc
decode_memory_reference_size(void *drcontext, app_pc pc, uint *size_in_bytes);

/**
 * Returns true iff \p instr's opcode is valid.
 * If the opcode is ever set to other than OP_INVALID or OP_UNDECODED it is assumed
 * to be valid.  However, calling instr_get_opcode() will attempt to
 * decode a valid opcode, hence the purpose of this routine.
 */
bool
instr_opcode_valid(instr_t *instr);

/**
 * Returns true iff \p instr is a control transfer instruction of any kind
 * This includes OP_jcc, OP_jcc_short, OP_loop*, OP_jecxz, OP_call*, and OP_jmp*.
 */
bool
instr_is_cti(instr_t *instr);

/**
 * Convenience routine that returns an initialized instr_t allocated on the
 * thread-local heap with opcode \p opcode and no sources or destinations.
 */
instr_t *
instr_create_0dst_0src(void *drcontext, int opcode);

/** Returns \p instr's opcode (an OP_ constant). */
int
instr_get_opcode(instr_t *instr);

/**
 * Performs address calculation in the same manner as
 * instr_compute_address() but handles multiple memory operands.  The
 * \p index parameter should be initially set to 0 and then
 * incremented with each successive call until this routine returns
 * false, which indicates that there are no more memory operands.  The
 * address of each is computed in the same manner as
 * instr_compute_address() and returned in \p addr; whether it is a
 * write is returned in \p is_write.  Either or both OUT variables can
 * be NULL.
 */
bool
instr_compute_address_ex(instr_t *instr, dr_mcontext_t *mc, uint index,
                         OUT app_pc *addr, OUT bool *write);

/** Returns true iff \p instr is an "undefined" instruction (ud2) */
bool
instr_is_undefined(instr_t *instr);

/**
 * Returns true iff \p instr's opcode is NOT OP_INVALID.
 * Not to be confused with an invalid opcode, which can be OP_INVALID or
 * OP_UNDECODED.  OP_INVALID means an instruction with no valid fields:
 * raw bits (may exist but do not correspond to a valid instr), opcode,
 * eflags, or operands.  It could be an uninitialized
 * instruction or the result of decoding an invalid sequence of bytes.
 */
bool
instr_valid(instr_t *instr);

/**
 * Returns true iff \p instr is used to implement system calls: OP_int with a
 * source operand of 0x80 on linux or 0x2e on windows, or OP_sysenter,
 * or OP_syscall, or #instr_is_wow64_syscall() for WOW64.
 */
bool
instr_is_syscall(instr_t *instr);

/** Returns true iff \p instr's raw bits are a valid encoding of instr. */
bool
instr_raw_bits_valid(instr_t *instr);

/****************************************************************************
 * BINARY TRACE DUMP FORMAT
 */

/**<pre>
 * Binary trace dump format:
 * the file starts with a tracedump_file_header_t
 * then, for each trace:
     struct _tracedump_trace_header
     if num_bbs > 0 # tracedump_origins
       foreach bb:
           app_pc tag;
           int bb_code_size;
           byte code[bb_code_size];
     endif
     foreach exit:
       struct _tracedump_stub_data
       if linkcount_size > 0
         linkcount_type_t count; # sizeof == linkcount_size
       endif
       if separate from body
       (i.e., exit_stub < cache_start_pc || exit_stub >= cache_start_pc+code_size):
           byte stub_code[15]; # all separate stubs are 15
       endif
     endfor
     byte code[code_size];
 * if the -tracedump_threshold option (deprecated) was specified:
     int num_below_treshold
     linkcount_type_t count_below_threshold
   endif
</pre>
 */
typedef struct _tracedump_file_header_t {
    int version;           /**< The DynamoRIO version that created the file. */
    bool x64;              /**< Whether a 64-bit DynamoRIO library created the file. */
    int linkcount_size;    /**< Size of the linkcount (linkcounts are deprecated). */
} tracedump_file_header_t;

/** Header for an individual trace in a binary trace dump file. */
typedef struct _tracedump_trace_header_t {
    int frag_id;           /**< Identifier for the trace. */
    app_pc tag;            /**< Application address for start of trace. */
    app_pc cache_start_pc; /**< Code cache address of start of trace. */
    int entry_offs;        /**< Offset into trace of normal entry. */
    int num_exits;         /**< Number of exits from the trace. */
    int code_size;         /**< Length of the trace in the code cache. */
    uint num_bbs;          /**< Number of constituent basic blocks making up the trace. */
    bool x64;              /**< Whether the trace contains 64-bit code. */
} tracedump_trace_header_t;

/** Size of tag + bb_code_size fields for each bb. */
#define BB_ORIGIN_HEADER_SIZE (sizeof(app_pc)+sizeof(int))

/**< tracedump_stub_data_t.stub_size will not exceed this value. */
#define SEPARATE_STUB_MAX_SIZE IF_X64_ELSE(23, 15)

/** The format of a stub in a trace dump file. */
typedef struct _tracedump_stub_data {
    int cti_offs;   /**< Offset from the start of the fragment. */
    /* stub_pc is an absolute address, since can be separate from body. */
    app_pc stub_pc; /**< Code cache address of the stub. */
    app_pc target;  /**< Target of the stub. */
    bool linked;    /**< Whether the stub is linked to its target. */
    int stub_size;  /**< Length of stub_code array */
    /****** the rest of the fields are optional and may not be present! ******/
    union {
        uint count32; /**< 32-bit exit execution count. */
        uint64 count64; /**< 64-bit exit execution count. */
    } count; /**< Which field is present depends on the first entry in
              * the file, which indicates the linkcount size. */
    /** Code for exit stubs.  Only present if:
     *   stub_pc < cache_start_pc ||
     *   stub_pc >= cache_start_pc+code_size). 
     * The actual size of the array varies and is indicated by the stub_size field.
     */
    byte stub_code[1/*variable-sized*/];
} tracedump_stub_data_t;

/** The last offset into tracedump_stub_data_t of always-present fields. */
#define STUB_DATA_FIXED_SIZE (offsetof(tracedump_stub_data_t, count))

/****************************************************************************/



#endif /* _DR_TOOLS_H_ */
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



/**************************************************
 * MODULE INFORMATION TYPES
 */

/**
 * Type used for dr_get_proc_address().  This can be obtained from the
 * #_module_data_t structure.  It is equivalent to the base address of
 * the module on both Windows and Linux.
 */
typedef void * module_handle_t;

#ifdef WINDOWS

#define MODULE_FILE_VERSION_INVALID ULLONG_MAX

/**
 * Used to hold .rsrc section version number information. This number is usually
 * presented as p1.p2.p3.p4 by PE parsing tools.
 */
typedef union _version_number_t {
    uint64 version;  /**< Representation as a 64-bit integer. */
    struct {
        uint ms;     /**< */
        uint ls;     /**< */
    } version_uint;  /**< Representation as 2 32-bit integers. */
    struct {
        ushort p2;   /**< */
        ushort p1;   /**< */
        ushort p4;   /**< */
        ushort p3;   /**< */
    } version_parts; /**< Representation as 4 16-bit integers. */
} version_number_t;

#endif



/**
 * Holds the names of a module.  This structure contains multiple
 * fields corresponding to different sources of a module name.  Note
 * that some of these names may not exist for certain modules.  It is
 * highly likely, however, that at least one name is available.  Use
 * dr_module_preferred_name() on the parent _module_data_t to get the
 * preferred name of the module.
 */
typedef struct _module_names_t {
    const char *module_name; /**< On windows this name comes from the PE header exports
                              * section (NULL if the module has no exports section).  On
                              * Linux the name will come from the ELF DYNAMIC program 
                              * header (NULL if the module has no SONAME entry). */
    const char *file_name; /**< The file name used to load this module. Note - on Windows
                            * this is not always available. */
#ifdef WINDOWS
    const char *exe_name; /**< If this module is the main executable of this process then
                           * this is the executable name used to launch the process (NULL
                           * for all other modules). */
    const char *rsrc_name; /**< The internal name given to the module in its resource
                            * section. Will be NULL if the module has no resource section
                            * or doesn't set this field within it. */
#else /* LINUX */
    uint64 inode; /**< The inode of the module file mapped in. */
#endif
} module_names_t;




/**
 * Frees a module_data_t returned by dr_module_iterator_next(), dr_lookup_module(),
 * dr_lookup_module_by_name(), or dr_copy_module_data(). \note Should NOT be used with
 * a module_data_t obtained as part of a module load or unload event.
 */
void
dr_free_module_data(module_data_t *data);

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
disassemble_with_info(void *drcontext, byte *pc, file_t outfile,
                      bool show_pc, bool show_bytes);


/**
 * Returns true iff \p instr is a conditional branch: OP_jcc, OP_jcc_short,
 * OP_loop*, or OP_jecxz.
 */
bool
instr_is_cbr(instr_t *instr);

/**
 * Sets the translation pointer for \p instr, used to recreate the
 * application address corresponding to this instruction.  When adding
 * or modifying instructions that are to be considered application
 * instructions (i.e., non meta-instructions: see
 * #instr_ok_to_mangle), the translation should always be set.  Pick
 * the application address that if executed will be equivalent to
 * restarting \p instr.
 * Returns the supplied \p instr (for easy chaining).  Use
 * #instr_get_app_pc to see the current value of the translation.
 */
instr_t *
instr_set_translation(instr_t *instr, app_pc addr);

/**
 * Returns a copy of \p orig with separately allocated memory for
 * operands and raw bytes if they were present in \p orig.
 */
instr_t *
instr_clone(void *drcontext, instr_t *orig);

/**
 * Returns \p instr's source operand at position \p pos (0-based).
 */
opnd_t
instr_get_src(instr_t *instr, uint pos);

/** Returns true iff \p opnd is a (near or far) instr_t pointer address operand. */
bool
opnd_is_instr(opnd_t opnd);

/** Assumes \p opnd is an instr_t pointer, returns its value. */
instr_t*
opnd_get_instr(opnd_t opnd);

/** Returns true iff \p opnd is a far instr_t pointer address operand. */
bool
opnd_is_far_instr(opnd_t opnd);

/**
 * Sets \p instr's source operand at position \p pos to be \p opnd.
 * Also calls instr_set_raw_bits_valid(\p instr, false) and
 * instr_set_operands_valid(\p instr, true).
 */
void
instr_set_src(instr_t *instr, uint pos, opnd_t opnd);

/**
 * Assumes \p opnd is a far program address.
 * Returns \p opnd's segment, a segment selector (not a SEG_ constant).
 */
ushort
opnd_get_segment_selector(opnd_t opnd);

/**
 * Returns a far instr_t pointer address with value \p seg_selector:instr.
 * \p seg_selector is a segment selector, not a SEG_ constant.
 */
opnd_t
opnd_create_far_instr(ushort seg_selector, instr_t *instr);

/** Returns an instr_t pointer address with value \p instr. */
opnd_t
opnd_create_instr(instr_t *instr);

/**
 * Assumes that \p cti_instr is a control transfer instruction
 * Returns the first source operand of \p cti_instr (its target).
 */
opnd_t
instr_get_target(instr_t *cti_instr);

/** Assumes \p opnd is a (near or far) program address, returns its value. */
app_pc
opnd_get_pc(opnd_t opnd);

/**
 * Calculates the size, in bytes, of the memory read or write of
 * the instr at \p pc.  If the instruction is a repeating string instruction,
 * considers only one iteration.
 * Returns the pc of the following instruction.
 * If the instruction at \p pc does not reference memory, or is invalid,
 * returns NULL.
 */
app_pc
decode_memory_reference_size(void *drcontext, app_pc pc, uint *size_in_bytes);

/**
 * Returns true iff \p instr's opcode is valid.
 * If the opcode is ever set to other than OP_INVALID or OP_UNDECODED it is assumed
 * to be valid.  However, calling instr_get_opcode() will attempt to
 * decode a valid opcode, hence the purpose of this routine.
 */
bool
instr_opcode_valid(instr_t *instr);

/**
 * Returns true iff \p instr is a control transfer instruction of any kind
 * This includes OP_jcc, OP_jcc_short, OP_loop*, OP_jecxz, OP_call*, and OP_jmp*.
 */
bool
instr_is_cti(instr_t *instr);

/**
 * Convenience routine that returns an initialized instr_t allocated on the
 * thread-local heap with opcode \p opcode and no sources or destinations.
 */
instr_t *
instr_create_0dst_0src(void *drcontext, int opcode);

/** Returns \p instr's opcode (an OP_ constant). */
int
instr_get_opcode(instr_t *instr);

/**
 * Performs address calculation in the same manner as
 * instr_compute_address() but handles multiple memory operands.  The
 * \p index parameter should be initially set to 0 and then
 * incremented with each successive call until this routine returns
 * false, which indicates that there are no more memory operands.  The
 * address of each is computed in the same manner as
 * instr_compute_address() and returned in \p addr; whether it is a
 * write is returned in \p is_write.  Either or both OUT variables can
 * be NULL.
 */
bool
instr_compute_address_ex(instr_t *instr, dr_mcontext_t *mc, uint index,
                         OUT app_pc *addr, OUT bool *write);

/** Returns true iff \p instr is an "undefined" instruction (ud2) */
bool
instr_is_undefined(instr_t *instr);

/**
 * Returns true iff \p instr's opcode is NOT OP_INVALID.
 * Not to be confused with an invalid opcode, which can be OP_INVALID or
 * OP_UNDECODED.  OP_INVALID means an instruction with no valid fields:
 * raw bits (may exist but do not correspond to a valid instr), opcode,
 * eflags, or operands.  It could be an uninitialized
 * instruction or the result of decoding an invalid sequence of bytes.
 */
bool
instr_valid(instr_t *instr);

/**
 * Returns true iff \p instr is used to implement system calls: OP_int with a
 * source operand of 0x80 on linux or 0x2e on windows, or OP_sysenter,
 * or OP_syscall, or #instr_is_wow64_syscall() for WOW64.
 */
bool
instr_is_syscall(instr_t *instr);

/** Returns true iff \p instr's raw bits are a valid encoding of instr. */
bool
instr_raw_bits_valid(instr_t *instr);

/****************************************************************************
 * BINARY TRACE DUMP FORMAT
 */

/**<pre>
 * Binary trace dump format:
 * the file starts with a tracedump_file_header_t
 * then, for each trace:
     struct _tracedump_trace_header
     if num_bbs > 0 # tracedump_origins
       foreach bb:
           app_pc tag;
           int bb_code_size;
           byte code[bb_code_size];
     endif
     foreach exit:
       struct _tracedump_stub_data
       if linkcount_size > 0
         linkcount_type_t count; # sizeof == linkcount_size
       endif
       if separate from body
       (i.e., exit_stub < cache_start_pc || exit_stub >= cache_start_pc+code_size):
           byte stub_code[15]; # all separate stubs are 15
       endif
     endfor
     byte code[code_size];
 * if the -tracedump_threshold option (deprecated) was specified:
     int num_below_treshold
     linkcount_type_t count_below_threshold
   endif
</pre>
 */
typedef struct _tracedump_file_header_t {
    int version;           /**< The DynamoRIO version that created the file. */
    bool x64;              /**< Whether a 64-bit DynamoRIO library created the file. */
    int linkcount_size;    /**< Size of the linkcount (linkcounts are deprecated). */
} tracedump_file_header_t;

/** Header for an individual trace in a binary trace dump file. */
typedef struct _tracedump_trace_header_t {
    int frag_id;           /**< Identifier for the trace. */
    app_pc tag;            /**< Application address for start of trace. */
    app_pc cache_start_pc; /**< Code cache address of start of trace. */
    int entry_offs;        /**< Offset into trace of normal entry. */
    int num_exits;         /**< Number of exits from the trace. */
    int code_size;         /**< Length of the trace in the code cache. */
    uint num_bbs;          /**< Number of constituent basic blocks making up the trace. */
    bool x64;              /**< Whether the trace contains 64-bit code. */
} tracedump_trace_header_t;

/** Size of tag + bb_code_size fields for each bb. */
#define BB_ORIGIN_HEADER_SIZE (sizeof(app_pc)+sizeof(int))

/**< tracedump_stub_data_t.stub_size will not exceed this value. */
#define SEPARATE_STUB_MAX_SIZE IF_X64_ELSE(23, 15)

/** The format of a stub in a trace dump file. */
typedef struct _tracedump_stub_data {
    int cti_offs;   /**< Offset from the start of the fragment. */
    /* stub_pc is an absolute address, since can be separate from body. */
    app_pc stub_pc; /**< Code cache address of the stub. */
    app_pc target;  /**< Target of the stub. */
    bool linked;    /**< Whether the stub is linked to its target. */
    int stub_size;  /**< Length of stub_code array */
    /****** the rest of the fields are optional and may not be present! ******/
    union {
        uint count32; /**< 32-bit exit execution count. */
        uint64 count64; /**< 64-bit exit execution count. */
    } count; /**< Which field is present depends on the first entry in
              * the file, which indicates the linkcount size. */
    /** Code for exit stubs.  Only present if:
     *   stub_pc < cache_start_pc ||
     *   stub_pc >= cache_start_pc+code_size). 
     * The actual size of the array varies and is indicated by the stub_size field.
     */
    byte stub_code[1/*variable-sized*/];
} tracedump_stub_data_t;

/** The last offset into tracedump_stub_data_t of always-present fields. */
#define STUB_DATA_FIXED_SIZE (offsetof(tracedump_stub_data_t, count))

/****************************************************************************/



#endif /* _DR_TOOLS_H_ */
