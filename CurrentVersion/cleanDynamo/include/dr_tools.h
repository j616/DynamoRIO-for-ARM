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
