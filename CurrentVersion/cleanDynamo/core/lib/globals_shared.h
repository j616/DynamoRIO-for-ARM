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
 * globals_shared.h
 *
 * definitions to be shared with core-external modules
 *
 */

/* FIXME: dynamorio references should be removed throughout */

#ifndef _GLOBALS_SHARED_H_
#define _GLOBALS_SHARED_H_

/* if not Linux, we assume Windows */
#ifndef LINUX
#  ifndef WINDOWS
#    define WINDOWS
#  endif
#endif

/* DR_API EXPORT TOFILE dr_defines.h */
/* DR_API EXPORT BEGIN */
/****************************************************************************
 * GENERAL TYPEDEFS AND DEFINES
 */

/**
 * @file dr_defines.h
 * @brief Basic defines and type definitions.
 */

#ifdef API_EXPORT_ONLY
/* A client's target operating system and architecture must be specified. */
#if (!defined(LINUX) && !defined(WINDOWS)) || (defined(LINUX) && defined(WINDOWS))
# error Target operating system unspecified: must define either WINDOWS xor LINUX
#endif
#endif

#ifdef API_EXPORT_ONLY
#if (!defined(X86_64) && !defined(X86_32)) || (defined(X86_64) && defined(X86_32))
# error Target architecture unspecified: must define either X86_64 xor X86_32
#endif
#endif

#if defined(X86_64) && !defined(X64)
# define X64
#endif

#ifdef API_EXPORT_ONLY
#ifdef WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <winbase.h>
#else
#  include <stdio.h>
#  include <stdlib.h>
#endif
#endif
/* DR_API EXPORT END */
#include <limits.h>  /* for USHRT_MAX */
#ifdef LINUX
#  include <sys/types.h>        /* Fix for case 5341. */
#  include <signal.h>
#endif
/* DR_API EXPORT BEGIN */
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

#ifdef AVOID_API_EXPORT
/* We want a consistent size so we stay away from MAX_PATH.
 * MAX_PATH is 260 on Windows, but 4096 on Linux, should up this.
 * FIXME: should undef MAX_PATH and define it to an error-producing value
 * and clean up all uses of it
 */
#endif
/** Cross-platform maximum file path length. */
#define MAXIMUM_PATH      260 

/* DR_API EXPORT END */
/* DR_API EXPORT VERBATIM */
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
/* DR_API EXPORT END */
/* DR_API EXPORT BEGIN */

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

#ifdef API_EXPORT_ONLY
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

#ifdef AVOID_API_EXPORT
/* Note that we considered using a 128-bit GUID for the client ID,
 * but decided it was unnecessary since the client registration
 * routine will complain about conflicting IDs.  Also, we're storing
 * this value in the registry, so no reason to make it any longer
 * than we have to.
 */
#endif
/**
 * ID used to uniquely identify a client.  This value is set at
 * client registration and passed to the client in dr_init().
 */
typedef uint client_id_t;

#ifdef API_EXPORT_ONLY
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
#endif /* API_EXPORT_ONLY */

#ifndef IN
# define IN /* marks input param */
#endif
#ifndef OUT
# define OUT /* marks output param */
#endif
#ifndef INOUT
# define INOUT /* marks input+output param */
#endif

/* DR_API EXPORT END */

#ifdef X64
# define POINTER_MAX ULLONG_MAX
# define SSIZE_T_MAX LLONG_MAX
# define POINTER_MAX_32BIT ((ptr_uint_t)UINT_MAX) 
#else
# define POINTER_MAX UINT_MAX
# define SSIZE_T_MAX INT_MAX
#endif

#define PTR_UINT_0       ((ptr_uint_t)0U)
#define PTR_UINT_1       ((ptr_uint_t)1U)
#define PTR_UINT_MINUS_1 ((ptr_uint_t)-1)

#define MAX_CLIENT_LIBS 16

/* FIXME: these macros double-evaluate their args -- if we can't trust
 * compiler to do CSE, should we replace w/ inline functions?  would need
 * separate signed and unsigned versions
 */
#ifndef DR_DO_NOT_DEFINE_MAX_MIN
# define MAX(x,y) ((x) >= (y) ? (x) : (y))
# define MIN(x,y) ((x) <= (y) ? (x) : (y))
#endif
#define PTR_UINT_ABS(x) ((x) < 0 ? -(x) : (x))

/* check if all bits in mask are set in var */
#define TESTALL(mask, var) (((mask) & (var)) == (mask))
/* check if any bit in mask is set in var */
#define TESTANY(mask, var) (((mask) & (var)) != 0)
/* check if a single bit is set in var */
#define TEST TESTANY

#define BOOLS_MATCH(a, b) (((a) && (b)) || (!(a) && !(b)))

/* macros to make conditional compilation look prettier */
#ifdef DEBUG
# define IF_DEBUG(x) x
# define _IF_DEBUG(x) , x
# define IF_DEBUG_ELSE(x, y) x
# define IF_DEBUG_ELSE_0(x) (x)
# define IF_DEBUG_ELSE_NULL(x) (x)
#else
# define IF_DEBUG(x)
# define _IF_DEBUG(x)
# define IF_DEBUG_ELSE(x, y) y
# define IF_DEBUG_ELSE_0(x) 0
# define IF_DEBUG_ELSE_NULL(x) (NULL)
#endif

#ifdef INTERNAL
# define IF_INTERNAL(x) x
# define IF_INTERNAL_ELSE(x,y) x
#else
# define IF_INTERNAL(x)
# define IF_INTERNAL_ELSE(x,y) y
#endif

#ifdef WINDOWS
# define IF_WINDOWS(x) x
# define IF_WINDOWS_(x) x, 
# define _IF_WINDOWS(x) , x
# define IF_WINDOWS_ELSE_0(x) (x)
# define IF_WINDOWS_ELSE(x,y) (x)
# define IF_WINDOWS_ELSE_NP(x,y) x
# define IF_LINUX(x)
# define IF_LINUX_ELSE(x,y) y
# define IF_LINUX_(x)
# define _IF_LINUX(x)
#else
# define IF_WINDOWS(x)
# define IF_WINDOWS_(x)
# define _IF_WINDOWS(x)
# define IF_WINDOWS_ELSE_0(x) (0)
# define IF_WINDOWS_ELSE(x,y) (y)
# define IF_WINDOWS_ELSE_NP(x,y) y
# define IF_LINUX(x) x
# define IF_LINUX_ELSE(x,y) x
# define IF_LINUX_(x) x,
# define _IF_LINUX(x) , x
#endif

#ifdef VMX86_SERVER
# define IF_VMX86(x) x
# define IF_VMX86_ELSE(x,y) x
# define _IF_VMX86(x) , x
# define IF_NOT_VMX86(x) 
#else
# define IF_VMX86(x)
# define IF_VMX86_ELSE(x,y) y
# define _IF_VMX86(x)
# define IF_NOT_VMX86(x) x
#endif

#ifdef LINUX
# ifdef HAVE_TLS
#  define IF_HAVE_TLS_ELSE(x, y) x
#  define IF_NOT_HAVE_TLS(x)
# else
#  define IF_HAVE_TLS_ELSE(x, y) y
#  define IF_NOT_HAVE_TLS(x) x
# endif
#else
/* Windows always has TLS.  Better to generally define HAVE_TLS instead? */
# define IF_HAVE_TLS_ELSE(x, y) x
# define IF_NOT_HAVE_TLS(x)
#endif

#if defined(WINDOWS) && !defined(NOT_DYNAMORIO_CORE)
# define IF_WINDOWS_AND_CORE(x) x
#else
# define IF_WINDOWS_AND_CORE(x)
#endif

#ifdef PROGRAM_SHEPHERDING
# define IF_PROG_SHEP(x) x
#else
# define IF_PROG_SHEP(x)
#endif

#if defined(PROGRAM_SHEPHERDING) && defined(RCT_IND_BRANCH)
# define IF_RCT_IND_BRANCH(x) x
#else
# define IF_RCT_IND_BRANCH(x)
#endif

#if defined(PROGRAM_SHEPHERDING) && defined(RETURN_AFTER_CALL)
# define IF_RETURN_AFTER_CALL(x) x
# define IF_RETURN_AFTER_CALL_ELSE(x, y) x
#else
# define IF_RETURN_AFTER_CALL(x)
# define IF_RETURN_AFTER_CALL_ELSE(x, y) y
#endif

#ifdef HOT_PATCHING_INTERFACE
# define IF_HOTP(x) x
#else
# define IF_HOTP(x)
#endif

#ifdef CLIENT_INTERFACE
# define IF_CLIENT_INTERFACE(x) x
# define IF_CLIENT_INTERFACE_ELSE(x, y) x
# define _IF_CLIENT_INTERFACE(x) , x
# define _IF_NOT_CLIENT_INTERFACE(x)
/* _IF_CLIENT_INTERFACE is too long */
# define _IF_CLIENT(x) , x
#else
# define IF_CLIENT_INTERFACE(x)
# define IF_CLIENT_INTERFACE_ELSE(x, y) y
# define _IF_CLIENT_INTERFACE(x)
# define _IF_NOT_CLIENT_INTERFACE(x) , x
# define _IF_CLIENT(x)
#endif

#ifdef CUSTOM_TRACES
# define IF_CUSTOM_TRACES(x) x
#else
# define IF_CUSTOM_TRACES(x)
#endif

#ifdef DR_APP_EXPORTS
# define IF_APP_EXPORTS(x) x
#else
# define IF_APP_EXPORTS(x)
#endif

#ifdef GBOP
# define IF_GBOP(x) x
#else
# define IF_GBOP(x)
#endif

#ifdef PROCESS_CONTROL
# define IF_PROC_CTL(x) x
#else
# define IF_PROC_CTL(x)
#endif

#ifdef KSTATS
# define IF_KSTATS(x) x
#else
# define IF_KSTATS(x)
#endif

/* DR_API EXPORT TOFILE dr_defines.h */
/* DR_API EXPORT BEGIN */
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
/* DR_API EXPORT END */

typedef enum {
    SYSLOG_INFORMATION = 0x1,
    SYSLOG_WARNING     = 0x2,
    SYSLOG_ERROR       = 0x4,
    SYSLOG_CRITICAL    = 0x8,
    SYSLOG_NONE        = 0x0,
    SYSLOG_ALL         = SYSLOG_INFORMATION | SYSLOG_WARNING | SYSLOG_ERROR | SYSLOG_CRITICAL
} syslog_event_type_t;

#define DYNAMO_OPTION(opt) (ASSERT_OWN_READWRITE_LOCK(IS_OPTION_STRING(opt), \
                            &options_lock), dynamo_options.opt)
/* For use where we don't want ASSERT defines. Currently only used in FATAL_USAGE_ERROR
 * so it can be used in files that require all asserts to be client asserts. */
#define DYNAMO_OPTION_NOT_STRING(opt) (dynamo_options.opt)

#if defined(INTERNAL) || defined(CLIENT_INTERFACE)
#define EXPOSE_INTERNAL_OPTIONS
#endif

#ifdef EXPOSE_INTERNAL_OPTIONS
  /* Use only for experimental non-release options */
  /* Internal option value can be set on the command line only in INTERNAL builds */
  /* We should remove the ASSERT if we convert an internal option into non-internal */
# define INTERNAL_OPTION(opt) ((IS_OPTION_INTERNAL(opt)) ? (DYNAMO_OPTION(opt)) : \
                               (ASSERT_MESSAGE("non-internal option argument "#opt, false), \
                                DYNAMO_OPTION(opt)))
#else
  /* Use only for experimental non-release options, 
     default value is assumed and command line options are ignored */
  /* We could use IS_OPTION_INTERNAL(opt) ? to determine whether an option is defined as INTERNAL in
     optionsx.h and have that be the only place to modify to transition between internal and external options.
     The compiler should be able to eliminate the inappropriate part of the constant expression,
     if the specific option is no longer defined as internal so we don't have to modify the code.
     Still I'd rather have explicit uses of DYNAMO_OPTION or INTERNAL_OPTION for now.
  */
# define INTERNAL_OPTION(opt) DEFAULT_INTERNAL_OPTION_VALUE(opt)
#endif /* EXPOSE_INTERNAL_OPTIONS */

#ifdef LINUX
#ifndef DR_DO_NOT_DEFINE_uint32
 typedef unsigned int uint32;
#endif
/* Note: Linux paths are longer than the 260 limit on Windows, 
   yet we can't afford the 4K of PATH_MAX from <limits.h> */
typedef uint64 timestamp_t;
#  define NAKED
#  define ZHEX32_FORMAT_STRING "%08x"
#  define HEX32_FORMAT_STRING "%x"

#else /* WINDOWS */
/* We no longer include windows.h here, in order to more easily include
 * this file for types like reg_t in hotpatch_interface.h and not force
 * hotpatch compilation to point at windows include directories.
 * By not including it here we must define MAX_PATH and include windows.h
 * for NOT_DYNAMORIO_CORE in options.c and drmarker.c for share/ compilation.
 */
#  define MAX_PATH 260 /* from winbase.h */
#ifndef DR_DO_NOT_DEFINE_uint
 typedef unsigned int uint;
#endif
#ifndef DR_DO_NOT_DEFINE_uint32
 typedef unsigned int uint32;
#endif
/* NOTE : unsigned __int64 not convertible to double */
typedef unsigned __int64 uint64;
typedef __int64 int64;
#  ifdef X64
typedef int64 ssize_t;
#  else
typedef int ssize_t;
#  endif
/* PR 232882: %I32x printf format code not supported on NT's ntdll */
#  ifdef X64
#    define ZHEX32_FORMAT_STRING "%08I32x"
#    define HEX32_FORMAT_STRING "%I32x"
#  else
#    define ZHEX32_FORMAT_STRING "%08x"
#    define HEX32_FORMAT_STRING "%x"
#  endif
/* VC6 doesn't define the standard ULLONG_MAX */
#  if _MSC_VER <= 1200 && !defined(ULLONG_MAX)
#    define ULLONG_MAX _UI64_MAX 
#  endif
typedef uint64 timestamp_t;
#  define NAKED __declspec( naked )
#endif

#define FIXED_TIMESTAMP_FORMAT "%10"INT64_FORMAT"u"

/* DR_API EXPORT TOFILE dr_defines.h */
/* DR_API EXPORT BEGIN */
#define UINT64_FORMAT_CODE INT64_FORMAT"u"
#define INT64_FORMAT_CODE INT64_FORMAT"d"
#define UINT64_FORMAT_STRING "%"UINT64_FORMAT_CODE
#define INT64_FORMAT_STRING "%"INT64_FORMAT_CODE
#define HEX64_FORMAT_STRING "%"INT64_FORMAT"x"
#define ZHEX64_FORMAT_STRING "%016"INT64_FORMAT"x"
#ifdef API_EXPORT_ONLY
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
#endif
/* DR_API EXPORT END */
/* workaround for lack of defines stack and assumption in genapi.pl
 * that ignored defines are outermost */
/* DR_API EXPORT BEGIN */
#ifdef API_EXPORT_ONLY
#define PFX "0x"PFMT
#define PIFX "0x"PIFMT
#endif
/* DR_API EXPORT END */

/* Statistics are 64-bit for x64, 32-bit for x86, so we don't have overflow
 * for pointer-sized stats.
 * TODO PR 227994: use per-statistic types for variable bitwidths.
 */
#ifdef X64
typedef int64 stats_int_t;
#else
typedef int stats_int_t;
#endif

/* For printing pointers: we do not use %p as gcc prepends 0x and uses
 * lowercase while cl does not prepend, puts leading 0's, and uses uppercase.
 * Also, the C standard does not allow min width for %p.
 * Two macros:
 * - PFMT == Pointer Format == with leading zeros
 * - PIFMT == Pointer Integer Format == no leading zeros
 * Convenience macros to shrink long lines:
 * - PFX == Pointer Format with leading 0x
 * - PIFX == Pointer Integer Format with leading 0x
 * For printing memory region sizes:
 * - SZFMT == Size Format
 * - SSZFMT == Signed Size Format
 * For printing 32-bit integers as hex we use %x.  We could use a macro
 * there and then disallow %x, to try and avoid 64-bit printing bugs,
 * but it wouldn't be a panacea.
 */
#define L_UINT64_FORMAT_STRING L"%"L_EXPAND_LEVEL(UINT64_FORMAT_CODE)
#ifdef X64
#  define PFMT ZHEX64_FORMAT_STRING
#  define PIFMT HEX64_FORMAT_STRING
#  define SZFMT UINT64_FORMAT_STRING
#  define SSZFMT INT64_FORMAT_STRING
#  define SZFC UINT64_FORMAT_CODE
#  define SSZFC INT64_FORMAT_CODE
#else
#  define PFMT ZHEX32_FORMAT_STRING
#  define PIFMT HEX32_FORMAT_STRING
#  define SZFMT "%u"
#  define SSZFMT "%d"
#  define SZFC "u"
#  define SSZFC "d"
#endif
#define L_PFMT L"%016"L_EXPAND_LEVEL(INT64_FORMAT)L"x"
#define PFX "0x"PFMT
#define PIFX "0x"PIFMT

/* printf code for {thread,process}_id_t */
#ifdef WINDOWS
# define IDFMT SZFMT
#else
# define IDFMT "%d"
#endif

/* Maximum length of any registry parameter. Note that some are further
 * restricted to MAXIMUM_PATH from their usage. */
#define MAX_REGISTRY_PARAMETER 512
/* Maximum length of option string in the registry */
#define MAX_OPTIONS_STRING     512
/* Maximum length of any individual list option's string */
#define MAX_LIST_OPTION_LENGTH 512
/* Maximum length of the path secified by a path option */
#define MAX_PATH_OPTION_LENGTH MAXIMUM_PATH
/* Maximum length of any individual option. */
#define MAX_OPTION_LENGTH      512

#define MAX_PARAMNAME_LENGTH 64

/* Arbitrary debugging-only module name length maximum */
#define MAX_MODNAME_INTERNAL 64

/* Maximum string representation of a DWORD:
 * 0x80000000 = -2147483648 -> 11 characters + 1 for null termination
 */
#define MAX_DWORD_STRING_LENGTH 12

typedef char pathstring_t[MAX_PATH_OPTION_LENGTH];
/* The liststring_t type is assumed to contain ;-separated values that are
 * appended to if multiple option instances are specified
 */
typedef char liststring_t[MAX_LIST_OPTION_LENGTH];

/* convenience macros for secure string buffer operations */
#define BUFFER_SIZE_BYTES(buf)      sizeof(buf)
#define BUFFER_SIZE_ELEMENTS(buf)   (BUFFER_SIZE_BYTES(buf) / sizeof(buf[0]))
#define BUFFER_LAST_ELEMENT(buf)    buf[BUFFER_SIZE_ELEMENTS(buf) - 1]
#define NULL_TERMINATE_BUFFER(buf)  BUFFER_LAST_ELEMENT(buf) = 0

#define BUFFER_ROOM_LEFT_W(wbuf) (BUFFER_SIZE_ELEMENTS(wbuf) - wcslen(wbuf) - 1)
#define BUFFER_ROOM_LEFT(abuf) (BUFFER_SIZE_ELEMENTS(abuf) - strlen(abuf) - 1)

/* strncat() calls require termination after each to be safe, especially if
 * cating repeatedly.  For example see hotpatch.c:hotp_load_hotp_dlls().
 */
#define CAT_AND_TERMINATE(buf, str) do {        \
      strncat(buf, str, BUFFER_ROOM_LEFT(buf)); \
      NULL_TERMINATE_BUFFER(buf);               \
  } while(0)


/* platform-independent */

#define EXPANDSTR(x) #x
#define STRINGIFY(x) EXPANDSTR(x)

/* Allow custom builds to specify the product name */
#ifdef CUSTOM_PRODUCT_NAME
#  define PRODUCT_NAME STRINGIFY(CUSTOM_PRODUCT_NAME)
#else
#  define PRODUCT_NAME "DynamoRIO"
#endif
/* PR 323321: split eventlog from reg name so we can use longer name in reg.
 * Jim's original comment here about "alphanum only" and memories
 * of issues w/ eventlog Araksha => Determin[a] have made us cautious
 * (though in my tests on win2k the longer name worked fine for eventlog).
 */
/* i#80: I thought about flattening the reg key but simpler to just
 * have two DynamoRIO levels
 */
#define COMPANY_NAME "DynamoRIO" /* used for reg key */
#define COMPANY_NAME_EVENTLOG "DynamoRIO" /* used for event log */
/* used in (c) stmt in log file and in resources */
#define COMPANY_LONG_NAME "DynamoRIO developers"

#ifdef BUILD_NUMBER
#  define BUILD_NUMBER_STRING "build "STRINGIFY(BUILD_NUMBER)
#else
#  define BUILD_NUMBER_STRING "custom build"
#  define BUILD_NUMBER (0)
#endif

#ifdef VERSION_NUMBER
#  define VERSION_NUMBER_STRING "version "STRINGIFY(VERSION_NUMBER)
#else
#  define VERSION_NUMBER_STRING "internal version"
#  define VERSION_NUMBER (0.0)
#endif

#ifdef HOT_PATCHING_INTERFACE
#  define HOT_PATCHING_DLL_CACHE_PATH "\\lib\\hotp\\"
#  define HOTP_MODES_FILENAME "ls-modes.cfg"
#  define HOTP_POLICIES_FILENAME "ls-defs.cfg"
#endif

/* FIXME: the environment vars need to be renamed - it will be a pain */
#define DYNAMORIO_VAR_HOME_ID       DYNAMORIO_HOME
#define DYNAMORIO_VAR_LOGDIR_ID     DYNAMORIO_LOGDIR
#define DYNAMORIO_VAR_OPTIONS_ID    DYNAMORIO_OPTIONS
#define DYNAMORIO_VAR_AUTOINJECT_ID DYNAMORIO_AUTOINJECT
#define DYNAMORIO_VAR_UNSUPPORTED_ID  DYNAMORIO_UNSUPPORTED
#define DYNAMORIO_VAR_RUNUNDER_ID   DYNAMORIO_RUNUNDER
#define DYNAMORIO_VAR_CMDLINE_ID    DYNAMORIO_CMDLINE
#define DYNAMORIO_VAR_ONCRASH_ID    DYNAMORIO_ONCRASH
#define DYNAMORIO_VAR_SAFEMARKER_ID DYNAMORIO_SAFEMARKER 
/* NT only, value should be all CAPS and specifies a boot option to match */

#define DYNAMORIO_VAR_CACHE_ROOT_ID   DYNAMORIO_CACHE_ROOT
/* we have to create our own properly secured directory, that allows
 * only trusted producers to create DLLs, and all publishers to read
 * them.  Note that per-user directories may also be created by the trusted component
 * allowing users to safely use their own private caches.
 */
#define DYNAMORIO_VAR_CACHE_SHARED_ID   DYNAMORIO_CACHE_SHARED
/* a directory giving full write privileges to Everyone.  Therefore
 * none of its contents can be trusted without explicit verification.
 * Expected to be a subdirectory of DYNAMORIO_CACHE_ROOT.
 */

/* Location for persisted caches; FIXME: currently the same as the ASLR sharing dir */
#define DYNAMORIO_VAR_PERSCACHE_ROOT_ID   DYNAMORIO_CACHE_ROOT
/* FIXME case 9651: security model, etc. */
#define DYNAMORIO_VAR_PERSCACHE_SHARED_ID   DYNAMORIO_CACHE_SHARED
/* case 10255: use a suffix to distinguish from ASLR files in same dir
 * DR persisted cache => "dpc"
 */
#define PERSCACHE_FILE_SUFFIX "dpc"

#ifdef HOT_PATCHING_INTERFACE
# define DYNAMORIO_VAR_HOT_PATCH_POLICES_ID  DYNAMORIO_HOT_PATCH_POLICIES
# define DYNAMORIO_VAR_HOT_PATCH_MODES_ID  DYNAMORIO_HOT_PATCH_MODES
#endif
#ifdef PROCESS_CONTROL
# define DYNAMORIO_VAR_APP_PROCESS_WHITELIST_ID  DYNAMORIO_APP_PROCESS_WHITELIST
# define DYNAMORIO_VAR_ANON_PROCESS_WHITELIST_ID  DYNAMORIO_ANON_PROCESS_WHITELIST

# define DYNAMORIO_VAR_APP_PROCESS_BLACKLIST_ID  DYNAMORIO_APP_PROCESS_BLACKLIST
# define DYNAMORIO_VAR_ANON_PROCESS_BLACKLIST_ID  DYNAMORIO_ANON_PROCESS_BLACKLIST
#endif

#define DYNAMORIO_VAR_HOME       STRINGIFY(DYNAMORIO_VAR_HOME_ID)
#define DYNAMORIO_VAR_LOGDIR     STRINGIFY(DYNAMORIO_VAR_LOGDIR_ID)
#define DYNAMORIO_VAR_OPTIONS    STRINGIFY(DYNAMORIO_VAR_OPTIONS_ID)
#define DYNAMORIO_VAR_AUTOINJECT STRINGIFY(DYNAMORIO_VAR_AUTOINJECT_ID)
#define DYNAMORIO_VAR_UNSUPPORTED  STRINGIFY(DYNAMORIO_VAR_UNSUPPORTED_ID)
#define DYNAMORIO_VAR_RUNUNDER   STRINGIFY(DYNAMORIO_VAR_RUNUNDER_ID)
#define DYNAMORIO_VAR_CMDLINE    STRINGIFY(DYNAMORIO_VAR_CMDLINE_ID)
#define DYNAMORIO_VAR_ONCRASH    STRINGIFY(DYNAMORIO_VAR_ONCRASH_ID)
#define DYNAMORIO_VAR_SAFEMARKER STRINGIFY(DYNAMORIO_VAR_SAFEMARKER_ID)
#define DYNAMORIO_VAR_CACHE_ROOT  STRINGIFY(DYNAMORIO_VAR_CACHE_ROOT_ID)
#define DYNAMORIO_VAR_CACHE_SHARED  STRINGIFY(DYNAMORIO_VAR_CACHE_SHARED_ID)
#define DYNAMORIO_VAR_PERSCACHE_ROOT  STRINGIFY(DYNAMORIO_VAR_PERSCACHE_ROOT_ID)
#define DYNAMORIO_VAR_PERSCACHE_SHARED  STRINGIFY(DYNAMORIO_VAR_PERSCACHE_SHARED_ID)
#ifdef HOT_PATCHING_INTERFACE
# define DYNAMORIO_VAR_HOT_PATCH_POLICIES  STRINGIFY(DYNAMORIO_VAR_HOT_PATCH_POLICES_ID)
# define DYNAMORIO_VAR_HOT_PATCH_MODES  STRINGIFY(DYNAMORIO_VAR_HOT_PATCH_MODES_ID)
#endif
#ifdef PROCESS_CONTROL
# define DYNAMORIO_VAR_APP_PROCESS_WHITELIST \
    STRINGIFY(DYNAMORIO_VAR_APP_PROCESS_WHITELIST_ID)
# define DYNAMORIO_VAR_ANON_PROCESS_WHITELIST \
    STRINGIFY(DYNAMORIO_VAR_ANON_PROCESS_WHITELIST_ID)

# define DYNAMORIO_VAR_APP_PROCESS_BLACKLIST \
    STRINGIFY(DYNAMORIO_VAR_APP_PROCESS_BLACKLIST_ID)
# define DYNAMORIO_VAR_ANON_PROCESS_BLACKLIST \
    STRINGIFY(DYNAMORIO_VAR_ANON_PROCESS_BLACKLIST_ID)
#endif

#ifdef LINUX

#  define DYNAMORIO_VAR_EXECVE  "DYNAMORIO_POST_EXECVE"
#  define DYNAMORIO_VAR_EXECVE_LOGDIR  "DYNAMORIO_EXECVE_LOGDIR"
#  define L_IF_WIN(x) x

#else /* WINDOWS */

#  define EXPAND_LEVEL(str) str              /* expand one level */
#  define L_EXPAND_LEVEL(str) L(str)
#  define L(str) L ## str
#  define LCONCAT(wstr,str) EXPAND_LEVEL(wstr)L("\\")L(str)
#  define L_IF_WIN(x) L_EXPAND_LEVEL(x)

/* unicode versions of shared names*/
#  define L_DYNAMORIO_VAR_HOME        L_EXPAND_LEVEL(DYNAMORIO_VAR_HOME)
#  define L_DYNAMORIO_VAR_LOGDIR      L_EXPAND_LEVEL(DYNAMORIO_VAR_LOGDIR)
#  define L_DYNAMORIO_VAR_OPTIONS     L_EXPAND_LEVEL(DYNAMORIO_VAR_OPTIONS)
#  define L_DYNAMORIO_VAR_AUTOINJECT  L_EXPAND_LEVEL(DYNAMORIO_VAR_AUTOINJECT)
#  define L_DYNAMORIO_VAR_UNSUPPORTED L_EXPAND_LEVEL(DYNAMORIO_VAR_UNSUPPORTED)
#  define L_DYNAMORIO_VAR_RUNUNDER    L_EXPAND_LEVEL(DYNAMORIO_VAR_RUNUNDER)
#  define L_DYNAMORIO_VAR_CMDLINE     L_EXPAND_LEVEL(DYNAMORIO_VAR_CMDLINE)
#  define L_DYNAMORIO_VAR_ONCRASH     L_EXPAND_LEVEL(DYNAMORIO_VAR_ONCRASH)
#  define L_DYNAMORIO_VAR_SAFEMARKER  L_EXPAND_LEVEL(DYNAMORIO_VAR_SAFEMARKER)
#  define L_DYNAMORIO_VAR_CACHE_ROOT  L_EXPAND_LEVEL(DYNAMORIO_VAR_CACHE_ROOT)
#  define L_DYNAMORIO_VAR_CACHE_SHARED L_EXPAND_LEVEL(DYNAMORIO_VAR_CACHE_SHARED)
# ifdef HOT_PATCHING_INTERFACE
#  define L_DYNAMORIO_VAR_HOT_PATCH_POLICIES  L_EXPAND_LEVEL(DYNAMORIO_VAR_HOT_PATCH_POLICIES)
#  define L_DYNAMORIO_VAR_HOT_PATCH_MODES  L_EXPAND_LEVEL(DYNAMORIO_VAR_HOT_PATCH_MODES)
# endif
# ifdef PROCESS_CONTROL
#  define L_DYNAMORIO_VAR_APP_PROCESS_WHITELIST \
    L_EXPAND_LEVEL(DYNAMORIO_VAR_APP_PROCESS_WHITELIST)
#  define L_DYNAMORIO_VAR_ANON_PROCESS_WHITELIST \
    L_EXPAND_LEVEL(DYNAMORIO_VAR_ANON_PROCESS_WHITELIST)

#  define L_DYNAMORIO_VAR_APP_PROCESS_BLACKLIST \
    L_EXPAND_LEVEL(DYNAMORIO_VAR_APP_PROCESS_BLACKLIST)
#  define L_DYNAMORIO_VAR_ANON_PROCESS_BLACKLIST \
    L_EXPAND_LEVEL(DYNAMORIO_VAR_ANON_PROCESS_BLACKLIST)
# endif

#  define L_PRODUCT_NAME              L_EXPAND_LEVEL(PRODUCT_NAME)
#  define L_COMPANY_NAME              L_EXPAND_LEVEL(COMPANY_NAME)
#  define L_COMPANY_LONG_NAME         L_EXPAND_LEVEL(COMPANY_LONG_NAME)

/* event log registry keys */
#  define EVENTLOG_HIVE HKEY_LOCAL_MACHINE
#  define EVENTLOG_NAME COMPANY_NAME_EVENTLOG
#  define EVENTSOURCE_NAME PRODUCT_NAME /* should be different than logfile name */

#  define EVENTLOG_REGISTRY_SUBKEY "System\\CurrentControlSet\\Services\\EventLog"
#  define L_EVENTLOG_REGISTRY_SUBKEY L_EXPAND_LEVEL(EVENTLOG_REGISTRY_SUBKEY)
#  define L_EVENTLOG_REGISTRY_KEY L"\\Registry\\Machine\\"L_EXPAND_LEVEL(EVENTLOG_REGISTRY_SUBKEY)
#  define L_EVENT_LOG_KEY LCONCAT(L_EVENTLOG_REGISTRY_KEY,EVENTLOG_NAME)
#  define L_EVENT_LOG_SUBKEY LCONCAT(L_EVENTLOG_REGISTRY_SUBKEY,EVENTLOG_NAME)
#  define L_EVENT_LOG_NAME L_EXPAND_LEVEL(EVENTLOG_NAME)      
#  define L_EVENT_SOURCE_NAME L_EXPAND_LEVEL(EVENTSOURCE_NAME)
#  define L_EVENT_SOURCE_KEY LCONCAT(L_EVENT_LOG_KEY, EVENTSOURCE_NAME)
#  define L_EVENT_SOURCE_SUBKEY LCONCAT(L_EVENT_LOG_SUBKEY, EVENTSOURCE_NAME)

#  define EVENT_LOG_KEY LCONCAT(L_EXPAND_LEVEL(EVENTLOG_REGISTRY_SUBKEY),EVENTLOG_NAME)
#  define EVENT_SOURCE_KEY LCONCAT(EVENT_LOG_KEY,EVENTSOURCE_NAME)
/* Log key values (NOTE the values here are the values our installer uses,
 * not sure what all of them mean).  FIXME would be nice if these were
 * shared with the installer config file. Only used by DRcontrol (via 
 * share/config.c) to set up new eventlogs (mainly for vista where our
 * installer doesn't work yet xref case 8482).*/
#  define L_EVENT_FILE_VALUE_NAME L"File"
#  define L_EVENT_FILE_NAME_PRE_VISTA \
       L"%SystemRoot%\\system32\\config\\"L_EXPAND_LEVEL(EVENTLOG_NAME)L".evt"
#  define L_EVENT_FILE_NAME_VISTA \
       L"%SystemRoot%\\system32\\winevt\\logs\\"L_EXPAND_LEVEL(EVENTLOG_NAME)L".elf"
#  define L_EVENT_MAX_SIZE_NAME L"MaxSize"
#  define EVENT_MAX_SIZE 0x500000
#  define L_EVENT_RETENTION_NAME L"Retention"
#  define EVENT_RETENTION 0
/* Src key values */
#  define L_EVENT_TYPES_SUPPORTED_NAME L"TypesSupported"
#  define EVENT_TYPES_SUPPORTED 0x7 /* info | warning | error */
#  define L_EVENT_CATEGORY_COUNT_NAME L"CategoryCount"
#  define EVENT_CATEGORY_COUNT 0
#  define L_EVENT_CATEGORY_FILE_NAME L"CategoryMessageFile"
#  define L_EVENT_MESSAGE_FILE L"EventMessageFile"

/* shared object directory base */
/* base root in global object namespace, not in BaseNamedObjects or Sessions */
#  define DYNAMORIO_SHARED_OBJECT_BASE L("\\")L_EXPAND_LEVEL(COMPANY_NAME)
/* shared object directory for shared DLL cache */
#  define DYNAMORIO_SHARED_OBJECT_DIRECTORY LCONCAT(DYNAMORIO_SHARED_OBJECT_BASE, "SharedCache")

/* registry */
#  define DYNAMORIO_REGISTRY_BASE_SUBKEY "Software\\"COMPANY_NAME"\\"PRODUCT_NAME
#  define DYNAMORIO_REGISTRY_BASE L"\\Registry\\Machine\\Software\\"L_EXPAND_LEVEL(COMPANY_NAME)L("\\")L_EXPAND_LEVEL(PRODUCT_NAME)
#  define DYNAMORIO_REGISTRY_HIVE HKEY_LOCAL_MACHINE
#  define DYNAMORIO_REGISTRY_KEY    DYNAMORIO_REGISTRY_BASE_SUBKEY
#  define L_DYNAMORIO_REGISTRY_KEY L"Software\\"L_EXPAND_LEVEL(COMPANY_NAME)L"\\"L_EXPAND_LEVEL(PRODUCT_NAME)

#  define INJECT_ALL_HIVE    HKEY_LOCAL_MACHINE
#  define INJECT_ALL_KEY     "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows"
#  define INJECT_ALL_SUBKEY  "AppInit_DLLs"
/* introduced on Vista */
#  define INJECT_ALL_LOAD_SUBKEY        "LoadAppInit_DLLs"
/* introduced on Windows 7/2008 R2 */
#  define INJECT_ALL_SIGN_SUBKEY        "RequireSignedAppInit_DLLs"

#  define INJECT_ALL_HIVE_L    L"\\Registry\\Machine\\"
#  define INJECT_ALL_KEY_L     L_EXPAND_LEVEL(INJECT_ALL_KEY)
#  define INJECT_ALL_SUBKEY_L  L_EXPAND_LEVEL(INJECT_ALL_SUBKEY)
#  define INJECT_ALL_LOAD_SUBKEY_L      L_EXPAND_LEVEL(INJECT_ALL_LOAD_SUBKEY)
#  define INJECT_ALL_SIGN_SUBKEY_L      L_EXPAND_LEVEL(INJECT_ALL_SIGN_SUBKEY)

#  define INJECT_DLL_NAME      "drpreinject.dll"
#  define INJECT_DLL_8_3_NAME  "DRPREI~1.DLL"

#  define INJECT_HELPER_DLL1_NAME      "drearlyhelp1.dll"
#  define INJECT_HELPER_DLL2_NAME      "drearlyhelp2.dll"

#  define DEBUGGER_INJECTION_HIVE       HKEY_LOCAL_MACHINE
#  define DEBUGGER_INJECTION_KEY        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options"
#  define DEBUGGER_INJECTION_VALUE_NAME "Debugger"

#  define DEBUGGER_INJECTION_HIVE_L       L"\\Registry\\Machine\\"
#  define DEBUGGER_INJECTION_KEY_L        L_EXPAND_LEVEL(DEBUGGER_INJECTION_KEY)
#  define DEBUGGER_INJECTION_VALUE_NAME_L L_EXPAND_LEVEL(DEBUGGER_INJECTION_VALUE_NAME)

#  define DRINJECT_NAME                 "drinject.exe"

/* shared module uses SVCHOST_NAME and EXE_SUFFIX separately */
#  define SVCHOST_NAME         "svchost"
#  define EXE_SUFFIX           ".exe"
#  define L_EXE_SUFFIX         L_EXPAND_LEVEL(EXE_SUFFIX)
#  define SVCHOST_EXE_NAME     SVCHOST_NAME EXE_SUFFIX
#  define L_SVCHOST_EXE_NAME   L_EXPAND_LEVEL(SVCHOST_NAME) L_EXPAND_LEVEL(EXE_SUFFIX)

/* for processview, etc */
#  define DYNAMORIO_LIBRARY_NAME "dynamorio.dll"
#  define DLLPATH_RELEASE    "\\lib\\release\\"DYNAMORIO_LIBRARY_NAME
#  define DLLPATH_DEBUG      "\\lib\\debug\\"DYNAMORIO_LIBRARY_NAME
#  define DLLPATH_PROFILE    "\\lib\\profile\\"DYNAMORIO_LIBRARY_NAME

#  define L_DYNAMORIO_LIBRARY_NAME L_EXPAND_LEVEL(DYNAMORIO_LIBRARY_NAME)
#  define L_DLLPATH_RELEASE    L"\\lib\\release\\"L_DYNAMORIO_LIBRARY_NAME
#  define L_DLLPATH_DEBUG      L"\\lib\\debug\\"L_DYNAMORIO_LIBRARY_NAME
#  define L_DLLPATH_PROFILE    L"\\lib\\profile\\"L_DYNAMORIO_LIBRARY_NAME

#  define INJECT_ALL_DLL_SUBPATH   "\\lib\\"INJECT_DLL_8_3_NAME
#  define L_INJECT_ALL_DLL_SUBPATH   L"\\lib\\"L_EXPAND_LEVEL(INJECT_DLL_8_3_NAME)

enum DLL_TYPE {
        DLL_NONE,
        DLL_UNKNOWN,
        DLL_RELEASE,
        DLL_DEBUG,
        DLL_PROFILE,
        DLL_CUSTOM,
        DLL_PATHHAS
};
#endif /* WINDOWS */

/* DYNAMORIO_RUNUNDER controls the injection technique and process naming, 
 *  it is a bitmask of the values below:
 * RUNUNDER_ON: 
 *  take over the app indicated by the corresponding app-specific
 *  subkey; when this is a global param, it only acts as a default for
 *  subkeys which don't explicitly set RUNUNDER. indicates current
 *  default takeover method via the preinjector / AppInit_DLLs
 * RUNUNDER_ALL:
 *  use as a global parameter only, for doing "run all". exclude with
 *  app-specific RUNUNDER_OFF.  You need to use in conjuction with ON
 *
 *
 * RUNUNDER_EXPLICIT:
 *  indicates that the app will use the alternate injection technique
 *  currently via -follow_explicit_children, but might change to drinject 
 *  in the per-executable debugger registry key at some point
 *
 *
 * RUNUNDER_COMMANDLINE_MATCH:
 *  indicates that the process command line must exactly match the
 *  value in the DYNAMORIO_CMDLINE app-specific subkey, or else no
 *  takeover will be done.  Note that only a single instance of 
 *  a given executable name can be controlled this way.
 *
 * RUNUNDER_COMMANDLINE_DISPATCH: 
 *  marks that the processes with this executable name should be
 *  differentiated by their canonicalized commandline.  For example,
 *  different dllhost instances will get their own different subkeys,
 *  just the way the svchost.exe instances have full sets of option
 *  keys.
 *
 * RUNUNDER_COMMANDLINE_NO_STRIP:
 *  this flag is used only in conjunction with
 *  RUNUNDER_COMMANDLINE_DISPATCH.  Our default canonicalization rule
 *  strips the first argument on the commandline for historic reasons.
 *  Since that was the way we had previously hardcoded the names for
 *  say "svchost.exe-netsvcs".  In case we actually want to dispatch
 *  on a single argument - which would be the case for say msiexec.exe
 *  /v then this flag is needed.
 *
 * RUNUNDER_ONCE:
 *  is used by staging mode to specify that the executable corresponding 
 *  to the current process shouldn't run under DR the next time, i.e., 
 *  turn off its RUNUNDER_ON flag after checking it for the current process.
 *  This is needed to prevent perpetual crash-&-boot cycles due to DR failure.  
 *  See case 3702.
 **/
enum {
    /* FIXME: keep in mind that we only read decimal values */
    RUNUNDER_OFF                  = 0x00,   /* 0 */
    RUNUNDER_ON                   = 0x01,   /* 1 */
    RUNUNDER_ALL                  = 0x02,   /* 2 */

    /* Dummy field to keep track of which processes were RUNUNDER_EXPLICIT
     * before we moved to -follow_systemwide by default (for -early_injection)
     * (note this was the old RUNUNDER_EXPLICIT value) */
    RUNUNDER_FORMERLY_EXPLICIT    = 0x04,   /* 4 */

    RUNUNDER_COMMANDLINE_MATCH    = 0x08,   /* 8 */
    RUNUNDER_COMMANDLINE_DISPATCH = 0x10,  /* 16 */
    RUNUNDER_COMMANDLINE_NO_STRIP = 0x20,  /* 32 */
    RUNUNDER_ONCE                 = 0x40,  /* 64 */

    RUNUNDER_EXPLICIT             = 0x80,  /* 128 */
};

/* A bitmask of possible actions to take on a nudge.  Accessed via
 * NUDGE_GENERIC(name) Recommended to always pass -nudge opt so to
 * synchronize options first.  For many state transition nudge's
 * option change will trigger any other actions: e.g. start
 * protecting, simulate attack, etc..  Only pulse-like events that
 * should be acted on only once need separate handling here.  Not all
 * combinations are meaningful, and the order of execution is not
 * determined from the order in the definitions.
 */
/* To use as an iterator define NUDGE_DEF(name, comment) */

/* CAUTION: DO NOT change ordering of the nudge definitions for non-NYI, i.e.,
 *          implemented nudges.  These numbers correspond to specific masks 
 *          that are used by the nodemanager/drcontrol (thus QA).  Will lead to
 *          a lot of unwanted confusion.
 */
#define NUDGE_DEFINITIONS()                                                     \
    /* Control nudges */                                                        \
    NUDGE_DEF(opt, "Synchronize dynamic options")                               \
    NUDGE_DEF(reset, "Reset code caches") /* flush & delete */                  \
    NUDGE_DEF(detach, "Detach")                                                 \
    NUDGE_DEF(mode, "Liveshield mode update")                                   \
    NUDGE_DEF(policy, "Liveshield policy update")                               \
    NUDGE_DEF(lstats, "Liveshield statistics NYI")                              \
    NUDGE_DEF(process_control, "Process control nudge") /* Case 8594. */        \
    NUDGE_DEF(upgrade, "DR upgrade NYI case 4179")                              \
    NUDGE_DEF(kstats, "Dump kstats in log or kstat file NYI")                   \
    /* internal options */                                                      \
    NUDGE_DEF(stats, "Dump internal stats in logfiles NYI")                     \
    NUDGE_DEF(invalidate, "Invalidate code caches NYI") /* flush */             \
    /* stress testing */                                                        \
    NUDGE_DEF(recreate_pc, "Recreate PC NYI")                                   \
    NUDGE_DEF(recreate_state, "Recreate state NYI")                             \
    NUDGE_DEF(reattach, "Reattach - almost detach, NYI")                        \
    /* diagnostics */                                                           \
    NUDGE_DEF(diagnose, "Request diagnostic file NYI")                          \
    NUDGE_DEF(ldmp, "Dump core")                                                \
    NUDGE_DEF(freeze, "Freeze coarse units")                                    \
    NUDGE_DEF(persist, "Persist coarse units")                                  \
    /* client nudge */                                                          \
    NUDGE_DEF(client, "Client nudge")                                           \
    /* security testing */                                                      \
    NUDGE_DEF(violation, "Simulate a security violation")                       \
    /* ADD NEW NUDGE_DEFs only immediately above this line  */                  \
    /* Since these are used as a bitmask only 32 types can be supported:
     * but on Linux only 28.  If we want more we can simply use the client_arg
     * field to multiplex.
     */

typedef enum {
#define NUDGE_DEF(name, comment) NUDGE_DR_##name, 
    NUDGE_DEFINITIONS()
#undef NUDGE_DEF
    NUDGE_DR_PARAMETRIZED_END
} nudge_generic_type_t;

/* note that these are bitmask values */
#define NUDGE_GENERIC(name) (1 << (NUDGE_DR_##name))

/* On Linux only 2 bits for this */
#define NUDGE_ARG_VERSION_1 1
#define NUDGE_ARG_CURRENT_VERSION NUDGE_ARG_VERSION_1

/* nudge_arg_t flags 
 * On Linux only 2 bits for these
 */
enum {
    NUDGE_IS_INTERNAL       = 0x01, /* nudge is internally generated */
#ifdef WINDOWS
    NUDGE_NUDGER_FREE_STACK = 0x02, /* nudger will free the nudge thread's stack so the
                                     * nudge thread itself shouldn't */
    NUDGE_FREE_ARG          = 0x04, /* nudge arg is in a separate allocation and should
                                     * be freed by the nudge thread */
#endif
};

typedef struct {
#ifdef LINUX
    /* We only have room for 16 bytes that we control, 24 bytes total.
     * Note that the kernel does NOT copy the huge amount of padding
     * at the tail end of siginfo_t so we cannot use that.
     */
    int ignored1; /* siginfo_t.si_signo: kernel sets so we cannot use */
    /* These make up the siginfo_t.si_errno field.  Since version starts
     * at 1, this field will never be 0 for a nudge signal, but is always
     * 0 for a libc sigqueue()-generated signal.
     */
    uint nudge_action_mask:28;
    uint version:2;
    uint flags:2;
    int ignored2; /* siginfo_t.si_code: has meaning to kernel so we avoid using */
#else
    uint version; /* version number for future proofing */
    uint nudge_action_mask; /* drawn from NUDGE_DEFS above */
    uint flags; /* flags drawn from above enum */
#endif
    client_id_t client_id; /* unique ID identifying client */
    uint64 client_arg; /* argument for a client nudge */
#ifdef WIN32
    /* Add future arguments for nudge actions here. 
     * There is no room for more Linux arguments.
     */
#endif
} nudge_arg_t;

#ifdef LINUX
/* i#61/PR 211530: Linux nudges.
 * We pick a signal that is very unlikely to be sent asynchronously by
 * the app, and for which we can distinguish synch from asynch by
 * looking at the interrupted pc.
 */
# define NUDGESIG_SIGNUM         SIGILL
#endif


#ifdef HOT_PATCHING_INTERFACE
/* These type definitions define the hot patch interface between the core &
 * the node manager.
 * CAUTION: Any changes to this can break the hot patch interface between the
 *          core and the node manager.
 */

/* All hot patch policy IDs must be of the form XXXX.XXXX; this ID is used to
 * generate the threat ID for a given hot patch violation.
 */
#define HOTP_POLICY_ID_LENGTH 9

/* Define AVOID_API_EXPORT here and undef it after the ifdef-endif using it.
 * This way it will just be used for compling dr code and not for
 * genapi.pl which generates client header files.  In otherwords, this allows
 * having code that isn't visible in the client headers but is visible for dr
 * builds.  This helps sharing types and code between dr and client, but with
 * some hidden extras for dr builds.
 */
#define AVOID_API_EXPORT 1

/* DR_API EXPORT TOFILE dr_probe.h */
/* DR_API EXPORT BEGIN */

/** Describes the status of a probe at any given point.  The status is returned
 * by dr_register_probes() in the dr_probe_desc_t structure for each probe
 * specified.  It can be obtained for individual probes by calling
 * dr_get_probe_status().
 */
typedef enum {
    /* Error codes. */

    /** Exceptions during callback execution and other unknown errors. */
    DR_PROBE_STATUS_ERROR = 1,

    /** An invalid or unknown probe ID was specified with dr_get_probe_status(). */
    DR_PROBE_STATUS_INVALID_ID = 2,

    /* All the invalid states listed below may arise statically (at the
     * time of parsing the probes, i.e., inside dr_register_probes() or
     * dynamically (i.e., when modules are loaded or unloaded)).
     */ 

    /** The numeric virtual address specified for the probe insertion location
     * or the callback function is invalid.
     */
    DR_PROBE_STATUS_INVALID_VADDR = 3,

    /** The pointer to the name of the library containing the probe insertion
     * location or the callback function is invalid or the library containing
     * the callback function can't be loaded.
     */
    DR_PROBE_STATUS_INVALID_LIB = 4,

    /** The library offset for either the probe insertion location or the
     * callback function is invalid; for ex., if the offset is out of bounds.
     */
    DR_PROBE_STATUS_INVALID_LIB_OFFS = 5,

    /** The pointer to the name of the exported function, where the probe is to
     * be inserted or which is the callback function, is invalid or the exported
     * function doesn't exist.
     */
    DR_PROBE_STATUS_INVALID_FUNC = 6,

    /* Codes for pending cases, i.e., valid probes haven't been inserted
     * because certain events haven't transpired.
     */

    /** The numeric virtual address specified for the probe insertion location
     * or the callback function isn't executable.  This may be so at the time
     * of probe registration or latter if the memory protections are changed.
     * An inserted probe might reach this state if the probe insertion point or
     * the callback function is made non-executable after being executable.
     */
    DR_PROBE_STATUS_VADDR_NOT_EXEC = 7,

    /** The library where the probe is to be inserted isn't in the process. */
    DR_PROBE_STATUS_LIB_NOT_SEEN = 8,

    /** Execution hasn't reached the probe insertion point yet.  This is valid
     * for Code Manipulation mode only because in that mode probes are inserted
     * only in the dynamic instruction stream.
     */
    DR_PROBE_STATUS_WAITING_FOR_EXEC = 9,

    /** Either the library where the probe is to be inserted has been unloaded
     * or the library containing the callback function has been unloaded.
     */
    DR_PROBE_STATUS_LIB_UNLOADED = 10,

    /* Miscellaneous status codes. */

    /** Probe was successfully inserted. */
    DR_PROBE_STATUS_INJECTED = 11,

    /** One or more aspects of the probe aren't supported as of now. */
    DR_PROBE_STATUS_UNSUPPORTED = 12,

# ifdef AVOID_API_EXPORT
    /* DON'T CHANGE THE VALUES OF THE DR_* CONSTANTS DEFINED ABOVE.  They are
     * exported to clients, whereas constants in this ifdef aren't.  Any change
     * to those values will likely break old clients with newer versions of DR
     * (backward compatibility).  New status codes should go after
     * DR_PROBE_STATUS_UNSUPPORTED.
     */
    /* Note: constants are numbered to prevent the compiler from resetting the
     * sequence based on the symbolic assignments below.  HOTP_INJECT_DETECT
     * ended up getting the same number as one the values above!  Ditto with
     * HOTP_INJECT_OFF.  Though these duplications only broke the tools build
     * they can cause subtle runtime errors, so forcing numbers.
     */

    /* The constants below are used only for LiveShields.  */

    /* Defines the current injection status of a policy, i.e., was it injected
     * or not, why and why not?  Today we don't distinguish the reasons for
     * error cases, i.e., all of them are rolled into one.
     *
     * Constants list from most important status to least, from a reporting
     * point of view; don't change this arbitrarily.
     *
     * CAUTION: Any changes to this will break drview, so they must be kept in
     * sync.
     */

    /* Deactivation, exception, error, etc. */
    HOTP_INJECT_ERROR = DR_PROBE_STATUS_ERROR,

    /* Completely injected in protect mode. */
    HOTP_INJECT_PROTECT = DR_PROBE_STATUS_INJECTED,

    /* Completely injected in detect mode.  Not applicable to probes as they
     * don't have detectors.  Restart numbering at 100 to give enough room for
     * future probe status constants.
     */
    HOTP_INJECT_DETECT = 100,

    /* One or more patch points in a vulnerability have been patched, but not 
     * all, yet.  N/A to probes as they can't group multiple patch points.
     */
    HOTP_INJECT_IN_PROGRESS = 101,

    /* Vulnerability was matched and is ready for injection, but no patch point
     * has been seen yet.
     */
    HOTP_INJECT_PENDING = DR_PROBE_STATUS_WAITING_FOR_EXEC,

    /* Vulnerability hasn't been matched yet.  May mean that it is not yet
     * vulnerable (because all dlls haven't been loaded) or just not vulnerable
     * at all;  there is no way to distinguish between the two.
     */
    HOTP_INJECT_NO_MATCH = DR_PROBE_STATUS_LIB_NOT_SEEN,

    /*
    TODO: must distinguish between no match & vulnerable vs. no match & not 
          vulnerable; future work if needed.
    HOTP_INJECT_NO_MATCH_VULNERABLE,
    HOTP_INJECT_NO_MATCH_NOT_VULNERABLE,
    */

    HOTP_INJECT_OFF = 102   /* Policy has been turned off, so no injection. */
# endif /* AVOID_API_EXPORT */
} dr_probe_status_t;
/* DR_API EXPORT END */
#undef AVOID_API_EXPORT

typedef dr_probe_status_t hotp_inject_status_t;


/* Modes are at a policy level, not a vulnerability level, even though the 
 * core organizes things at the vulnerability level.
 */
typedef enum {
    HOTP_MODE_OFF = 0,
    HOTP_MODE_DETECT  = 1,
    HOTP_MODE_PROTECT = 2
} hotp_policy_mode_t;

/* This structure is used to form a table that contains the status of all
 * active policies.  This is separated out into a separate table, as opposed
 * to being part of the hotp_vul_info_t and thus part of the global 
 * vulnerability table, because the node manager will be directly reading 
 * this information from the core's memory.  Thus, this structure serves 
 * as a container to expose only that data which will be needed by the node 
 * manager from the core regarding hot patches.
 */
typedef struct {
    /* polciy_id is the same as the one in hotp_vul_t.  Duplicated because can't
     * have this pointing to the hopt_vul_t structure because the node manager 
     * will have to chase the pointer for each element to read it rather than a
     * single block of memory.
     */
    char policy_id[HOTP_POLICY_ID_LENGTH + 1];
    hotp_inject_status_t inject_status;

    /* This is the same as the one in hotp_vul_t.  Duplicated for the same
     * reason policy_id (see above) was.  Can't have the hotp_vul_t structure
     * pointing here too because that struct/table needs to be initialized for 
     * the policy status table to be created; catch-22.  
     * Fix for case 5484, where the node manager wasn't able to tell if an 
     * inject status was for a policy that was turned on or off.
     */
    hotp_policy_mode_t mode;
} hotp_policy_status_t;

/* Node manager should read either this struct first or the first two elements
 * in it.  Then it should read the rest.  Note: 'size' refers to the size of
 * both this struct and the size of the policy_status_array in bytes.
 */
typedef struct {
    /* This is the crc for the whole table, except itself, i.e., the crc
     * computation starts from 'size'.  This is needed otherwise writing the
     * crc value itself will change the crc of the table!  Can get past it
     * by doing crc twice, but it is needlessly expensive.
     *
     * CAUTION: don't not move crc and size around as it can cause both the
     * node manager & the core to break.
     */
    uint crc;
    uint size; /* Size of this struct plus the table. */
    uint num_policies;
    hotp_policy_status_t *policy_status_array;
} hotp_policy_status_table_t;

#endif /* ifdef HOT_PATCHING_INTERFACE */

/* These constants & macros are used by core, share and preinject, so this is
 * the only place they will build for win32 and linux! */
/* return codes for [gs]et_parameter style functions
 * failure == 0 for compatibility with get_parameter()
 * if GET_PARAMETER_NOAPPSPECIFIC is returned, that means the
 *  parameter returned is from the global options, because there
 *  was no app-specific key present.
 * Note: constants shouldn't be added to this enum without checking whether
 *       the macros below will work; they should if errors are all negative and
 *       valid codes are all positive.
 */
enum {
    GET_PARAMETER_BUF_TOO_SMALL = -1,
    GET_PARAMETER_FAILURE = 0,
    GET_PARAMETER_SUCCESS = 1,
    GET_PARAMETER_NOAPPSPECIFIC = 2,
    SET_PARAMETER_FAILURE = GET_PARAMETER_FAILURE,
    SET_PARAMETER_SUCCESS = GET_PARAMETER_SUCCESS
};
#define IS_GET_PARAMETER_FAILURE(x) ((x) <= 0 )
#define IS_GET_PARAMETER_SUCCESS(x) ((x) > 0 )

/* X86 only but no ifdef since hotp builds don't set that define */
/* User-mode machine context.
 * We have it here instead of in arch_exports.h so that we
 * can expose it to hotpatch_interface.h.
 *
 * If any field offsets are changed, or fields are added, update
 * the following:
 *   - OFFSET defines in x86/arch.h and their uses in fcache_{enter,return}
 *   - DR_MCONTEXT_SIZE and PUSH_DR_MCONTEXT in x86/x86.asm
 *   - clean call layout of dr_mcontext_t on the stack in x86/mangle.c
 *   - interception layout of dr_mcontext_t on the stack in win32/callback.c
 *   - context_to_mcontext (and vice versa) in win32/ntdll.c
 *   - dump_mcontext in x86/arch.c
 *   - inject_into_thread in win32/inject.c
 * Also, hotp_context_t exposes the dr_mcontext_t struct to hot patches,
 * so be careful when changing any field offsets.
 *
 * FIXME: remove eax-ebx-ecx-edx and use the local_state_t
 * version from within DR as well as from the ibl (case 3701).
 *
 * PR 264138: for xmm fields, we do NOT specify 16-byte alignment for
 * our own, to avoid the 8-byte padding at the end for 32-bit mode
 * that we don't want to manually insert for our hand-rolled structs
 * on the stack.
 * It's ok for the client version to have padding as dr_get_mcontext copies,
 * but we don't currently assume alignment of client-declared dr_mcontext_t,
 * so we have no alignment declarations there at the moment either.
 */

/* PR 306394: for 32-bit xmm0-7 are caller-saved, yet we don't expect
 * them to be touched by DR, client, kernel, or libraries DR/client
 * call: so for now we just leave space for future use.
 * In particular for LOL64 we should be careful about kernel use.
 */
#ifdef WINDOWS
# define NUM_XMM_SAVED 6 /* xmm0-5; for 32-bit we have space for xmm0-7 */
#else
# ifdef X64
#  define NUM_XMM_SAVED 16 /* xmm0-15 */
# else
#  define NUM_XMM_SAVED 0 /* we have space for xmm0-7, but not saved right now */
# endif
#endif

/* DR_API EXPORT TOFILE dr_defines.h */
/* DR_API EXPORT BEGIN */
/** 128-bit XMM register. */
typedef union _dr_xmm_t {
#ifdef AVOID_API_EXPORT
    /* We avoid having 8-byte-aligned fields here for 32-bit: they cause
     * cl to add padding in app_state_at_intercept_t and unprotected_context_t,
     * which messes up our interception stack layout and our x86.asm offsets.
     * We don't access these very often, so we just omit this field.
     */
#endif
#ifdef API_EXPORT_ONLY
    uint64 u64[2]; /**< Representation as 2 64-bit integers. */
#endif
    uint   u32[4]; /**< Representation as 4 32-bit integers. */
    byte   u8[16]; /**< Representation as 8 8-bit integers. */
    reg_t  reg[IF_X64_ELSE(2,4)]; /**< Representation as 2 or 4 registers. */
} dr_xmm_t;

#ifdef AVOID_API_EXPORT
/* If this is increased, you'll probably need to increase the size of
 * inject_into_thread's buf and INTERCEPTION_CODE_SIZE (for Windows).
 * Also, update NUM_XMM_SLOTS in x86.asm and get_xmm_caller_saved.
 */
#endif
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
#ifdef AVOID_API_EXPORT
    /* FIXME: have special comment syntax instead of bogus ifdef to
     * get genapi to strip out internal-only comments? */
    /* Our inlined ibl uses eax-edx, so we place them together to fit
     * on the same 32-byte cache line; yet we also want to simplify
     * things by keeping this in pusha order.  Whether on a 32-bit or
     * 64-bit machine, or a 32-byte or 64-byte cache line, they will
     * still be on the same line, assuming this struct is
     * cache-line-aligned (which it is if in dcontext).
     * Any changes in order here must be mirrored in x86/x86.asm offsets.
     */
#endif
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
#ifdef AVOID_API_EXPORT
    /* PR 264138: we must preserve xmm0-5 if on a 64-bit Windows kernel,
     * and xmm0-15 if in a 64-bit Linux app (PR 302107).  (Note that
     * mmx0-7 are also caller-saved on linux but we assume they're not
     * going to be used by DR, libc, or client routines: overlap w/
     * floating point).  For Windows we assume that none of our routines
     * (or libc routines that we call, except the floating-point ones,
     * where we explicitly save state) clobber beyond xmm0-5.  Rather than
     * have a separate WOW64 build, we have them in the struct but ignored
     * for normal 32-bit.
     * PR 306394: for now we leave space for preserving xmm0-7 for 32-bit,
     * but we do not currently save them.
     */
#endif
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
/* DR_API EXPORT END */

#endif /* ifndef _GLOBALS_SHARED_H_ */
