/* **********************************************************
 * Copyright (c) 2008 VMware, Inc.  All rights reserved.
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

/*
 * module_shared.c
 * Windows DLL routines that are shared between the core, preinject, and
 * drmarker-using code like libutil.
 * It's a pain to link module.c with non-core targets like preinject,
 * so we split these routines out here.
 * Note that NOT_DYNAMORIO_CORE_PROPER still links ntdll.c, while
 * NOT_DYNAMORIO_CORE (i.e., libutil) does not (since it is a pain
 * to link both ntdll.lib and a libc.lib).
 */

#include "configure.h"
#if defined(NOT_DYNAMORIO_CORE)
# define ASSERT(x)
# define ASSERT_NOT_REACHED()
# define ASSERT_NOT_IMPLEMENTED(x)
# define DODEBUG(x)
# define DECLARE_NEVERPROT_VAR(var, ...) var = __VA_ARGS__;
# define ALIGN_BACKWARD(x, alignment) (((ULONG_PTR)x) & (~((alignment)-1)))
# define PAGE_SIZE 4096
#else
/* we include globals.h mainly for ASSERT, even though we're
 * used by preinject.
 * preinject just defines its own internal_error!
 */
# include "../globals.h"
# if !defined(NOT_DYNAMORIO_CORE_PROPER)
#  include "os_private.h" /* for is_readable_pe_base() */
#  include "../module_shared.h" /* for is_in_code_section() */
# endif
#endif

#include "ntdll.h"

/* We have to hack away things we use here that won't work for non-core */
#if defined(NOT_DYNAMORIO_CORE_PROPER) || defined(NOT_DYNAMORIO_CORE)
# undef strcasecmp
# define strcasecmp _stricmp
# define wcscasecmp _wcsicmp
# undef TRY_EXCEPT_ALLOW_NO_DCONTEXT
# define TRY_EXCEPT_ALLOW_NO_DCONTEXT(dc, try, except) try
# undef ASSERT_OWN_NO_LOCKS
# define ASSERT_OWN_NO_LOCKS() /* who cares if not the core */
# undef ASSERT_CURIOSITY
# define ASSERT_CURIOSITY(x) /* who cares if not the core */
/* since not including os_shared.h (this is impl in ntdll.c): */
bool is_readable_without_exception(const byte *pc, size_t size);
/* allow converting functions to and from data pointers */
# pragma warning(disable : 4055)
# pragma warning(disable : 4054)
# define convert_data_to_function(func) ((generic_func_t) (func))
#endif

/* This routine was moved here from os.c since we need it for 
 * get_proc_address_64 (via get_module_exports_directory_*()) for preinject
 * and drmarker, neither of which link os.c.
 */
/* is_readable_without_exception checks to see that all bytes with addresses
 * from pc to pc+size-1 are readable and that reading from there won't
 * generate an exception.  this is a stronger check than
 * !not_readable() below.
 * FIXME : beware of multi-thread races, just because this returns true, 
 * doesn't mean another thread can't make the region unreadable between the 
 * check here and the actual read later.  See safe_read() as an alt.
 */
/* throw-away buffer */
DECLARE_NEVERPROT_VAR(static char is_readable_buf[4/*efficient read*/], {0});
bool 
is_readable_without_exception(const byte *pc, size_t size)
{
    /* Case 7967: NtReadVirtualMemory is significantly faster than
     * NtQueryVirtualMemory (probably even for large regions where NtQuery can
     * walk by mbi.RegionSize but we have to walk by page size).  We don't care
     * if multiple threads write into the buffer at once.  Nearly all of our
     * calls ask about areas smaller than a page.
     */
    byte *check_pc = (byte *) ALIGN_BACKWARD(pc, PAGE_SIZE);
    if (size > (size_t)((byte *)POINTER_MAX - pc))
        size = (byte *)POINTER_MAX - pc;
    do {
        size_t bytes_read = 0;
#if defined(NOT_DYNAMORIO_CORE)
        if (!ReadProcessMemory(NT_CURRENT_PROCESS, check_pc, is_readable_buf,
                               sizeof(is_readable_buf), (SIZE_T*) &bytes_read) ||
            bytes_read != sizeof(is_readable_buf))
#else
        if (!nt_read_virtual_memory(NT_CURRENT_PROCESS, check_pc, is_readable_buf,
                                    sizeof(is_readable_buf), &bytes_read) ||
            bytes_read != sizeof(is_readable_buf))
#endif
            return false;
        check_pc += PAGE_SIZE;
    } while (check_pc != 0/*overflow*/ && check_pc < pc+size);
    return true;
}

/* returns NULL if exports directory doesn't exist
 * if exports_size != NULL returns also exports section size
 * assumes base_addr is a safe is_readable_pe_base() 
 *
 * NOTE - only verifies readability of the IMAGE_EXPORT_DIRECTORY, does not verify target
 * readability of any RVAs it contains (for that use get_module_exports_directory_check
 * or verify in the caller at usage). Xref case 9717.
 */
static
IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory_common(app_pc base_addr,
                                    size_t *exports_size /* OPTIONAL OUT */
                                    _IF_NOT_X64(bool ldr64))
{
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *) base_addr;
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *) (base_addr + dos->e_lfanew);
    IMAGE_DATA_DIRECTORY *expdir;
    ASSERT(dos->e_magic == IMAGE_DOS_SIGNATURE);
    ASSERT(nt != NULL && nt->Signature == IMAGE_NT_SIGNATURE);
#ifndef X64
    if (ldr64) {
        expdir = ((IMAGE_OPTIONAL_HEADER64 *)&(nt->OptionalHeader))->DataDirectory +
            IMAGE_DIRECTORY_ENTRY_EXPORT;
    } else
#endif
        expdir = OPT_HDR(nt, DataDirectory) + IMAGE_DIRECTORY_ENTRY_EXPORT;

    /* avoid link issues: we don't have is_readable_pe_base */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    /* callers should have done this in release builds */
    ASSERT(is_readable_pe_base(base_addr));
#endif

    /* RVA conversions are trivial only for MEM_IMAGE */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    DODEBUG({
        MEMORY_BASIC_INFORMATION mbi;
        ASSERT(query_virtual_memory(base_addr, &mbi, sizeof(mbi)) == sizeof(mbi));
        /* We do see MEM_MAPPED PE files: case 7947 */
        if (mbi.Type != MEM_IMAGE) {
            LOG(THREAD_GET, LOG_SYMBOLS, 1, 
                "get_module_exports_directory(base_addr="PFX"): !MEM_IMAGE\n",
                base_addr);
            ASSERT_CURIOSITY(expdir == NULL || expdir->Size == 0);
        }
    });
#endif

    /* libutil doesn't support vararg macros so can't undef-out */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    LOG(THREAD_GET, LOG_SYMBOLS, 5, 
        "get_module_exports_directory(base_addr="PFX", expdir="PFX")\n",
        base_addr, expdir);
#endif

    if (expdir != NULL) {
        ULONG size = expdir->Size;
        ULONG exports_vaddr = expdir->VirtualAddress;

        /* libutil doesn't support vararg macros so can't undef-out */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
        LOG(THREAD_GET, LOG_SYMBOLS, 5, 
            "get_module_exports_directory(base_addr="PFX") expdir="PFX
            " size=%d exports_vaddr=%d\n", 
            base_addr, expdir, size, exports_vaddr);
#endif

        /* not all DLLs have exports - e.g. drpreinject.dll, or
         * shdoclc.dll in notepad help */
        if (size > 0) {
            IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)
                (base_addr + exports_vaddr);
            ASSERT_CURIOSITY(size >= sizeof(IMAGE_EXPORT_DIRECTORY));
            if (is_readable_without_exception((app_pc)exports,
                                              sizeof(IMAGE_EXPORT_DIRECTORY))) {
                if (exports_size != NULL)
                    *exports_size = size;
                ASSERT_CURIOSITY(exports->Characteristics == 0);
                return exports;
            } else {
                ASSERT_CURIOSITY(false && "bad exports directory, partial map?" ||
                                 EXEMPT_TEST("win32.partial_map.exe"));
            }
        }
    } else 
        ASSERT_CURIOSITY(false && "no exports directory");

    return NULL;
}

/* Same as get_module_exports_directory except also verifies that the functions (and,
 * if check_names, ordinals and fnames) arrays are readable. NOTE - does not verify that
 * the RVA names pointed to by fnames are themselves readable strings. */
static
IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory_check_common(app_pc base_addr,
                                          size_t *exports_size /*OPTIONAL OUT*/,
                                          bool check_names
                                          _IF_NOT_X64(bool ldr64))
{
    IMAGE_EXPORT_DIRECTORY *exports = get_module_exports_directory_common
        (base_addr, exports_size _IF_NOT_X64(ldr64));
    if (exports != NULL) {
        PULONG functions =  (PULONG) (base_addr + exports->AddressOfFunctions);
        PUSHORT ordinals = (PUSHORT) (base_addr + exports->AddressOfNameOrdinals);
        PULONG fnames    =  (PULONG) (base_addr + exports->AddressOfNames);
        if (exports->NumberOfFunctions > 0) {
            if (!is_readable_without_exception((byte *)functions,
                                               exports->NumberOfFunctions *
                                               sizeof(*functions))) {
                ASSERT_CURIOSITY(false &&  "ill-formed exports directory, unreadable "
                                 "functions array, partial map?" ||
                                 EXEMPT_TEST("win32.partial_map.exe"));
                return NULL;
            }
        }
        if (exports->NumberOfNames > 0 && check_names) {
            ASSERT_CURIOSITY(exports->NumberOfFunctions > 0 &&
                             "ill-formed exports directory");
            if (!is_readable_without_exception((byte *)ordinals, exports->NumberOfNames *
                                               sizeof(*ordinals)) ||
                !is_readable_without_exception((byte *)fnames, exports->NumberOfNames *
                                               sizeof(*fnames))) {
                ASSERT_CURIOSITY(false &&  "ill-formed exports directory, unreadable "
                                 "ordinal or names array, partial map?" ||
                                 EXEMPT_TEST("win32.partial_map.exe"));
                return NULL;
            }
        }
    }
    return exports;
}

/* FIXME - this walk is similar to that used in several other module.c
 * functions, we should look into sharing. Also, like almost all of the
 * module.c routines this could be racy with app memory deallocations.
 * FIXME - we could also have a version that lookups by ordinal. We could
 * also allow wildcards and, if desired, extend it to return multiple
 * matching addresses.
 */
/* Interface is similar to msdn GetProcAddress, takes in a module_handle_t
 * (this is just the allocation base of the module) and a name and returns
 * the address of the export with said name.  Returns NULL on failure.
 * NOTE - will return NULL for forwarded exports, exports pointing outside of
 * the module and for exports not in a code section (FIXME - is this the
 * behavior we want?). Name is case insensitive.
 */ 
static generic_func_t
get_proc_address_common(module_handle_t lib, const char *name _IF_NOT_X64(bool ldr64),
                        const char **forwarder OUT)
{
    app_pc module_base;
    size_t exports_size;
    uint i;
    IMAGE_EXPORT_DIRECTORY *exports;
    PULONG functions; /* array of RVAs */
    PUSHORT ordinals;
    PULONG fnames; /* array of RVAs */

    /* avoid non-core issues: we don't have get_allocation_size or dcontexts */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    size_t module_size;
    dcontext_t *dcontext = get_thread_private_dcontext();
#endif

    ASSERT(name != NULL && *name != '\0'); /* verify valid args */
    if (lib == NULL || name == NULL || name == '\0')
        return NULL;

    /* avoid non-core issues: we don't have get_allocation_size or is_readable_pe_base */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    /* FIXME - get_allocation_size and is_readable_pe_base are expensive
     * operations, we could put the onus on the caller to only pass in a
     * valid module_handle_t/pe_base and just assert instead if performance of
     * this routine becomes a concern, esp. since the caller has likely
     * already done it. */
    module_size = get_allocation_size(lib, &module_base);
    if (!is_readable_pe_base(module_base))
        return NULL;
#else
    module_base = (app_pc) lib;
#endif

    exports = get_module_exports_directory_check_common
        (module_base, &exports_size, true _IF_NOT_X64(ldr64));
    if (exports == NULL || exports_size == 0 || exports->NumberOfNames == 0 ||
        /* just extra sanity check */ exports->AddressOfNames == 0)
        return NULL;

    /* avoid non-core issues: we don't have module_size */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
    /* sanity check */
    ASSERT(exports->AddressOfFunctions < module_size &&
           exports->AddressOfFunctions > 0 &&
           exports->AddressOfNameOrdinals < module_size &&
           exports->AddressOfNameOrdinals > 0 &&
           exports->AddressOfNames < module_size &&
           exports->AddressOfNames > 0);
#endif

    functions = (PULONG)(module_base + exports->AddressOfFunctions);
    ordinals = (PUSHORT)(module_base + exports->AddressOfNameOrdinals);
    fnames = (PULONG)(module_base + exports->AddressOfNames);
    
    /* FIXME - linear walk, if this routine becomes performance critical we
     * we should use a binary search. */
    for (i = 0; i < exports->NumberOfNames; i++) {
        char *export_name = (char *)(module_base + fnames[i]);
        bool match = false;
        ASSERT_CURIOSITY((app_pc)export_name > module_base &&  /* sanity check */
                         (app_pc)export_name < module_base + module_size ||
                         EXEMPT_TEST("win32.partial_map.exe"));
        /* FIXME - xref case 9717, we haven't verified that export_name string is
         * safely readable (might not be the case for improperly formed or partially
         * mapped module) and the try will only protect us if we have a thread_private
         * dcontext. Could use is_string_readable_without_exception(), but that may be
         * too much of a perf hit for the no private dcontext case. */
        TRY_EXCEPT_ALLOW_NO_DCONTEXT(dcontext, {
            match = (strcasecmp(name, export_name) == 0);
        }, {
            ASSERT_CURIOSITY_ONCE(false && "Exception during get_proc_address" ||
                                  EXEMPT_TEST("win32.partial_map.exe"));
        });
        if (match) {
            /* we have a match */
            uint ord = ordinals[i];
            app_pc func;
            /* note - function array is indexed by ordinal */
            if (ord >= exports->NumberOfFunctions) {
                ASSERT_CURIOSITY(false && "invalid ordinal index");
                return NULL;
            }
            func = (app_pc)(module_base + functions[ord]);
            if (func == module_base) {
                /* entries can be 0 when no code/data is exported for that
                 * ordinal */
                ASSERT_CURIOSITY(false &&
                                 "get_proc_addr of name with empty export");
                return NULL;
            }
            /* avoid non-core issues: we don't have module_size */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
            if (func < module_base || func >= module_base + module_size) {
                /* FIXME - export isn't in the module, should we still return
                 * it?  Will shimeng.dll or the like ever do this to replace
                 * a function?  For now we return NULL. Xref case 9717, can also
                 * happen for a partial map in which case NULL is the right thing
                 * to return. */
                ASSERT_CURIOSITY(false && "get_proc_addr export location "
                                 "outside of module bounds" ||
                                  EXEMPT_TEST("win32.partial_map.exe"));
                return NULL;
            }
#endif
            if (func >= (app_pc)exports &&
                func < (app_pc)exports + exports_size) {
                /* FIXME - is forwarded function, should we still return it
                 * or return the target? Check - what does GetProcAddress do?
                 * For now we return NULL. Looking up the target would require
                 * a get_module_handle call which might not be safe here. 
                 * With current and planned usage we shouldn' be looking these
                 * up anyways. */
                if (forwarder != NULL) {
                    /* func should point at something like "NTDLL.strlen" */
                    *forwarder = (const char *) func;
                    return NULL;
                } else {
                    ASSERT_NOT_IMPLEMENTED(false &&
                                           "get_proc_addr export is forwarded");
                    return NULL;
                }
            }
            /* avoid non-core issues: we don't have is_in_code_section */
#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
# ifndef CLIENT_INTERFACE
            /* CLIENT_INTERFACE uses a data export for versioning (PR 250952) */
            /* FIXME - this check is also somewhat costly. */
            if (!is_in_code_section(module_base, func, NULL, NULL)) {
                /* FIXME - export isn't in a code section. Prob. a data
                 * export?  For now we return NULL as all current users are
                 * going to call or hook the returned value. */
                ASSERT_CURIOSITY(false &&
                                 "get_proc_addr export not in code section");
                return NULL;
            }
# endif
#endif
            /* get around warnings converting app_pc to generic_func_t */
            return convert_data_to_function(func);
        }
    }

    /* export name wasn't found */
    return NULL;
}

IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory(app_pc base_addr, size_t *exports_size /* OPTIONAL OUT */)
{
    return get_module_exports_directory_common
        (base_addr, exports_size _IF_NOT_X64(false));
}

IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory_check(app_pc base_addr,
                                   size_t *exports_size /*OPTIONAL OUT*/,
                                   bool check_names)
{
    return get_module_exports_directory_check_common
        (base_addr, exports_size, check_names _IF_NOT_X64(false));
}

generic_func_t
get_proc_address(module_handle_t lib, const char *name)
{
    return get_proc_address_common(lib, name _IF_NOT_X64(false), NULL);
}

#ifndef NOT_DYNAMORIO_CORE /* else need get_own_peb() alternate */

/* could be linked w/ non-core but only used by loader.c so far */
generic_func_t
get_proc_address_ex(module_handle_t lib, const char *name, const char **forwarder OUT)
{
    return get_proc_address_common(lib, name _IF_NOT_X64(false), forwarder);
}

/* returns NULL if no loader module is found
 * N.B.: walking loader data structures at random times is dangerous! See
 * get_ldr_module_by_pc in module.c for code to grab the ldr lock (which is
 * also unsafe).  Here we presume that we already own the ldr lock and that
 * the ldr list is consistent, which should be the case for preinject (the only
 * user).  FIXME stick this in module.c with get_ldr_module_by_pc, would need
 * to get module.c compiled into preinjector which is a significant hassle.
 */
LDR_MODULE *
get_ldr_module_by_name(wchar_t *name)
{
    PEB *peb = get_own_peb();
    PEB_LDR_DATA *ldr = peb->LoaderData;
    LIST_ENTRY *e, *mark;
    LDR_MODULE *mod;
    uint traversed = 0;     /* a simple infinite loop break out */

    /* Now, you'd think these would actually be in memory order, but they
     * don't seem to be for me!
     */
    mark = &ldr->InMemoryOrderModuleList;

    for (e = mark->Flink; e != mark; e = e->Flink) {
        mod = (LDR_MODULE *) ((char *)e -
                              offsetof(LDR_MODULE, InMemoryOrderModuleList));
        /* NOTE - for comparison we could use pe_name or mod->BaseDllName.
         * Our current usage is just to get user32.dll for which BaseDllName
         * is prob. better (can't rename user32, and a random dll could have
         * user32.dll as a pe_name).  If wanted to be extra certain could
         * check FullDllName for %systemroot%/system32/user32.dll as that
         * should ensure uniqueness. 
         */
        ASSERT(mod->BaseDllName.Length <= mod->BaseDllName.MaximumLength &&
               mod->BaseDllName.Buffer != NULL);
        if (wcscasecmp(name, mod->BaseDllName.Buffer) == 0) {
            return mod;
        }

        if (traversed++ > MAX_MODULE_LIST_INFINITE_LOOP_THRESHOLD) {
            /* Only caller (preinject) should hold the ldr lock and the ldr
             * state should be consistent so we don't expect to get stuck. */
            ASSERT_NOT_REACHED();
            /* TODO: In case we ever hit this we may want to retry the
             * traversal once more */
            return NULL;
        }
    }
    return NULL;
}
#endif /* !NOT_DYNAMORIO_CORE */

/****************************************************************************/
#ifndef X64

/* PR 271719: Access x64 loader data from WOW64.
 * We duplicate a bunch of data structures and code here, but this is cleaner
 * than compiling the original code as x64 and hacking the build process to
 * get it linked: stick as char[] inside code section or something.
 * We assume that all addresses are in the low 4GB (above 4GB is off-limits
 * for a WOW64 process), so we can ignore the high dword of each pointer.
 */

typedef struct ALIGN_VAR(8) _LIST_ENTRY_64 {
    struct _LIST_ENTRY_64 *Flink;
    uint Flink_hi;
    struct _LIST_ENTRY_64 *Blink;
    uint Blink_hi;
} LIST_ENTRY_64;

/* UNICODE_STRING_64 is in ntdll.h */

/* module information filled by the loader */
typedef struct ALIGN_VAR(8) _PEB_LDR_DATA_64 {  
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    uint  SsHandle_hi;
    LIST_ENTRY_64 InLoadOrderModuleList;
    LIST_ENTRY_64 InMemoryOrderModuleList;
    LIST_ENTRY_64 InInitializationOrderModuleList;
} PEB_LDR_DATA_64;

/* Note that these lists are walked through corresponding LIST_ENTRY pointers
 * i.e., for InInit*Order*, Flink points 16 bytes into the LDR_MODULE structure
 */
typedef struct ALIGN_VAR(8) _LDR_MODULE_64 {
    LIST_ENTRY_64 InLoadOrderModuleList;
    LIST_ENTRY_64 InMemoryOrderModuleList;
    LIST_ENTRY_64 InInitializationOrderModuleList;
    PVOID BaseAddress;
    uint BaseAddress_hi;
    PVOID EntryPoint;
    uint EntryPoint_hi;
    ULONG SizeOfImage;
    int padding;
    UNICODE_STRING_64 FullDllName;
    UNICODE_STRING_64 BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY_64 HashTableEntry; /* see notes for LDR_MODULE */
    ULONG TimeDateStamp;
} LDR_MODULE_64;

static PEB_LDR_DATA_64 *
get_ldr_data_64(void)
{
    /* __readgsqword is not supported for 32-bit */
    byte *peb64;
    __asm {
        mov eax, dword ptr gs:X64_PEB_TIB_OFFSET
        mov peb64, eax
    };
    return *(PEB_LDR_DATA_64 **)(peb64 + X64_LDR_PEB_OFFSET);
}

/* returns NULL if no loader module is found
 * N.B.: walking loader data structures at random times is dangerous! See
 * get_ldr_module_by_pc in module.c for code to grab the ldr lock (which is
 * also unsafe).  Here we presume that we already own the ldr lock and that
 * the ldr list is consistent, which should be the case for preinject (the only
 * user).  FIXME stick this in module.c with get_ldr_module_by_pc, would need
 * to get module.c compiled into preinjector which is a significant hassle.
 */
HANDLE
get_module_handle_64(wchar_t *name)
{
    PEB_LDR_DATA_64 *ldr = get_ldr_data_64();
    LIST_ENTRY_64 *e, *mark;
    LDR_MODULE_64 *mod;
    uint traversed = 0;     /* a simple infinite loop break out */

    /* Now, you'd think these would actually be in memory order, but they
     * don't seem to be for me!
     */
    mark = &ldr->InMemoryOrderModuleList;

    for (e = mark->Flink; e != mark; e = e->Flink) {
        mod = (LDR_MODULE_64 *) ((char *)e -
                              offsetof(LDR_MODULE_64, InMemoryOrderModuleList));
        /* NOTE - for comparison we could use pe_name or mod->BaseDllName.
         * Our current usage is just to get user32.dll for which BaseDllName
         * is prob. better (can't rename user32, and a random dll could have
         * user32.dll as a pe_name).  If wanted to be extra certain could
         * check FullDllName for %systemroot%/system32/user32.dll as that
         * should ensure uniqueness. 
         */
        ASSERT(mod->BaseDllName.Length <= mod->BaseDllName.MaximumLength &&
               mod->BaseDllName.Buffer != NULL);
        if (wcscasecmp(name, mod->BaseDllName.Buffer) == 0) {
            return (HANDLE) mod->BaseAddress;
        }

        if (traversed++ > MAX_MODULE_LIST_INFINITE_LOOP_THRESHOLD) {
            /* Only caller (preinject) should hold the ldr lock and the ldr
             * state should be consistent so we don't expect to get stuck. */
            ASSERT_NOT_REACHED();
            /* TODO: In case we ever hit this we may want to retry the
             * traversal once more */
            return NULL;
        }
    }
    return NULL;
}

/* we return void* since that's easier for preinject and drmarker to deal with */
void *
get_proc_address_64(HANDLE lib, const char *name)
{
    return (void *) get_proc_address_common(lib, name _IF_NOT_X64(true), NULL);
}

#endif /* !X64 */
/****************************************************************************/

