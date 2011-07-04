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

/* Copyright (c) 2003-2007 Determina Corp. */
/* Copyright (c) 2002-2003 Massachusetts Institute of Technology */
/* Copyright (c) 2002 Hewlett-Packard Company */

/*
 * injector.c - standalone injector for win32
 */

/* Disable warnings from vc8 include files */
/* buildtools\VC\8.0\dist\VC\PlatformSDK\Include\prsht.h(201)
 * "nonstandard extension used : nameless struct/union"
 */
#pragma warning( disable : 4201 )
/* buildtools\VC\8.0\dist\VC\PlatformSDK\Include\wintrust.h(459)
 * "named type definition in parentheses"
 */
#pragma warning( disable : 4115 )

#include "../globals.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>             /* memset */
#include <time.h>
#include <tchar.h>

#include "globals_shared.h"
#include "ntdll.h"
#include "inject_shared.h"
#include "os_private.h"

#ifdef UNICODE
# error dr_inject.h and drdeploy.c not set up for unicde
#endif

#define VERBOSE 0
#if VERBOSE
# define DO_VERBOSE(x) x
# undef printf
# define VERBOSE_PRINT(x) printf x
#else
# define DO_VERBOSE(x) 
# define VERBOSE_PRINT(x)
#endif
#define FP stderr

/* FIXME: case 64 would like ^C to kill child process, it doesn't.
 * also, child process seems able to read stdin but not to write
 * to stdout or stderr (in fact it dies if it tries)
 * I think Ctrl-C is delivered only if you have a debugger(windbg) attached.
 */
#define HANDLE_CONTROL_C 0

/* global vars */
static int limit; /* in seconds */
static BOOL showmem;
static BOOL showstats;
static BOOL inject;
static BOOL force_injection;
static BOOL use_environment = TRUE; /* FIXME : for now default to using
                                     * the environment, below we check and
                                     * never use the environment if using
                                     * debug injection.  Revisit.
                                     */
static const char *ops_param;
static double wallclock; /* in seconds */

/* FIXME : assert stuff, internal error, display_message duplicated from 
 * pre_inject, share? */

/* for asserts, copied from utils.h */
#ifdef assert
# undef assert
#endif
/* avoid mistake of lower-case assert */
#define assert assert_no_good_use_ASSERT_instead
void internal_error(char *file, int line, char *msg);
#undef ASSERT
#ifdef DEBUG
void display_error(char *msg);
# ifdef INTERNAL
#   define ASSERT(x)         if (!(x)) internal_error(__FILE__, __LINE__, #x)
# else 
#   define ASSERT(x)         if (!(x)) internal_error(__FILE__, __LINE__, "")
# endif /* INTERNAL */
#else
# define display_error(msg) ((void) 0)
# define ASSERT(x)         ((void) 0)
#endif /* DEBUG */

/* in ntdll.c */
extern char *get_application_name(void);
extern char *get_application_pid(void);

static void
display_error_helper(wchar_t *msg) 
{
    wchar_t title_buf[MAX_PATH + 64];
    _snwprintf(title_buf, BUFFER_SIZE_ELEMENTS(title_buf), 
              L_PRODUCT_NAME L" Notice: %hs(%hs)", 
              get_application_name(), get_application_pid());
    NULL_TERMINATE_BUFFER(title_buf);

    /* for unit tests: assume that if a limit is set, we are in a
     *  script so it's ok to just display to stderr. avoids hangs when
     *  an error is encountered. */
    if (limit <= 0)
        nt_messagebox(msg, title_buf);
    else {
        fprintf(FP, "\n\n%ls\n%ls\n\n", title_buf, msg);
        fflush(FP);
    }
}

void
internal_error(char *file, int line, char *expr)
{
#ifdef INTERNAL
# define FILENAME_LENGTH L""
#else 
/* truncate file name to first character */
# define FILENAME_LENGTH L".1"   
#endif
    wchar_t buf[512];
    _snwprintf(buf, BUFFER_SIZE_ELEMENTS(buf), 
              L"Injector Error %" FILENAME_LENGTH L"hs:%d %hs\n", 
              file, line, expr);
    NULL_TERMINATE_BUFFER(buf);
    display_error_helper(buf);
    TerminateProcess(GetCurrentProcess(), (uint)-1);
}

#ifdef DEBUG
void 
display_error(char *msg)
{
# ifdef DISABLED /* going with msgbox always! */
    fprintf(FP, msg);
# else
    wchar_t buf[512];
    _snwprintf(buf, BUFFER_SIZE_ELEMENTS(buf), L"%hs", msg);
    NULL_TERMINATE_BUFFER(buf);
    display_error_helper(buf);
# endif
}
#endif /* DEBUG */

#if HANDLE_CONTROL_C
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType   //  control signal type
                           )
{
    printf("Inside HandlerRoutine!\n");
    fflush(stdout);
    /*    GenerateConsoleCtrlEvent(dwCtrlType, phandle);*/
    return TRUE;
}
#endif 

/*************************************************************************/

/* Opaque type to users, holds our state */
typedef struct _dr_inject_info_t {
    PROCESS_INFORMATION pi;
    bool using_debugger_injection;
    char image_name[MAXIMUM_PATH];
} dr_inject_info_t;

DYNAMORIO_EXPORT
char *
dr_inject_get_image_name(void *data)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    if (data == NULL)
        return NULL;
    return info->image_name;
}

DYNAMORIO_EXPORT
HANDLE
dr_inject_get_process_handle(void *data)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    if (data == NULL)
        return INVALID_HANDLE_VALUE;
    return info->pi.hProcess;
}

DYNAMORIO_EXPORT
process_id_t
dr_inject_get_process_id(void *data)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    if (data == NULL)
        return 0;
    return info->pi.dwProcessId;
}

DYNAMORIO_EXPORT
bool
dr_inject_using_debug_key(void *data)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    if (data == NULL)
        return false;
    return info->using_debugger_injection;
}

DYNAMORIO_EXPORT
void
dr_inject_print_stats(void *data, int elapsed_secs, bool showstats, bool showmem)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    VM_COUNTERS mem;
    /* not in DR library - floating point use is OK */
    int secs = elapsed_secs;

    if (data == NULL)
        return;

    if (!get_process_mem_stats(info->pi.hProcess, &mem)) {
        /* failed */
        memset(&mem, 0, sizeof(VM_COUNTERS));
    }

    if (showstats) {
        int cpu = get_process_load(info->pi.hProcess);
        /* Elapsed real (wall clock) time.  */
        if (secs >= 3600) {     /* One hour -> h:m:s.  */
            fprintf(FP, "%ld:%02ld:%02ldelapsed ",
                    secs / 3600,
                    (secs % 3600) / 60,
                    secs % 60);
        } else {
            fprintf(FP, "%ld:%02ld.%02ldelapsed ",      /* -> m:s.  */
                    secs / 60,
                    secs % 60,
                    0 /* for now */);
        }
        fprintf(FP, "%d%%CPU \n", cpu);
        fprintf(FP, "(%lu tot, %lu RSS, %lu paged, %lu non, %lu swap)k\n",
                mem.PeakVirtualSize/1024,
                mem.PeakWorkingSetSize/1024,
                mem.QuotaPeakPagedPoolUsage/1024,
                mem.QuotaPeakNonPagedPoolUsage/1024,
                mem.PeakPagefileUsage/1024);
    }
    if (showmem) {
        fprintf(FP, "Process Memory Statistics:\n");
        fprintf(FP, "\tPeak virtual size:         %6d KB\n",
               mem.PeakVirtualSize/1024);
        fprintf(FP, "\tPeak working set size:     %6d KB\n",
               mem.PeakWorkingSetSize/1024);
        fprintf(FP, "\tPeak paged pool usage:     %6d KB\n",
               mem.QuotaPeakPagedPoolUsage/1024);
        fprintf(FP, "\tPeak non-paged pool usage: %6d KB\n",
               mem.QuotaPeakNonPagedPoolUsage/1024);
        fprintf(FP, "\tPeak pagefile usage:       %6d KB\n",
               mem.PeakPagefileUsage/1024);
    }
}

/*************************************************************************/
/* Following code handles the copying of environment variables to the 
 * registry (the -env option, default on) and unsetting them later
 *
 * FIXME : race conditions with someone else modifying this registry key,
 *         doesn't restore registry if -no_wait
 * NOTE : doesn't propagate if using debug injection method (by design)
 *         
 */

static BOOL created_product_reg_key;
static BOOL created_image_reg_key;
static HKEY image_name_key;
static HKEY product_name_key;

/* use macros to avoid duplication */
/* This check is necessary since we don't have
 * preprocessor warnings enabled on windows */
#if defined(TEMP_CMD) || defined(DO_CMD)
#error TEMP_CMD or DO_ENV_VARS macro already declared!
#endif
/* if need to add or remove environment variables looked for, do it here */
#define DO_ENV_VARS()                             \
 TEMP_CMD(options, OPTIONS);                      \
 TEMP_CMD(logdir, LOGDIR);                        \
 TEMP_CMD(unsupported, UNSUPPORTED);              \
 TEMP_CMD(rununder, RUNUNDER);                    \
 TEMP_CMD(autoinject, AUTOINJECT);                \
 TEMP_CMD(cache_root, CACHE_ROOT);                \
 TEMP_CMD(cache_shared, CACHE_SHARED)

#define TEMP_CMD(name, NAME)                               \
 static BOOL overwrote_##name##_value;                     \
 static BOOL created_##name##_value;                       \
 static TCHAR old_##name##_value[MAX_REGISTRY_PARAMETER]

/* Note - leading spacs is to avoid make ugly rule on zero argument function
 * declerations not using void - xref PR 215100 */
 DO_ENV_VARS();

static void
set_registry_from_env(const TCHAR *image_name, const TCHAR *dll_path) 
{
#undef TEMP_CMD
#define TEMP_CMD(name, NAME)                     \
 BOOL do_##name;                                 \
 TCHAR name##_value[MAX_REGISTRY_PARAMETER]

    DO_ENV_VARS();

    DWORD disp, size, type;
    int res, len;
    int rununder_int_value;

    /* get environment variable values if they are set */
#undef TEMP_CMD    
#define TEMP_CMD(name, NAME)                                                  \
 name##_value[0] = '\0';  /* to be pedantic */                                \
 len = GetEnvironmentVariable(_TEXT(DYNAMORIO_VAR_##NAME), name##_value,      \
                              BUFFER_SIZE_ELEMENTS(name##_value));            \
 do_##name = (use_environment &&                                              \
              (len > 0 ||                                                     \
              (len == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)));       \
 ASSERT(len < BUFFER_SIZE_ELEMENTS(name##_value));                            \
 VERBOSE_PRINT(("Environment var %s for %s, value = %s\n",                    \
        do_##name ? "set" : "not set", #name, name##_value));

    DO_ENV_VARS();

    if (ops_param != NULL) {
        /* -ops overrides env var */
        strncpy(options_value, ops_param, BUFFER_SIZE_ELEMENTS(options_value));
        NULL_TERMINATE_BUFFER(options_value);
        do_options = TRUE;
    }

    /* we always want to set the rununder to make sure RUNUNDER_ON is on
     * to support following children; we set RUNUNDER_EXPLICIT to allow
     * injecting even when preinject is configured. */
    /* FIXME: we read only decimal */
    rununder_int_value = _ttoi(rununder_value);
    rununder_int_value |= RUNUNDER_ON | RUNUNDER_EXPLICIT;

    do_rununder = true;
    _itot(rununder_int_value, rununder_value, 
          10 /* FIXME : is the radix abstracted somewhere */);

    /* for follow_children, we set DYNAMORIO_AUTOINJECT (unless
     * overridden by env var: then child will use env value, while
     * parent uses cmdline path) */
    if (!do_autoinject && dll_path != NULL) {
        _tcsncpy(autoinject_value, dll_path, BUFFER_SIZE_ELEMENTS(autoinject_value));
        do_autoinject = true;
    }

    /* FIXME : doesn't support svchost-* yet */
    ASSERT(_tcsicmp(_TEXT(SVCHOST_EXE_NAME), image_name));
    res = RegCreateKeyEx(DYNAMORIO_REGISTRY_HIVE, 
                         _TEXT(DYNAMORIO_REGISTRY_KEY), 0,  NULL, 
                         REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL,
                         &product_name_key, &disp);
    ASSERT(res == ERROR_SUCCESS);
    if (disp == REG_CREATED_NEW_KEY) {
        created_product_reg_key = TRUE;
    }
    res = RegCreateKeyEx(product_name_key, image_name, 0, NULL, 
                         REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE|KEY_SET_VALUE, 
                         NULL, &image_name_key, &disp);
    ASSERT(res == ERROR_SUCCESS);
    if (disp == REG_CREATED_NEW_KEY) {
        created_image_reg_key = TRUE;
    }
    
    DO_VERBOSE({
        printf("created product key? %s\ncreated image key? %s\n",
               created_product_reg_key ? "yes" : "no", 
               created_image_reg_key ? "yes" : "no");
        fflush(stdout);
    });
 
    /* Now set values */
#undef TEMP_CMD
#define TEMP_CMD(name, NAME)                                                  \
 if (do_##name) {                                                             \
    size = BUFFER_SIZE_BYTES(old_##name##_value);                             \
    res = RegQueryValueEx(image_name_key, _TEXT(DYNAMORIO_VAR_##NAME),        \
                          NULL, &type, (BYTE *) old_##name##_value, &size);   \
    ASSERT(size <= BUFFER_SIZE_BYTES(old_##name##_value));                    \
    if (res == ERROR_SUCCESS) {                                               \
        overwrote_##name##_value = TRUE;                                      \
        ASSERT(type == REG_SZ);                                               \
    } else {                                                                  \
        created_##name##_value = TRUE;                                        \
    }                                                                         \
    res = RegSetValueEx(image_name_key, _TEXT(DYNAMORIO_VAR_##NAME),          \
                        0, REG_SZ, (BYTE *) name##_value,                     \
                        (DWORD) (_tcslen(name##_value)+1)*sizeof(TCHAR));     \
    ASSERT(res == ERROR_SUCCESS);                                             \
    VERBOSE_PRINT(("%s %s registry value with \"%s\" replacing \"%s\"\n",     \
           overwrote_##name##_value  ? "overwrote" : "created", #name,        \
           name##_value, overwrote_##name##_value ? old_##name##_value : ""));\
 }
    
    DO_ENV_VARS();

    DO_VERBOSE({fflush(stdout);});
}

static void
unset_registry_from_env(const TCHAR *image_name)
{
    int res;

    VERBOSE_PRINT(("Restoring registry configuration\n"));

    /* restore registry values */
#undef TEMP_CMD
#define TEMP_CMD(name, NAME)                                                 \
 if (overwrote_##name##_value) {                                             \
     res = RegSetValueEx(image_name_key, _TEXT(DYNAMORIO_VAR_##NAME), 0,     \
                         REG_SZ /* FIXME : abstracted somewhere? */,         \
                         (BYTE *) old_##name##_value,                        \
                         (DWORD)(_tcslen(old_##name##_value)+1) * sizeof(TCHAR));\
     ASSERT(res == ERROR_SUCCESS);                                           \
     VERBOSE_PRINT(("Restored %s value to %s\n", #name, old_##name##_value));\
 } else if (created_##name##_value) {                                        \
     res = RegDeleteValue(image_name_key, _TEXT(DYNAMORIO_VAR_##NAME));      \
     ASSERT(res == ERROR_SUCCESS);                                           \
     VERBOSE_PRINT(("Deleted %s value\n", #name));                           \
 }

    DO_ENV_VARS();

    /* delete keys if created them */
    if (created_image_reg_key) {
        res = RegDeleteKey(product_name_key, image_name);
        ASSERT(res == ERROR_SUCCESS);
        VERBOSE_PRINT(("deleted image reg key\n"));
    }
    if (created_product_reg_key) {
        res = RegDeleteKey(DYNAMORIO_REGISTRY_HIVE, 
                           _TEXT(DYNAMORIO_REGISTRY_KEY));
        ASSERT(res == ERROR_SUCCESS);
        VERBOSE_PRINT(("deleted product reg key\n"));
    }
    DO_VERBOSE({fflush(stdout);});
}

#undef DO_ENV_VARS
#undef TEMP_CMD

/************************** end environment->registry **********************/

/***************************************************************************/
/* The following code handles checking for, setting and unsetting of the
 * debug key injection method
 *
 * This whole section can go away once we have our own version of create 
 * process that doesn't check the debugger key FIXME
 */

static HKEY debugger_key;
static TCHAR debugger_key_full_name[MAX_PATH];
static TCHAR debugger_key_value[3*MAX_PATH];
static DWORD debugger_key_value_size = BUFFER_SIZE_BYTES(debugger_key_value);
static BOOL (*debug_stop_function)(int); 

DYNAMORIO_EXPORT
BOOL
using_debugger_key_injection(const TCHAR *image_name)
{
    int res;

    debug_stop_function = (BOOL (*)(int))
        (GetProcAddress(GetModuleHandle(TEXT("Kernel32")), 
                        "DebugActiveProcessStop"));
    
    _sntprintf(debugger_key_full_name, 
               BUFFER_SIZE_ELEMENTS(debugger_key_full_name), _TEXT("%hs\\%s"),
               DEBUGGER_INJECTION_KEY, image_name);
    NULL_TERMINATE_BUFFER(debugger_key_full_name);

    VERBOSE_PRINT(("debugger key %s\n", debugger_key_full_name));

    res = RegOpenKeyEx(DEBUGGER_INJECTION_HIVE, debugger_key_full_name, 
                       0, KEY_QUERY_VALUE + KEY_SET_VALUE, &debugger_key);
    if (ERROR_SUCCESS != res) 
        return FALSE;
    res = RegQueryValueEx(debugger_key, _TEXT(DEBUGGER_INJECTION_VALUE_NAME),
                          NULL, NULL, (BYTE *) debugger_key_value, 
                          &debugger_key_value_size);
    if (ERROR_SUCCESS != res || 
        /* FIXME : it would be better to check if our commandline matched 
         * what was in the registry value, instead of looking for drinject */
        _tcsstr(debugger_key_value, _TEXT(DRINJECT_NAME)) == 0) {
        RegCloseKey(debugger_key);
        return FALSE;
    }

    /* since returning TRUE, we don't close the debugger_key (it will be 
     * needed by the unset and restore functions). The restore function will
     * close it */

    return TRUE;
}

static
void unset_debugger_key_injection()
{
    if (debug_stop_function == NULL) {
        int res = RegDeleteValue(debugger_key, 
                                 _TEXT(DEBUGGER_INJECTION_VALUE_NAME));
        VERBOSE_PRINT(("Successfully deleted debugger registry value? %s\n", 
                       (ERROR_SUCCESS == res) ? "yes" : "no"));
        if (ERROR_SUCCESS != res) {
            ASSERT(FALSE);
            /* prevent infinite recursion, die now */
            abort();
        }
    }
}

static
void restore_debugger_key_injection(int id, BOOL started)
{
    int res;
    if (debug_stop_function == NULL) {
        res = RegSetValueEx(debugger_key, 
                            _TEXT(DEBUGGER_INJECTION_VALUE_NAME), 0, REG_SZ, 
                            (BYTE *) debugger_key_value, debugger_key_value_size);
        VERBOSE_PRINT(("Successfully restored debugger registry value? %s\n", 
                       (ERROR_SUCCESS == res) ? "yes" : "no"));
    } else {
        if (started) {
            res = (*debug_stop_function)(id);
            VERBOSE_PRINT(("Successfully detached from debugging process? %s\n",
                           res ? "yes" : "no"));
        }
    }
    /* close the global debugger key */
    RegCloseKey(debugger_key);
}

/***************************** end debug key injection ********************/

static const TCHAR *
get_image_name(const TCHAR *app_name)
{
    const TCHAR *name_start = double_tcsrchr(app_name, _TEXT('\\'), _TEXT('/'));
    if (name_start == NULL) 
        name_start = app_name;
    else
        name_start++;
    return name_start;
}


/* Returns 0 on success.
 * On failure, returns a Windows API error code.
 */
DYNAMORIO_EXPORT
int
dr_inject_process_create(const char *app_name, const char *app_cmdline,
                         void **data OUT)
{
    dr_inject_info_t *info = HeapAlloc(GetProcessHeap(), 0, sizeof(*info));
    STARTUPINFO si;
    int errcode = 0;
    bool res;
    if (data == NULL)
        return ERROR_INVALID_PARAMETER;
    
    /* Launch the application process. */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    /* My old drinject code set dwFlags to STARTF_USESTDHANDLES and
     * used GetStartupInfo to get values for hStd{Output,Error} but that
     * ends up not working: perhaps that was before I had bInheritHandles
     * set to true?  Xref PR 208715, i#261, i#142.
     */

    strncpy(info->image_name, get_image_name(app_name),
            BUFFER_SIZE_ELEMENTS(info->image_name));

    /* FIXME, won't need to check this, or unset/restore debugger_key_injection
     * if we have our own version of CreateProcess that doesn't check the 
     * debugger key */
    info->using_debugger_injection = using_debugger_key_injection(info->image_name);

    if (info->using_debugger_injection) {
        unset_debugger_key_injection();
    }

    /* Must specify TRUE for bInheritHandles so child inherits stdin! */
    res = CreateProcess(app_name, (char *)app_cmdline, NULL, NULL, TRUE,
                        CREATE_SUSPENDED |
                        ((debug_stop_function && info->using_debugger_injection) ?
                         DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS : 0),
                        NULL, NULL, &si, &info->pi);
    if (!res)
        errcode = GetLastError();

    if (info->using_debugger_injection) {
        restore_debugger_key_injection(info->pi.dwProcessId, res);
    }

    *data = (void *) info;
    return errcode;
}

DYNAMORIO_EXPORT
bool
dr_inject_process_inject(void *data, bool force_injection,
                         const char *library_path)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    CONTEXT cxt;
    bool inject = true;
    char library_path_buf[MAXIMUM_PATH];

    /* force_injection prevents overriding of inject based on registry */
    if (!force_injection) {
        int inject_flags = systemwide_should_inject(info->pi.hProcess, NULL);
        bool syswide_will_inject = systemwide_inject_enabled() && 
            TEST(INJECT_TRUE, inject_flags) && 
            !TEST(INJECT_EXPLICIT, inject_flags);
        bool should_not_takeover = TEST(INJECT_EXCLUDED, inject_flags) && 
            info->using_debugger_injection;
        /* case 10794: to support follow_children we inject even if
         * syswide_will_inject.  we use RUNUNDER_EXPLICIT to avoid
         * user32 injection from happening, to get consistent injection.
         * (if we didn't things would work but we'd have 
         * a warning "<Blocking load of module dynamorio.dll>" on the 2nd
         * injection)
         */
        inject = !should_not_takeover;
        if (!inject) {
            /* we should always be injecting (we set the registry above) 
             * unless we are using debugger_key_injection, in which 
             * case we use what is in the registry (whoever wrote the registry 
             * should take care of late or nonexistent user32 loading in the 
             * rununder value) */
            ASSERT(info->using_debugger_injection);
            display_error("application is excluded from injection\n");
        } else {
            /* Don't inject if in safe mode */
            if (is_safe_mode()) {
                inject = false;
                display_error("System is in safe mode, not injecting\n");
            }
        }
    }

    if (!inject)
        return false;

    if (library_path == NULL) {
        int err;
        err = get_process_parameter(info->pi.hProcess,
                                    PARAM_STR(DYNAMORIO_VAR_AUTOINJECT),
                                    library_path_buf, sizeof(library_path_buf));
        if (err != GET_PARAMETER_SUCCESS && err != GET_PARAMETER_NOAPPSPECIFIC) {
            inject = false;
            display_error("WARNING: this application is not configured to run under DynamoRIO!\n"
                          "Use drconfig.exe or drrun.exe to configure.");
        }
        NULL_TERMINATE_BUFFER(library_path_buf);
        library_path = library_path_buf;
    }

#ifdef PARAMS_IN_REGISTRY
    /* don't set registry from environment if using debug key */
    if (!info->using_debugger_injection) {
        set_registry_from_env(info->image_name, library_path);
    }
#endif

    inject_init();
    /* FIXME PR 211367: use early_inject instead of this late injection!
     * but non-trivial to gather the relevant addresses: so wait for
     * earliest injection => i#234/PR 204587 prereq?
     */
    if (!inject_into_thread(info->pi.hProcess, &cxt, info->pi.hThread,
                            (char *)library_path)) {
        close_handle(info->pi.hProcess);
        TerminateProcess(info->pi.hProcess, 0);
        return false;
    }
    return true;
}

DYNAMORIO_EXPORT
bool
dr_inject_process_run(void *data)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    /* resume the suspended app process so its main thread can run */
    ResumeThread(info->pi.hThread);
    close_handle(info->pi.hThread);

    return true;
}

DYNAMORIO_EXPORT
int
dr_inject_process_exit(void *data, bool terminate)
{
    dr_inject_info_t *info = (dr_inject_info_t *) data;
    int exitcode = -1;
#ifdef PARAMS_IN_REGISTRY
    if (!info->using_debugger_injection) {
        unset_registry_from_env(info->image_name);
    }
#endif
    if (terminate) {
        TerminateProcess(info->pi.hProcess, 0);
    }
    GetExitCodeProcess(info->pi.hProcess, (LPDWORD) &exitcode);
    close_handle(info->pi.hProcess);
    HeapFree(GetProcessHeap(), 0, info);
    return exitcode;
}

