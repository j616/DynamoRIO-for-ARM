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
 * ntdll.h
 * Routines for calling Windows system calls via the ntdll.dll wrappers.
 * We return a boolean (but type int) instead of NTSTATUS, for most cases.
 * 1 indicates success, 0 indicates an error.
 * 
 * New routines however should return the raw NTSTATUS and leave to
 * the callers to report or act on some specific failure.  Should use
 * NT_SUCCESS to verify success, luckily here 0 indicates success, so
 * misuse as a bool will be caught easily.
 */

#ifndef _NTDLL_H_
#define _NTDLL_H_ 1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stddef.h> /* for offsetof */

#include "globals_shared.h" /* for reg_t */

/* Current method is to statically link with ntdll.lib obtained from the DDK */
/* We cannot call get_module_handle at arbitrary points.
 * Some syscalls are at certain points in win32 API internal state
 * such that doing so causes problems.
 */
/* We used to dynamically get the proc address but that led to some untimely
 *  loader lock acquisitions by kernel32.GetProcAddress  (Bug 411).  
 *  This should serve as an example why using kernel32 functions is not safe.
*/

/* A simple wrapper to define ntdll entry points used inside our functions.
   Since there is no official header file exporting these, 
   we encapsulate signatures obtained from other sources.
 */
#define GET_NTDLL(NtFunction, signature)                             \
  NTSYSAPI NTSTATUS NTAPI NtFunction signature

/***************************************************************************
 * structs and defines
 * mostly from either Windows NT/2000 Native API Reference's ntdll.h
 * or from the ddk's header files
 */

typedef unsigned int uint;
typedef LONG NTSTATUS;
/* make sure to cast to signed in case passed reg_t */
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

#define NT_CURRENT_PROCESS ( (HANDLE) PTR_UINT_MINUS_1 )
#define NT_CURRENT_THREAD  ( (HANDLE) (ptr_uint_t)-2 )

typedef struct _UNICODE_STRING {
    /* Length field is size in bytes not counting final 0 */
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

#ifndef X64
typedef struct ALIGN_VAR(8) _UNICODE_STRING_64 {
    /* Length field is size in bytes not counting final 0 */
    USHORT Length;
    USHORT MaximumLength;
    int padding;
    PWSTR  Buffer;
    uint   Buffer_hi;
} UNICODE_STRING_64;
#endif

typedef struct _STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PCHAR  Buffer;
} ANSI_STRING;
typedef ANSI_STRING *PANSI_STRING;
typedef ANSI_STRING OEM_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

/* from DDK2003SP1/3790.1830/inc/wnet/ntdef.h */
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L /* N.B.: this is an invalid parameter on NT4! */
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L /* N.B.: introduced with Win2003 */

typedef ULONG ACCESS_MASK;

/* from DDK2003SP1/3790.1830/inc/ddk/wnet/ntddk.h */
#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)
#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

/* From http://undocumented.ntinternals.net/UserMode/Structures/RTL_USER_PROCESS_PARAMETERS.html */

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    PVOID ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StdInputHandle;
    HANDLE StdOutputHandle;
    HANDLE StdErrorHandle;
    UNICODE_STRING CurrentDirectoryPath;
    HANDLE CurrentDirectoryHandle;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PVOID Environment;
    ULONG StartingPositionLeft;
    ULONG StartingPositionTop;
    ULONG Width;
    ULONG Height;
    ULONG CharWidth;
    ULONG CharHeight;
    ULONG ConsoleTextAttributes;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopName;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    // RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20]
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

/* module information filled by the loader */
typedef struct _PEB_LDR_DATA {  
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

/* Note that these lists are walked through corresponding LIST_ENTRY pointers
 * i.e., for InInit*Order*, Flink points 16 bytes into the LDR_MODULE structure
 */
typedef struct _LDR_MODULE {
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID BaseAddress;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashTableEntry; /* CHECK: according to WINE this is HANDLE SectionHandle, ULONG CheckSum */
    /* FIXME: on XP dt ntdll!_LDR_DATA_TABLE_ENTRY does indeed agree with WINE */
    ULONG TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

/* This macro is defined so that 32-bit dlls can be handled in 64-bit DR. 
 * Not all IMAGE_OPTIONAL_HEADER fields are affected, only ImageBase,
 * LoaderFlags, NumberOfRvaAndSizes, SizeOf{Stack,Heap}{Commit,Reserve},
 * and DataDirectory, of which we use only ImageBase and DataDirectory.
 * All other fields happen to have the same offsets and sizes in both
 * IMAGE_OPTIONAL_HEADER32 and IMAGE_OPTIONAL_HEADER64.
 */
#ifdef X64
/* Don't need to use module_is_32bit() here as that is heavyweight.  Also, as
 * it is used directly in process_image() just when the module processing
 * begins, we don't have to do all the checks here.
 */
# define OPT_HDR(nt_hdr_p, field) OPT_HDR_BASE(nt_hdr_p, field, )
# define OPT_HDR_P(nt_hdr_p, field) OPT_HDR_BASE(nt_hdr_p, field, (app_pc)&)
# define OPT_HDR_BASE(nt_hdr_p, field, amp) \
    ((nt_hdr_p)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?      \
        amp(((IMAGE_OPTIONAL_HEADER32 *)&((nt_hdr_p)->OptionalHeader))->field) : \
        amp(((IMAGE_OPTIONAL_HEADER64 *)&((nt_hdr_p)->OptionalHeader))->field))
#else
# define OPT_HDR(nt_hdr_p, field) ((nt_hdr_p)->OptionalHeader.field)
# define OPT_HDR_P(nt_hdr_p, field) (&((nt_hdr_p)->OptionalHeader.field))
#endif

/* For use by routines that walk the module lists. */
enum {MAX_MODULE_LIST_INFINITE_LOOP_THRESHOLD = 2048};

/* from winternl.h from wine (thus not official)
 * these defines are (some of the) regular LDR_MODULE.Flags values
 */
#define LDR_IMAGE_IS_DLL                0x00000004
#define LDR_LOAD_IN_PROGRESS            0x00001000
#define LDR_UNLOAD_IN_PROGRESS          0x00002000
#define LDR_NO_DLL_CALLS                0x00040000
#define LDR_PROCESS_ATTACHED            0x00080000
#define LDR_MODULE_REBASED              0x00200000

typedef struct _PEBLOCKROUTINE *PPEBLOCKROUTINE;
typedef struct _PEB_FREE_BLOCK *PPEB_FREE_BLOCK;
typedef PVOID *PPVOID;

typedef struct _RTL_BITMAP {
    ULONG  SizeOfBitMap; /* Number of bits in the bitmap */
    LPBYTE BitMapBuffer; /* Bitmap data, assumed sized to a DWORD boundary */
} RTL_BITMAP, *PRTL_BITMAP;
typedef const RTL_BITMAP *PCRTL_BITMAP;

/* The layout here is from ntdll pdb on x64 xpsp2, though we
 * changed some PVOID types to more specific types.
 */
typedef struct _PEB {                                     /* offset: 32bit / 64bit */
    BOOLEAN                      InheritedAddressSpace;           /* 0x000 / 0x000 */
    BOOLEAN                      ReadImageFileExecOptions;        /* 0x001 / 0x001 */
    BOOLEAN                      BeingDebugged;                   /* 0x002 / 0x002 */
#if 0
    /* x64 xpsp2 lists this as a bitfield but compiler only accepts int bitfields: */
    BOOLEAN                      ImageUsesLargePages:1;           /* 0x003 / 0x003 */
    BOOLEAN                      SpareBits:7;                     /* 0x003 / 0x003 */
#else
    BOOLEAN                      ImageUsesLargePages;             /* 0x003 / 0x003 */
#endif
    HANDLE                       Mutant;                          /* 0x004 / 0x008 */
    PVOID                        ImageBaseAddress;                /* 0x008 / 0x010 */
    PPEB_LDR_DATA                LoaderData;                      /* 0x00c / 0x018 */
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;               /* 0x010 / 0x020 */
    PVOID                        SubSystemData;                   /* 0x014 / 0x028 */
    PVOID                        ProcessHeap;                     /* 0x018 / 0x030 */
    PRTL_CRITICAL_SECTION        FastPebLock;                     /* 0x01c / 0x038 */
#if 0
    /* x64 xpsp2 lists these fields as: */
    PVOID                        AtlThunkSListPtr;                /* 0x020 / 0x040 */
    PVOID                        SparePtr2;                       /* 0x024 / 0x048 */
#else
    /* xpsp2 and earlier */
    PPEBLOCKROUTINE              FastPebLockRoutine;              /* 0x020 / 0x040 */
    PPEBLOCKROUTINE              FastPebUnlockRoutine;            /* 0x024 / 0x048 */
#endif
    DWORD                        EnvironmentUpdateCount;          /* 0x028 / 0x050 */
    PVOID                        KernelCallbackTable;             /* 0x02c / 0x058 */
#if 0
    /* x64 xpsp2 lists these fields as: */
    DWORD                        SystemReserved[1];               /* 0x030 / 0x060 */
    DWORD                        SpareUlong;                      /* 0x034 / 0x064 */
#else
    /* xpsp2 and earlier */
    DWORD                        EvengLogSection;                 /* 0x030 / 0x060 */
    DWORD                        EventLog;                        /* 0x034 / 0x064 */
#endif
    PPEB_FREE_BLOCK              FreeList;                        /* 0x038 / 0x068 */
    DWORD                        TlsExpansionCounter;             /* 0x03c / 0x070 */
    PRTL_BITMAP                  TlsBitmap;                       /* 0x040 / 0x078 */
    DWORD                        TlsBitmapBits[2];                /* 0x044 / 0x080 */
    PVOID                        ReadOnlySharedMemoryBase;        /* 0x04c / 0x088 */
    PVOID                        ReadOnlySharedMemoryHeap;        /* 0x050 / 0x090 */
    PPVOID                       ReadOnlyStaticServerData;        /* 0x054 / 0x098 */
    PVOID                        AnsiCodePageData;                /* 0x058 / 0x0a0 */
    PVOID                        OemCodePageData;                 /* 0x05c / 0x0a8 */
    PVOID                        UnicodeCaseTableData;            /* 0x060 / 0x0b0 */
    DWORD                        NumberOfProcessors;              /* 0x064 / 0x0b8 */
    DWORD                        NtGlobalFlag;                    /* 0x068 / 0x0bc */
    LARGE_INTEGER                CriticalSectionTimeout;          /* 0x070 / 0x0c0 */
    ptr_uint_t                   HeapSegmentReserve;              /* 0x078 / 0x0c8 */
    ptr_uint_t                   HeapSegmentCommit;               /* 0x07c / 0x0d0 */
    ptr_uint_t                   HeapDeCommitTotalFreeThreshold;  /* 0x080 / 0x0d8 */
    ptr_uint_t                   HeapDeCommitFreeBlockThreshold;  /* 0x084 / 0x0e0 */
    DWORD                        NumberOfHeaps;                   /* 0x088 / 0x0e8 */
    DWORD                        MaximumNumberOfHeaps;            /* 0x08c / 0x0ec */
    PPVOID                       ProcessHeaps;                    /* 0x090 / 0x0f0 */
    PVOID                        GdiSharedHandleTable;            /* 0x094 / 0x0f8 */
    PVOID                        ProcessStarterHelper;            /* 0x098 / 0x100 */
    DWORD                        GdiDCAttributeList;              /* 0x09c / 0x108 */
    PRTL_CRITICAL_SECTION        LoaderLock;                      /* 0x0a0 / 0x110 */
    DWORD                        OSMajorVersion;                  /* 0x0a4 / 0x118 */
    DWORD                        OSMinorVersion;                  /* 0x0a8 / 0x11c */
    WORD                         OSBuildNumber;                   /* 0x0ac / 0x120 */
    WORD                         OSCSDVersion;                    /* 0x0ae / 0x122 */
    DWORD                        OSPlatformId;                    /* 0x0b0 / 0x124 */
    DWORD                        ImageSubsystem;                  /* 0x0b4 / 0x128 */
    DWORD                        ImageSubsystemMajorVersion;      /* 0x0b8 / 0x12c */
    DWORD                        ImageSubsystemMinorVersion;      /* 0x0bc / 0x130 */
    ptr_uint_t                   ImageProcessAffinityMask;        /* 0x0c0 / 0x138 */
#ifdef X64
    DWORD                        GdiHandleBuffer[60];             /* 0x0c4 / 0x140 */
#else
    DWORD                        GdiHandleBuffer[34];             /* 0x0c4 / 0x140 */
#endif
    PVOID                        PostProcessInitRoutine;          /* 0x14c / 0x230 */
    PVOID                        TlsExpansionBitmap;              /* 0x150 / 0x238 */
    DWORD                        TlsExpansionBitmapBits[32];      /* 0x154 / 0x240 */
    DWORD                        SessionId;                       /* 0x1d4 / 0x2c0 */
    ULARGE_INTEGER               AppCompatFlags;                  /* 0x1d8 / 0x2c8 */
    ULARGE_INTEGER               AppCompatFlagsUser;              /* 0x1e0 / 0x2d0 */
    PVOID                        pShimData;                       /* 0x1e8 / 0x2d8 */
    PVOID                        AppCompatInfo;                   /* 0x1ec / 0x2e0 */
    UNICODE_STRING               CSDVersion;                      /* 0x1f0 / 0x2e8 */
    PVOID                        ActivationContextData;           /* 0x1f8 / 0x2f8 */
    PVOID                        ProcessAssemblyStorageMap;       /* 0x1fc / 0x300 */
    PVOID                        SystemDefaultActivationContextData;/* 0x200 / 0x308 */
    PVOID                        SystemAssemblyStorageMap;        /* 0x204 / 0x310 */
    ptr_uint_t                   MinimumStackCommit;              /* 0x208 / 0x318 */
    PPVOID                       FlsCallback;                     /* 0x20c / 0x320 */
    LIST_ENTRY                   FlsListHead;                     /* 0x210 / 0x328 */
    PVOID                        FlsBitmap;                       /* 0x218 / 0x338 */
    DWORD                        FlsBitmapBits[4];                /* 0x21c / 0x340 */
    DWORD                        FlsHighIndex;                    /* 0x22c / 0x350 */
} PEB, *PPEB;

#ifndef _W64
# define _W64
#endif
#ifndef X64
typedef _W64 long LONG_PTR, *PLONG_PTR;
typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG KAFFINITY;
#endif
typedef LONG KPRIORITY;

typedef struct _KERNEL_USER_TIMES {
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES;

/* from DDK2003SP1/3790.1830/inc/ddk/wnet/ntddk.h */
typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,          // Note: this is kernel mode only
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    /* added after XP+ */
    ProcessImageFileName,
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,
    ProcessDebugFlags,
    ProcessHandleTracing,
    ProcessIoPriority,
    ProcessExecuteFlags,
    ProcessResourceManagement,
    ProcessCookie,
    ProcessImageInformation,
    MaxProcessInfoClass             // MaxProcessInfoClass should always be the last enum
} PROCESSINFOCLASS;

/* Process Information Structures */

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

typedef struct _DESCRIPTOR_TABLE_ENTRY {
    ULONG Selector;
    LDT_ENTRY Descriptor;
} DESCRIPTOR_TABLE_ENTRY, *PDESCRIPTOR_TABLE_ENTRY;

/* format of data returned by QueryInformationProcess ProcessVmCounters */
typedef struct _VM_COUNTERS {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS;

/* format of data returned by QueryInformationProcess ProcessDeviceMap */
typedef struct _PROCESS_DEVICEMAP_INFORMATION {
    union {
        struct {
            HANDLE DirectoryHandle;
        } Set;
        struct {
            ULONG DriveMap;
            UCHAR DriveType[32];
        } Query;
    };
#ifdef X64
    ULONG Flags;
#endif
} PROCESS_DEVICEMAP_INFORMATION, *PPROCESS_DEVICEMAP_INFORMATION;

#if defined(NOT_DYNAMORIO_CORE)
# ifndef bool
typedef int bool;
# endif /* bool */
typedef unsigned __int64 uint64;
#endif /* NOT_DYNAMORIO_CORE */

/* Case 7395: mmcgui build fails with redefinition of
 * _JOBOBJECT_EXTENDED_LIMIT_INFORMATION and _IO_COUNTERS
 */
#if !defined(NOT_DYNAMORIO_CORE) && !defined(NOT_DYNAMORIO_CORE_PROPER)

/* For NtQueryInformationProcess using ProcessQuotaLimits or
 * ProcessPooledQuotaLimits or for NtSetInformationProcess using
 * ProcessQuotaLimits */
/* QUOTA_LIMITS defined in VC98/Include/WINNT.H - from WinNT+ */

/* note for NtSetInformationProcess when setting can set either
 * working set or the other values: only when both
 * MinimumWorkingSetSize and MaximumWorkingSetSize are non-zero
 * working set is adjusted, and the other values are ignored. 
 * (Nebbett p.141)
 * 
 * Job and working set note from MSDN "Processes can still empty their
 * working sets using the SetProcessWorkingSetSize function, even when
 * JOB_OBJECT_LIMIT_WORKINGSET is used.  However, you cannot use
 * SetProcessWorkingSetSize to change the minimum or maximum working
 * set size."
 */

/* PageFaultHistory Information 
 * NtQueryInformationProcess ProcessWorkingSetWatch 
 */ 
typedef struct _PROCESS_WS_WATCH_INFORMATION {
    PVOID FaultingPc;
    PVOID FaultingVa;
} PROCESS_WS_WATCH_INFORMATION, *PPROCESS_WS_WATCH_INFORMATION;

/* NtQueryInformationProcess ProcessPooledUsageAndLimits */
typedef struct _POOLED_USAGE_AND_LIMITS {
    SIZE_T PeakPagedPoolUsage;
    SIZE_T PagedPoolUsage;
    SIZE_T PagedPoolLimit;
    SIZE_T PeakNonPagedPoolUsage;
    SIZE_T NonPagedPoolUsage;
    SIZE_T NonPagedPoolLimit;
    SIZE_T PeakPagefileUsage;
    SIZE_T PagefileUsage;
    SIZE_T PagefileLimit;
} POOLED_USAGE_AND_LIMITS;
typedef POOLED_USAGE_AND_LIMITS *PPOOLED_USAGE_AND_LIMITS;

/* Process Security Context Information
 *  NtSetInformationProcess ProcessAccessToken
 *  PROCESS_SET_ACCESS_TOKEN access needed to use
 */
typedef struct _PROCESS_ACCESS_TOKEN {
    //
    // Handle to Primary token to assign to the process.
    // TOKEN_ASSIGN_PRIMARY access to this token is needed.
    //
    HANDLE Token;

    //
    // Handle to the initial thread of the process.
    // A process's access token can only be changed if the process has
    // no threads or one thread.  If the process has no threads, this
    // field must be set to NULL.  Otherwise, it must contain a handle
    // open to the process's only thread.  THREAD_QUERY_INFORMATION access
    // is needed via this handle.
    HANDLE Thread;
} PROCESS_ACCESS_TOKEN, *PPROCESS_ACCESS_TOKEN;

/* End of Process Information Structures */

/* Basic Job Limit flags, specified in JOBOBJECT_BASIC_LIMIT_INFORMATION */
#define JOB_OBJECT_LIMIT_WORKINGSET                 0x00000001
#define JOB_OBJECT_LIMIT_PROCESS_TIME               0x00000002
#define JOB_OBJECT_LIMIT_JOB_TIME                   0x00000004
#define JOB_OBJECT_LIMIT_ACTIVE_PROCESS             0x00000008
#define JOB_OBJECT_LIMIT_AFFINITY                   0x00000010
#define JOB_OBJECT_LIMIT_PRIORITY_CLASS             0x00000020
#define JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME          0x00000040
#define JOB_OBJECT_LIMIT_SCHEDULING_CLASS           0x00000080

/* Extended Job Limit flags, specified in JOBOBJECT_EXTENDED_LIMIT_INFORMATION */
#define JOB_OBJECT_LIMIT_PROCESS_MEMORY             0x00000100
#define JOB_OBJECT_LIMIT_JOB_MEMORY                 0x00000200
#define JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION 0x00000400
#define JOB_OBJECT_LIMIT_BREAKAWAY_OK               0x00000800
#define JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK        0x00001000
#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE          0x00002000

/* End of Job Limits */
#endif /* !NOT_DYNAMORIO_CORE && !NOT_DYNAMORIO_CORE_PROPER */

typedef struct _CLIENT_ID {
    /* These are numeric ids */
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

/* OS dependent SEH frame supported by ntdll.dll 
   referred to in WINNT.H as _EXCEPTION_REGISTRATION_RECORD */
typedef struct _EXCEPTION_REGISTRATION {
     struct _EXCEPTION_REGISTRATION* prev;
     PVOID                   handler;
} EXCEPTION_REGISTRATION, *PEXCEPTION_REGISTRATION_RECORD;

typedef struct _GDI_TEB_BATCH
{
    ULONG  Offset;
    HANDLE HDC;
    ULONG  Buffer[0x136];
} GDI_TEB_BATCH;

/* The layout here is from ntdll pdb on x64 xpsp2 */
typedef struct _TEB {                               /* offset: 32bit / 64bit */
    /* We lay out NT_TIB, which is declared in winnt.h */    
    PEXCEPTION_REGISTRATION_RECORD ExceptionList;           /* 0x000 / 0x000 */
    PVOID                     StackBase;                    /* 0x004 / 0x008 */
    PVOID                     StackLimit;                   /* 0x008 / 0x010 */
    PVOID                     SubSystemTib;                 /* 0x00c / 0x018 */
    union {
        PVOID                 FiberData;                    /* 0x010 / 0x020 */
        DWORD                 Version;                      /* 0x010 / 0x020 */
    };
    PVOID                     ArbitraryUserPointer;         /* 0x014 / 0x028 */
    struct _TEB*              Self;                         /* 0x018 / 0x030 */
    PVOID                     EnvironmentPointer;           /* 0x01c / 0x038 */
    CLIENT_ID                 ClientId;                     /* 0x020 / 0x040 */
    PVOID                     ActiveRpcHandle;              /* 0x028 / 0x050 */
    PVOID                     ThreadLocalStoragePointer;    /* 0x02c / 0x058 */
    PEB*                      ProcessEnvironmentBlock;      /* 0x030 / 0x060 */
    DWORD                     LastErrorValue;               /* 0x034 / 0x068 */
    DWORD                     CountOfOwnedCriticalSections; /* 0x038 / 0x06c */
    PVOID                     CsrClientThread;              /* 0x03c / 0x070 */
    PVOID                     Win32ThreadInfo;              /* 0x040 / 0x078 */
    DWORD                     User32Reserved[26];           /* 0x044 / 0x080 */
    DWORD                     UserReserved[5];              /* 0x0ac / 0x0e8 */
    PVOID                     WOW32Reserved;                /* 0x0c0 / 0x100 */
    DWORD                     CurrentLocale;                /* 0x0c4 / 0x108 */
    DWORD                     FpSoftwareStatusRegister;     /* 0x0c8 / 0x10c */
    PVOID /* kernel32 data */ SystemReserved1[54];          /* 0x0cc / 0x110 */
    LONG                      ExceptionCode;                /* 0x1a4 / 0x2c0 */
    PVOID                     ActivationContextStackPointer;/* 0x1a8 / 0x2c8 */
#ifdef X64
    byte                      SpareBytes1[28];              /* 0x1ac / 0x2d0 */
#else
    byte                      SpareBytes1[40];              /* 0x1ac / 0x2d0 */
#endif
    GDI_TEB_BATCH             GdiTebBatch;                  /* 0x1d4 / 0x2f0 */
    CLIENT_ID                 RealClientId;                 /* 0x6b4 / 0x7d8 */
    PVOID                     GdiCachedProcessHandle;       /* 0x6bc / 0x7e8 */
    DWORD                     GdiClientPID;                 /* 0x6c0 / 0x7f0 */
    DWORD                     GdiClientTID;                 /* 0x6c4 / 0x7f4 */
    PVOID                     GdiThreadLocalInfo;           /* 0x6c8 / 0x7f8 */
    ptr_uint_t                Win32ClientInfo[62];          /* 0x6cc / 0x800 */
    PVOID                     glDispatchTable[233];         /* 0x7c4 / 0x9f0 */
    ptr_uint_t                glReserved1[29];              /* 0xb68 / 0x1138 */
    PVOID                     glReserved2;                  /* 0xbdc / 0x1220 */
    PVOID                     glSectionInfo;                /* 0xbe0 / 0x1228 */
    PVOID                     glSection;                    /* 0xbe4 / 0x1230 */
    PVOID                     glTable;                      /* 0xbe8 / 0x1238 */
    PVOID                     glCurrentRC;                  /* 0xbec / 0x1240 */
    PVOID                     glContext;                    /* 0xbf0 / 0x1248 */
    DWORD                     LastStatusValue;              /* 0xbf4 / 0x1250 */
    UNICODE_STRING            StaticUnicodeString;          /* 0xbf8 / 0x1258 */
    WORD                      StaticUnicodeBuffer[261];     /* 0xc00 / 0x1268 */
    PVOID                     DeallocationStack;            /* 0xe0c / 0x1478 */
    PVOID                     TlsSlots[64];                 /* 0xe10 / 0x1480 */
    LIST_ENTRY                TlsLinks;                     /* 0xf10 / 0x1680 */
    PVOID                     Vdm;                          /* 0xf18 / 0x1690 */
    PVOID                     ReservedForNtRpc;             /* 0xf1c / 0x1698 */
    PVOID                     DbgSsReserved[2];             /* 0xf20 / 0x16a0 */
    DWORD                     HardErrorMode;                /* 0xf28 / 0x16b0 */
    PVOID                     Instrumentation[14];          /* 0xf2c / 0x16b8 */
    PVOID                     SubProcessTag;                /* 0xf64 / 0x1728 */
    PVOID                     EtwTraceData;                 /* 0xf68 / 0x1730 */
    PVOID                     WinSockData;                  /* 0xf6c / 0x1738 */
    DWORD                     GdiBatchCount;                /* 0xf70 / 0x1740 */
    byte                      InDbgPrint;                   /* 0xf74 / 0x1744 */
    byte                      FreeStackOnTermination;       /* 0xf75 / 0x1745 */
    byte                      HasFiberData;                 /* 0xf76 / 0x1746 */
    byte                      IdealProcessor;               /* 0xf77 / 0x1747 */
    DWORD                     GuaranteedStackBytes;         /* 0xf78 / 0x1748 */
    PVOID                     ReservedForPerf;              /* 0xf7c / 0x1750 */
    PVOID                     ReservedForOle;               /* 0xf80 / 0x1758 */
    DWORD                     WaitingOnLoaderLock;          /* 0xf84 / 0x1760 */
    ptr_uint_t                SparePointer1;                /* 0xf88 / 0x1768 */
    ptr_uint_t                SoftPatchPtr1;                /* 0xf8c / 0x1770 */
    ptr_uint_t                SoftPatchPtr2;                /* 0xf90 / 0x1778 */
    PPVOID                    TlsExpansionSlots;            /* 0xf94 / 0x1780 */
#ifdef X64
    PVOID                     DeallocationBStore;           /* ----- / 0x1788 */
    PVOID                     BStoreLimit;                  /* ----- / 0x1790 */
#endif
    DWORD                     ImpersonationLocale;          /* 0xf98 / 0x1798 */
    DWORD                     IsImpersonating;              /* 0xf9c / 0x179c */
    PVOID                     NlsCache;                     /* 0xfa0 / 0x17a0 */
    PVOID                     pShimData;                    /* 0xfa4 / 0x17a8 */
    DWORD                     HeapVirtualAffinity;          /* 0xfa8 / 0x17b0 */
    PVOID                     CurrentTransactionHandle;     /* 0xfac / 0x17b8 */
    PVOID                     ActiveFrame;                  /* 0xfb0 / 0x17c0 */
    PVOID                     FlsData;                      /* 0xfb4 / 0x17c8 */
    byte                      SafeThunkCall;                /* 0xfb8 / 0x17d0 */
    byte                      BooleanSpare[3];              /* 0xfb9 / 0x17d1 */
} TEB;

typedef struct _THREAD_BASIC_INFORMATION { // Information Class 0
    NTSTATUS ExitStatus;
    PNT_TIB TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef struct _USER_STACK {
    PVOID FixedStackBase;
    PVOID FixedStackLimit;
    PVOID ExpandableStackBase;
    PVOID ExpandableStackLimit;
    PVOID ExpandableStackBottom;
} USER_STACK, *PUSER_STACK;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
//          Data[1]             // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation
} KEY_INFORMATION_CLASS;

typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessesAndThreadsInformation = 5,
    SystemProcessorTimes = 8,
    SystemGlobalFlag = 9,
    SystemModuleInformation = 11,
    SystemLockInformation = 12,
    SystemHandleInformation = 16,
    SystemObjectInformation = 17
    /* many more, see Nebbett */
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG   Unknown;
    ULONG   MaximumIncrement;
    ULONG   PhysicalPageSize;
    ULONG   NumberOfPhysicalPages;
    ULONG   LowestPhysicalPage;
    ULONG   HighestPhysicalPage;
    ULONG   AllocationGranularity;
    PVOID   LowestUserAddress;
    PVOID   HighestUserAddress;
    ULONG_PTR ActiveProcessors;
    UCHAR   NumberProcessors;
#ifdef X64
    ULONG   Unknown2; /* set to 0: probably just padding to 8-byte max field align */
#endif
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    USHORT  ProcessorArchitecture;
    USHORT  ProcessorLevel;
    USHORT  ProcessorRevision;
    USHORT  Unknown;
    ULONG   FeatureBits;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    ULONG   ReadOperationCount;
    ULONG   WriteOperationCount;
    ULONG   OtherOperationCount;
    ULONG   AvailablePages;
    ULONG   TotalCommittedPages;
    ULONG   TotalCommitLimit;
    ULONG   PeakCommitment;
    ULONG   PageFaults;
    ULONG   WriteCopyFaults;
    ULONG   TranstitionFaults;
    ULONG   Reserved1;
    ULONG   DemandZeroFaults;
    ULONG   PagesRead;
    ULONG   PageReadIos;
    ULONG   Reserved2[2];
    ULONG   PageFilePagesWritten;
    ULONG   PageFilePagesWriteIos;
    ULONG   MappedFilePagesWritten;
    ULONG   PagedPoolUsage;
    ULONG   NonPagedPoolUsage;
    ULONG   PagedPoolAllocs;
    ULONG   PagedPoolFrees;
    ULONG   NonPagedPoolAllocs;
    ULONG   NonPagedPoolFrees;
    ULONG   TotalFreeSystemPtes;
    ULONG   SystemCodePage;
    ULONG   TotalSystemDriverPages;
    ULONG   TotalSystemCodePages;
    ULONG   SmallNonPagedLookasideListAllocateHits;
    ULONG   SmallPagedLookasieListAllocateHits;
    ULONG   Reserved3;
    ULONG   MmSystemCachePage;
    ULONG   PagedPoolPage;
    ULONG   SystemDriverPage;
    ULONG   FastReadNoWait;
    ULONG   FastReadWait;
    ULONG   FastReadResourceMiss;
    ULONG   FastReadNotPossible;
    ULONG   FastMdlReadNoWait;
    ULONG   FastMdlReadWait;
    ULONG   FastMdlReadResourceMiss;
    ULONG   FastMdlReadNotPossible;
    ULONG   MapDataNoWait;
    ULONG   MapDataWait;
    ULONG   MapDataNoWaitMiss;
    ULONG   MapDataWaitMiss;
    ULONG   PinMappedDataCount;
    ULONG   PinReadNoWait;
    ULONG   PinReadWait;
    ULONG   PinReadNoWaitMiss;
    ULONG   PinReadWaitMiss;
    ULONG   CopyReadNoWait;
    ULONG   CopyReadWait;
    ULONG   CopyReadNoWaitMiss;
    ULONG   CopyReadWaitMiss;
    ULONG   MdlReadNoWait;
    ULONG   MdlReadWait;
    ULONG   MdlReadNoWaitMiss;
    ULONG   MdlReadWaitMiss;
    ULONG   ReadAheadIos;
    ULONG   LazyWriteIos;
    ULONG   LazyWritePages;
    ULONG   DataFlushes;
    ULONG   DataPages;
    ULONG   ContextSwitches;
    ULONG   FirstLevelTbFills;
    ULONG   SecondLevelTbFills;
    ULONG   SystemCalls;
    /* Fields added in Windows 7 */
    ULONG   Unknown[4];
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_TIME_OF_DAY_INFORMATION {
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG   CurrentTimeZoneId;
} SYSTEM_TIME_OF_DAY_INFORMATION, *PSYSTEM_TIME_OF_DAY_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_TIMES {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG   InterruptCount;
} SYSTEM_PROCESSOR_TIMES, *PSYSTEM_PROCESSOR_TIMES;

typedef struct _IO_COUNTERSEX {
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
} IO_COUNTERSEX, *PIO_COUNTERSEX;

typedef enum _THREAD_STATE {
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition,
    StateUnknown
} THREAD_STATE;

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrVirtualMemory,
    WrPageOut,
    WrRendevous,
    WrSpare2,
    WrSpare3,
    WrSpare4,
    WrSpare5,
    WrSpare6,
    WrKernel
} KWAIT_REASON;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    ULONG   WaitTime;
    PVOID   StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
    ULONG   ContextSwitchCount;
    THREAD_STATE ThreadState;
    KWAIT_REASON WaitReason;
} SYSTEM_THREADS, *PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES { 
    ULONG NextEntryDelta;
    ULONG ThreadCount;
    ULONG Reserved1[6];
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY BasePriority;
    HANDLE ProcessId;
    HANDLE InheritedFromProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG PageDirectoryFrame;
    VM_COUNTERS VmCounters;
    IO_COUNTERSEX IoCounters;  /* Windows 2000+ only */
    SYSTEM_THREADS Threads[1]; /* Variable size */
} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;


typedef struct _SYSTEM_GLOBAL_FLAG {
    ULONG   GlobalFlag;
} SYSTEM_GLOBAL_FLAG, *PSYSTEM_GLOBAL_FLAG;

typedef struct _FILE_NETWORK_OPEN_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG   FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation,
    MemoryWorkingSetList,
    MemorySectionName,
    MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_SECTION_NAME {
    UNICODE_STRING SectionFileName;
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

#define SYMBOLIC_LINK_QUERY (0x1)
#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYMBOLIC_LINK_QUERY)

/* Speculated arg 10 to NtCreateUserProcess.
 * Note the similarities to CreateThreadEx arg 11 below.  Struct starts with size then
 * after that looks kind of like an array of 16 byte (32 on 64-bit) elements corresponding
 * to the IN and OUT informational ptrs.  Each array elment consists of a ?flags? then
 * the sizeof of the IN/OUT ptr buffer then the ptr itself then 0.
 */

typedef enum { /* NOTE - these are speculative */
    THREAD_INFO_ELEMENT_BUFFER_IS_INOUT = 0x00000, /* buffer is ??IN/OUT?? */
    THREAD_INFO_ELEMENT_BUFFER_IS_OUT   = 0x10000, /* buffer is IN (?) */
    THREAD_INFO_ELEMENT_BUFFER_IS_IN    = 0x20000, /* buffer is OUT (?) */
} thread_info_elm_buf_access_t;

typedef enum { /* NOTE - these are speculative */
    THREAD_INFO_ELEMENT_CLIENT_ID       = 0x3, /* buffer is CLIENT_ID - OUT */
    THREAD_INFO_ELEMENT_TEB             = 0x4, /* buffer is TEB * - OUT */
    THREAD_INFO_ELEMENT_NT_PATH_TO_EXE  = 0x5, /* buffer is wchar * path to exe
                                                * [ i.e. L"\??\c:\foo.exe" ] - IN */
    THREAD_INFO_ELEMENT_EXE_STUFF       = 0x6, /* buffer is exe_stuff_t (see above) 
                                                * - INOUT */
    THREAD_INFO_ELEMENT_UNKNOWN_1       = 0x9, /* Unknown - ptr_uint_t sized
                                                * [ observed 1 ] - IN */
} thread_info_elm_buf_type_t;

typedef struct _thread_info_element_t { /* NOTE - this is speculative */
    ptr_uint_t flags;   /* thread_info_elm_buf_access_t | thread_info_elm_buf_type_t */
    size_t buffer_size; /* sizeof of buffer, in bytes */
    void *buffer;       /* flags determine disposition, could be IN or OUT or both */
    ptr_uint_t unknown;  /* [ observed always 0 ] */
} thread_info_elm_t;

typedef struct _exe_stuff_t { /* NOTE - this is speculative */
    OUT void *exe_entrypoint_addr; /* Entry point to the exe being started. */
    // ratio of uint32 to ptr_uint_t assumes no larger changes between 32 and 64-bit
    ptr_uint_t unknown1[3]; // possibly intermixed with uint32s below IN? OUT?
    uint32 unknown2[8];     // possible intermixed with ptr_uint_ts above IN? OUT?
} exe_stuff_t;

typedef struct _create_proc_thread_info_t { /* NOTE - this is speculative */
    size_t struct_size; /* observed 0x34 or 0x44 (0x68 on 64-bit) = sizeof(this struct) */
    /* Observed - first thread_info_elm_t always 
     * flags = 0x20005
     * buffer_size = varies (sizeof buffer string in bytes)
     * buffer = wchar * : nt path to executable i.e. "\??\c:\foo.exe" - IN */
    thread_info_elm_t nt_path_to_exe;
    /* Observed - second thread_info_elm_t always
     * flags = 0x10003
     * buffer_size = sizeof(CLIENT_ID)
     * buffer = PCLIENT_ID : OUT */
    thread_info_elm_t client_id;
    /* Observed - third thread_info_elm_t always
     * flags = 0x6
     * buffer_size = 0x30 (or 0x40 on 64-bit) == sizeof(exe_stuff_t)
     * buffer = exe_stuff_t * : IN/OUT */
    thread_info_elm_t exe_stuff;
    /* While the first three thread_info_elm_t have been present in every call I've seen
     * (and attempts to remove or re-arrange them caused the system call to fail,
     * assuming I managed to do it right), there's more variation in the later fields
     * (sometimes present, sometimes not) - most commonly there'll be nothing or just the
     * TEB * info field (flags = 0x10003) which I've seen here a lot on 32bit. */
#if 0 /* 0 sized array is non-standard extension */
    thread_info_elm_t info[];
#endif
}  create_proc_thread_info_t;

/* Speculated arg 11 to NtCreateThreadEx.  See the similar arg 10 of
 * NtCreateUserProcess above. */
typedef struct _create_thread_info_t { /* NOTE - this is speculative */
    size_t struct_size; /* observed 0x24 (0x48 on 64-bit) == sizeof(this struct) */
    /* Note kernel32!CreateThread hardcodes all the values in this structure and
     * I've never seen any variation elsewhere. Trying to swap the order caused the
     * system call to fail when I tried it (assuming I did it right). */
    /* Observed - always
     * flags = 0x10003
     * buffer_size = sizeof(CLIENT_ID)
     * buffer = PCLIENT_ID : OUT */
    thread_info_elm_t client_id;
    /* Observed - always
     * flags = 0x10004
     * buffer_size = sizeof(CLIENT_ID)
     * buffer = TEB ** : OUT */
    thread_info_elm_t teb;
} create_thread_info_t;

/***************************************************************************
 * convenience enums
 */
typedef enum {MEMORY_RESERVE_ONLY = MEM_RESERVE, 
              MEMORY_COMMIT = MEM_RESERVE|MEM_COMMIT
} memory_commit_status_t;

/***************************************************************************
 * function declarations
 */

void
ntdll_init(void);

void
ntdll_exit(void);

int
close_handle(HANDLE h);

/* from winddk.h used by cygwin */
#define DUPLICATE_SAME_ATTRIBUTES   0x00000004
NTSTATUS
duplicate_handle(HANDLE source_process, HANDLE source, HANDLE target_process, 
                 HANDLE *target, ACCESS_MASK access, uint attributes, 
                 uint options);

ACCESS_MASK
nt_get_handle_access_rights(HANDLE handle);

typedef enum {THREAD_EXITED, THREAD_NOT_EXITED, THREAD_EXIT_ERROR} thread_exited_status_t;
thread_exited_status_t
is_thread_exited(HANDLE hthread);

thread_id_t
thread_id_from_handle(HANDLE h);

process_id_t
process_id_from_handle(HANDLE h);

process_id_t
process_id_from_thread_handle(HANDLE h);

PEB *
get_peb(HANDLE h);

PEB *
get_own_peb(void);

TEB *
get_teb(HANDLE h);

TEB *
get_own_teb(void);

void *
get_ntdll_base(void);

#if !defined(NOT_DYNAMORIO_CORE_PROPER) && !defined(NOT_DYNAMORIO_CORE)
bool
is_in_ntdll(app_pc pc);
#endif

/* replacement for API call VirtualQuery */
size_t
query_virtual_memory(const byte *pc, MEMORY_BASIC_INFORMATION *mbi, size_t mbilen);

NTSTATUS
get_mapped_file_name(const byte *pc, PWSTR buf, USHORT buf_bytes);

/* for allocating in another process, 
 * keep in mind base is now IN/OUT value, a NULL value means no preference
 * will bump size up to PAGE_SIZE multiple */
NTSTATUS
nt_remote_allocate_virtual_memory(HANDLE process, void **base, size_t size,
                                  uint prot, memory_commit_status_t commit);

/* will bump size up to PAGE_SIZE multiple 
 * keep in mind base is now IN/OUT value, a NULL value means no preference
 */
NTSTATUS
nt_allocate_virtual_memory(void **base, size_t size, uint prot,
                           memory_commit_status_t commit);

NTSTATUS
nt_commit_virtual_memory(void *base, size_t size, uint prot);
NTSTATUS
nt_decommit_virtual_memory(void *base, size_t size);

NTSTATUS
nt_free_virtual_memory(void *base);

int
protect_virtual_memory(void *base, size_t size, uint prot, uint *old_prot);

int
nt_remote_protect_virtual_memory(HANDLE process,
                       void *base, size_t size, uint prot, uint *old_prot);

int
nt_read_virtual_memory(HANDLE process, const void *base, void *buffer, 
                       size_t buffer_length, size_t *bytes_read);

int
nt_write_virtual_memory(HANDLE process, void *base, const void *buffer, 
                        size_t buffer_length, size_t *bytes_written);

void
nt_continue(CONTEXT *cxt);

NTSTATUS
nt_get_context(HANDLE hthread, CONTEXT *cxt);

NTSTATUS
nt_set_context(HANDLE hthread, CONTEXT *cxt);

int
nt_thread_suspend(HANDLE hthread, int *previous_suspend_count);

int
nt_thread_resume(HANDLE hthread, int *previous_suspend_count);

int
nt_terminate_thread(HANDLE hthread, NTSTATUS exit_code);

int 
nt_terminate_process(HANDLE hprocess, NTSTATUS exit_code);

NTSTATUS
nt_terminate_process_for_app(HANDLE hprocess, NTSTATUS exit_code);

int
am_I_sole_thread(HANDLE hthread, int *amI /*OUT*/);

/* checks current thread, and turns errors into false */
int
check_sole_thread(void);

/* creates a waitable timer */
HANDLE
nt_create_and_set_timer(PLARGE_INTEGER due_time, LONG period);

/* thread sleeps for given relative or absolute time */
int
nt_sleep(PLARGE_INTEGER due_time);

void
nt_yield(void);

int
nt_raise_exception(EXCEPTION_RECORD* pexcrec, CONTEXT* pcontext);

int
nt_messagebox(wchar_t *msg, wchar_t *title);

int
tls_alloc(int synch, uint *teb_offs /* OUT */);

/* Allocates num tls slots aligned with alignment align */
int
tls_calloc(int synch, uint *teb_offs /* OUT */, int num, uint alignment);

int
tls_free(int synch, uint teb_offs);

int
tls_cfree(int synch, uint teb_offs, int num);

int
get_process_mem_stats(HANDLE h, VM_COUNTERS *info);

NTSTATUS
get_process_mem_quota(HANDLE h, QUOTA_LIMITS *qlimits);

NTSTATUS
get_process_handle_count(HANDLE ph, ULONG *handle_count);

int
get_process_load(HANDLE h);

int
is_wow64_process(HANDLE h);

NTSTATUS
nt_get_drive_map(HANDLE process, PROCESS_DEVICEMAP_INFORMATION *map OUT);

void *
get_section_address(HANDLE h);

/* Section attributes as described in Nebbett, p. 105.
 * Some are documented in the SDK CreateFileMapping */
/* from wnet/winnt.h, new in Win2003 */
#define SEC_LARGE_PAGES           0x80000000     
/* SEC_VLM in VC98/winnt.h but in DDK, apparently not implemented on x86 */
#ifndef SEC_VLM /* not defined in VC8 headers */
# define SEC_VLM                  0x02000000
#endif
/* Similarly these may have existed in earlier windows versions as well */
#define SEC_BASED_UNSUPPORTED     0x00200000
#define SEC_NO_CHANGE_UNSUPPORTED 0x00400000

int
get_section_attributes(HANDLE h, uint *section_attributes /* OUT */,
                       LARGE_INTEGER* section_size /* OUT OPTIONAL */);


HANDLE
reg_create_key(HANDLE parent, PCWSTR keyname, ACCESS_MASK rights);

HANDLE
reg_open_key(PCWSTR keyname, ACCESS_MASK rights);

int
reg_close_key(HANDLE hkey);

int
reg_delete_key(HANDLE hkey);

typedef enum _reg_query_value_result {
    REG_QUERY_FAILURE,
    REG_QUERY_BUFFER_TOO_SMALL,
    REG_QUERY_SUCCESS
} reg_query_value_result_t;

reg_query_value_result_t
reg_query_value(PCWSTR hkey, PCWSTR subkey, 
                KEY_VALUE_INFORMATION_CLASS key_class, PVOID info, ULONG info_size,
                ACCESS_MASK rights);

int
reg_set_key_value(HANDLE hkey, PCWSTR subkey, PCWSTR val);
int
reg_set_dword_key_value(HANDLE hkey, PCWSTR subkey, DWORD val);

int
reg_flush_key(HANDLE hkey);

int
reg_enum_key(PCWSTR keyname, ULONG index, KEY_INFORMATION_CLASS info_class,
             PVOID key_info, ULONG key_info_size);

int
reg_enum_value(PCWSTR keyname, ULONG index, KEY_VALUE_INFORMATION_CLASS key_class,
               PVOID key_info, ULONG key_info_length);

int
env_get_value(PCWSTR var, wchar_t *val, size_t valsz);

#define LengthRequiredSID(subauthorities)                                       \
      (sizeof(SID) - (ANYSIZE_ARRAY * sizeof(DWORD)) /* SID.SubAuthority */     \
       + ((subauthorities) * sizeof(DWORD)))

#ifndef SECURITY_MAX_SID_SIZE
/* see DDK2003SP1/3790.1830/inc/wnet/winnt.h */
# define SECURITY_MAX_SID_SIZE \
      (LengthRequiredSID(SID_MAX_SUB_AUTHORITIES))
#endif

/* Note: the above static SID size may change in future versions, so
 * we need to keep track of winnt.h
 */
#define SECURITY_MAX_TOKEN_SIZE (SECURITY_MAX_SID_SIZE + sizeof(SID_AND_ATTRIBUTES))

NTSTATUS
get_primary_user_token(PTOKEN_USER ptoken, USHORT token_buffer_length);
NTSTATUS
get_current_user_token(PTOKEN_USER ptoken, USHORT token_buffer_length);
NTSTATUS
get_primary_owner_token(PTOKEN_OWNER powner, USHORT owner_buffer_length);

NTSTATUS
get_current_user_SID(PWSTR sid_string, USHORT buffer_length);

bool
get_owner_sd(PISECURITY_DESCRIPTOR SecurityDescriptor,
             PSID* Owner);
void
initialize_security_descriptor(PISECURITY_DESCRIPTOR SecurityDescriptor);
/* use only on security descriptors created with initialize_security_descriptor() */
bool
set_owner_sd(PISECURITY_DESCRIPTOR SecurityDescriptor,
             PSID Owner);

bool
equal_sid(IN PSID Sid1, IN PSID Sid2);
void
initialize_known_SID(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
                     ULONG SubAuthority0,
                     SID *pSid);

NTSTATUS
query_seg_descriptor(HANDLE hthread, DESCRIPTOR_TABLE_ENTRY *entry);

NTSTATUS
query_win32_start_addr(HANDLE hthread, PVOID start_addr);

NTSTATUS 
query_system_info(SYSTEM_INFORMATION_CLASS info_class, int info_size, PVOID info);

int
query_full_attributes_file(PCWSTR filename,                           
                           PFILE_NETWORK_OPEN_INFORMATION info);

/* access mask flags, from ntddk.h rest are in winnt.h so no need to replicate
 * them here */
#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

/* share flags, from ntddk.h, rest are in winnt.h */
#define FILE_SHARE_VALID_FLAGS          0x00000007

/* attribute flags, from ntddk.h, rest are in winnt.h */
#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

/* flags for NtCreateFile and NtOpenFile */
/* Create/Open options from ntddk.h */
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define FILE_STRUCTURED_STORAGE                 0x00000441

#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036

/* iob.Information for NtCreateFile or NtOpenFile */
#define FILE_SUPERSEDED     0x00000000
#define FILE_OPENED         0x00000001
#define FILE_CREATED        0x00000002
#define FILE_OVERWRITTEN    0x00000003
#define FILE_EXISTS         0x00000004
#define FILE_DOES_NOT_EXIST 0x00000005

/* CreateDisposition, from the ntddk.h */
#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

/* note this is our own private flag, to explicitly set file owner */
#define FILE_DISPOSITION_SET_OWNER      0x10000000

/* from ntddk.h, special ByteOffset Parameters */
#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

/* From NTSTATUS.H -- this shouldn't change, but you never know... */
/* DDK2003SP1/3790.1830/inc/wnet/ntstatus.h */
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

/* {Conflicting Address Range}  The specified address range conflicts with the address space. */
#define STATUS_CONFLICTING_ADDRESSES     ((NTSTATUS)0xC0000018L)

/* "The address handle given to the transport was invalid." */
#define STATUS_INVALID_ADDRESS           ((NTSTATUS)0xC0000141L)

/* "The data was too large to fit into the specified buffer." */
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)

/*  {Bad File} The attributes of the specified mapping file for a
*  section of memory cannot be read. */
#define STATUS_INVALID_FILE_FOR_SECTION  ((NTSTATUS)0xC0000020L)

/*  {Access Denied} A process has requested access to an object, but
 *  has not been granted those access rights.*/
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)

/* "The buffer is too small to contain the entry. No information has been
 * written to the buffer." */
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)

/*  Object Name not found. */
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)

/* Error: Object Name already exists */
#define STATUS_OBJECT_NAME_COLLISION     ((NTSTATUS)0xC0000035L)

/*  A file cannot be opened because the share access flags are incompatible. */
#define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)

/*  A non close operation has been requested of a file object with a delete pending. */
#define STATUS_DELETE_PENDING            ((NTSTATUS)0xC0000056L)

/* The volume for a file has been externally altered such that the opened file 
 * is no longer valid.
 */
#define STATUS_FILE_INVALID              ((NTSTATUS)0xC0000098L)

/*  The file that was specified as a target is a directory and the
 * caller specified that it could be anything but a directory.
 */
#define STATUS_FILE_IS_A_DIRECTORY       ((NTSTATUS)0xC00000BAL)

/* Warning: An attempt was made to create an object and the object name already existed. */
#define STATUS_OBJECT_NAME_EXISTS        ((NTSTATUS)0x40000000L)

/*  An invalid parameter was passed to a service or function as the fourth argument. */
#define STATUS_INVALID_PARAMETER_4       ((NTSTATUS)0xC00000F2L)

/* Warning: Image Relocated
 *  "An image file could not be mapped at the address specified in the image file. "
 *  "Local fixups must be performed on this image."
 */
#define STATUS_IMAGE_NOT_AT_BASE         ((NTSTATUS)0x40000003L)

#if !defined(STATUS_NO_MEMORY)
/* {Not Enough Quota} Not enough virtual memory or paging file quota
 *  is available to complete the specified operation. */
#  define STATUS_NO_MEMORY                 ((NTSTATUS)0xC0000017L)   /* winnt */
/* only seen on virtual memory reservation, maybe on NT it served multiple purposes */
#endif

/*  Page file quota was exceeded. */
#define STATUS_PAGEFILE_QUOTA_EXCEEDED   ((NTSTATUS)0xC000012CL)
/* not seen */

/* {Out of Virtual Memory} Your system is low on virtual memory. To
 *  ensure that Windows runs properly, increase the size of your
 *  virtual memory paging file. For more information, see Help.
 */
#define STATUS_COMMITMENT_LIMIT          ((NTSTATUS)0xC000012DL)
/* Seen both when reaching the Commit limit (memory + page file size), 
 * as well as when a process in a Job reaches its ProcessMemoryLimit
 */

/* {Virtual Memory Minimum Too Low} Your system is low on virtual
 *  memory. Windows is increasing the size of your virtual memory
 *  paging file.  During this process, memory requests for some
 *  applications may be denied. For more information, see Help.
 */
#define STATUS_COMMITMENT_MINIMUM        ((NTSTATUS)0xC00002C8L)

/* This is the exception code microsoft uses for throw exception */
#define EXCEPTION_THROWN 0xe06d7363

/* Status to use with NtIsProcessInJob() on Win2003+ */
/*  The specified process is not part of a job */
#define STATUS_PROCESS_NOT_IN_JOB        ((NTSTATUS)0x00000123L)
/*  The specified process is part of a job. */
#define STATUS_PROCESS_IN_JOB            ((NTSTATUS)0x00000124L)

/* A specified privilege does not exist. */
#define STATUS_NO_SUCH_PRIVILEGE         ((NTSTATUS)0xC0000060L)
/*  A required privilege is not held by the client. */
#define STATUS_PRIVILEGE_NOT_HELD        ((NTSTATUS)0xC0000061L)

/* Case 10579: pop but do not transfer control for NtCallbackReturn */
#define STATUS_CALLBACK_POP_STACK        ((NTSTATUS)0xC0000423L)

/* needed for PR 233191 */
#define STATUS_INVALID_INFO_CLASS        ((NTSTATUS)0xC0000003L)

/* This is in VS2005 winnt.h but not in SDK winnt.h */
#ifndef IMAGE_SIZEOF_BASE_RELOCATION
# define IMAGE_SIZEOF_BASE_RELOCATION         8
#endif

NTSTATUS
nt_create_file(OUT HANDLE *file_handle, 
               const wchar_t *filename, HANDLE dir_handle OPTIONAL,
               size_t alloc_size, ACCESS_MASK rights, uint attributes, uint sharing,
               uint create_disposition, uint create_options);

/* is_dir and synch are really bools */
/* note synch must be true if you are ever going to use read_file or write_file
 * on the returned handle */
HANDLE
create_file(PCWSTR filename, int is_dir, ACCESS_MASK rights, uint sharing,
            uint create_disposition, int synch);

int
delete_file(PCWSTR filename);

int
flush_file_buffers(HANDLE file_handle);

int
read_file(HANDLE file_handle, void *buffer, uint num_bytes_to_read, 
          IN uint64* file_byte_offset OPTIONAL,
          OUT size_t *num_bytes_read);
int
write_file(HANDLE file_handle, const void *buffer, uint num_bytes_to_write, 
           OPTIONAL uint64* file_byte_offset,
           OUT size_t *num_bytes_written);

int
close_file(HANDLE hfile);


/* from DDK2003SP1/3790.1830/inc/ddk/wnet/ntddk.h */
/* Define the file information class values */
typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation,   // 2
    FileBothDirectoryInformation,   // 3
    FileBasicInformation,           // 4  wdm
    FileStandardInformation,        // 5  wdm
    FileInternalInformation,        // 6
    FileEaInformation,              // 7
    FileAccessInformation,          // 8
    FileNameInformation,            // 9
    FileRenameInformation,          // 10
    FileLinkInformation,            // 11
    FileNamesInformation,           // 12
    FileDispositionInformation,     // 13
    FilePositionInformation,        // 14 wdm
    FileFullEaInformation,          // 15
    FileModeInformation,            // 16
    FileAlignmentInformation,       // 17
    FileAllInformation,             // 18
    FileAllocationInformation,      // 19
    FileEndOfFileInformation,       // 20 wdm
    FileAlternateNameInformation,   // 21
    FileStreamInformation,          // 22
    FilePipeInformation,            // 23
    FilePipeLocalInformation,       // 24
    FilePipeRemoteInformation,      // 25
    FileMailslotQueryInformation,   // 26
    FileMailslotSetInformation,     // 27
    FileCompressionInformation,     // 28
    FileObjectIdInformation,        // 29
    FileCompletionInformation,      // 30
    FileMoveClusterInformation,     // 31
    FileQuotaInformation,           // 32
    FileReparsePointInformation,    // 33
    FileNetworkOpenInformation,     // 34
    FileAttributeTagInformation,    // 35
    FileTrackingInformation,        // 36
    FileIdBothDirectoryInformation, // 37
    FileIdFullDirectoryInformation, // 38
    /* the following types introduced in XP and later */
    FileValidDataLengthInformation, // 39
    FileShortNameInformation,       // 40
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

/* Information class structures returned by NtQueryInformationFile */
typedef struct _FILE_BASIC_INFORMATION {                    
    LARGE_INTEGER CreationTime;                             
    LARGE_INTEGER LastAccessTime;                           
    LARGE_INTEGER LastWriteTime;                            
    LARGE_INTEGER ChangeTime;                               
    ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;         
                                                            
typedef struct _FILE_STANDARD_INFORMATION {                 
    LARGE_INTEGER AllocationSize;                           
    LARGE_INTEGER EndOfFile;                                
    ULONG NumberOfLinks;                                    
    BOOLEAN DeletePending;                                  
    BOOLEAN Directory;                                      
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;   
                                                            
typedef struct _FILE_POSITION_INFORMATION {                 
    LARGE_INTEGER CurrentByteOffset;                        
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;   
                                                            
typedef struct _FILE_ALIGNMENT_INFORMATION {                
    ULONG AlignmentRequirement;                             
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION; 
                                                            
#define MAX_FILE_NAME_LENGTH MAX_PATH
typedef struct _FILE_NAME_INFORMATION {                     
    ULONG FileNameLength;       /* length in _bytes_ */
    WCHAR FileName[MAX_FILE_NAME_LENGTH];
    /* NOTE that the official structure is FileName[1] so that
     * callers control the length they are interested in */
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;
                                                            
typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[MAX_FILE_NAME_LENGTH];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {               
    ULONG FileAttributes;                                       
    ULONG ReparseTag;                                           
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;  
                                                                
typedef struct _FILE_DISPOSITION_INFORMATION {                  
    BOOLEAN DeleteFile;                                         
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION; 
                                                                
typedef struct _FILE_END_OF_FILE_INFORMATION {                  
    LARGE_INTEGER EndOfFile;                                    
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION; 
                                                                
typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {                                    
    LARGE_INTEGER ValidDataLength;                                                      
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;             

/* FileSystem types for nt_query_volume_info */
/* should be available in ntifs.h from IFS.
 * This version is from reactos/0.2.9/include/ndk/iotypes.h 
 */
typedef enum _FS_INFORMATION_CLASS {
    FileFsVolumeInformation=1,
    FileFsLabelInformation,     /* not documented in IFS */
    FileFsSizeInformation,
    FileFsDeviceInformation,
    FileFsAttributeInformation,
    FileFsControlInformation,
    FileFsFullSizeInformation,
    FileFsObjectIdInformation,
    FileFsDriverPathInformation, /* in reactos */
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef struct _FILE_FS_FULL_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER CallerAvailableAllocationUnits;
    LARGE_INTEGER ActualAvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_FULL_SIZE_INFORMATION, *PFILE_FS_FULL_SIZE_INFORMATION;


/* Event types and functions */
typedef enum _EVENT_TYPE {
    NotificationEvent,   /* manual-reset event - used for broadcasting to multiple waiting threads */
    SynchronizationEvent /* automatically changes state to non-signaled after releasing a waiting thread */
} EVENT_TYPE, *PEVENT_TYPE;

/* event synchronization, using both automatic and manual events */
HANDLE
nt_create_event(EVENT_TYPE event_type);
void
nt_close_event(HANDLE hevent);

typedef enum {WAIT_TIMEDOUT, WAIT_SIGNALED, WAIT_ERROR} wait_status_t;
#define INFINITE_WAIT ((PLARGE_INTEGER)NULL)
wait_status_t
nt_wait_event_with_timeout(HANDLE hevent, PLARGE_INTEGER timeout);
void
nt_set_event(HANDLE hevent);
/* should be used only for manual events (NotificationEvent) */
void
nt_clear_event(HANDLE hevent);
void
nt_signal_and_wait(HANDLE hevent_to_signal, HANDLE hevent_to_wait);

HANDLE
create_iocompletion(void);

HANDLE
open_pipe(PCWSTR pipename, HANDLE hsync);

size_t 
nt_pipe_transceive(HANDLE hpipe, void *input, uint input_size,
                   void *output, uint output_size, uint timeout_ms);

#define TIMER_UNITS_PER_MILLISECOND (1000 * 10) /* 100ns intervals */

/* uint query_time_seconds() declared in os_shared.h */
/* uint64 query_time_millis() declared in os_shared.h */

void
query_system_time(SYSTEMTIME *st);

void
nt_query_performance_counter(PLARGE_INTEGER counter, PLARGE_INTEGER frequency);

#ifdef WINDOWS_PC_SAMPLE
HANDLE
nt_create_profile(HANDLE process_handle, void  *start, uint size, uint *buffer, 
               uint buffer_size, uint shift);

void
nt_set_profile_interval(uint nanoseconds);

int
nt_query_profile_interval(void);

void
nt_start_profile(HANDLE profile_handle);

void
nt_stop_profile(HANDLE profile_handle);
#endif

/* avoid needing x86_code.c from x86.asm from get_own_context_helper() */
#if !defined(NOT_DYNAMORIO_CORE) && !defined(NOT_DYNAMORIO_CORE_PROPER)
/* Executable name must be in kernel object name form
 * (e.g., \SystemRoot\System32\notepad.exe, or \??\c:\foo\bar.exe)
 * The executable name on the command line can be in any form.
 * On success returns a handle for the child
 * On failure returns INVALID_HANDLE_VALUE
 */
HANDLE
create_process(wchar_t *exe, wchar_t *cmdline);

/* NOTE see important usage information in ntdll.c, threads created with this
 * function can NOT return from their start routine */
HANDLE
create_thread(HANDLE hProcess, bool target_64bit, void *start_addr,
              void *arg, const void *arg_buf, size_t arg_buf_size,
              uint stack_reserve, uint stack_commit, bool suspended, thread_id_t *tid);
HANDLE
create_thread_have_stack(HANDLE hProcess, bool target_64bit, void *start_addr,
                         void *arg, const void *arg_buf, size_t arg_buf_size,
                         byte *stack_base, size_t stack_size,
                         bool suspended, thread_id_t *tid);

/* NOTE : this isn't equivalent to nt_get_context(NT_CURRENT_THREAD, cxt)
 * (where the returned context is undefined) so use this to get the context
 * of the current thread */
/* FIXME : no support for float/mmx/sse/debug, 
 *         also if integer or control does both
 * Xref PR 264138 where we have to preserve xmm registers: however, no
 * current uses need to get our own xmm registers, so we don't.  
 * PR 266070: If any future use uses this context to either set a
 * dr_mcontext_t or passes it to nt_set_context() we'll have to add
 * the xmm regs.
 */
/* NOTE : we use a macro so that we get the register state of the calling
 * function (we don't want ebp/esp to point to a leaf routine's stack frame) */
#define GET_OWN_CONTEXT(cxt) {                                          \
    if (TESTANY(CONTEXT_CONTROL|CONTEXT_INTEGER, (cxt)->ContextFlags)) {\
        byte *__get_own_context_cur_esp;                                \
        get_own_context_helper((cxt));                                  \
        GET_STACK_PTR(__get_own_context_cur_esp);                       \
        (cxt)->CXT_XSP = (reg_t) __get_own_context_cur_esp;             \
    }                                                                   \
    get_own_context((cxt));                                             \
}

/* unstatic for use by GET_OWN_CONTEXT macro */
void
get_own_context_integer_control(CONTEXT *cxt, reg_t cs, reg_t ss,
                                dr_mcontext_t *mc);

/* don't call this directly, use GET_OWN_CONTEXT macro instead (it fills
 * in CONTEXT_INTEGER and CONTEXT_CONTROL values, this fills the rest) */
void
get_own_context(CONTEXT *cxt);
#endif /* !defined(NOT_DYNAMORIO_CORE) && !defined(NOT_DYNAMORIO_CORE_PROPER) */


/***************************************************/
/* in module_shared.c */

enum { /* can't put w/ os_exports.h enum b/c needed for non-core */
    /* for accessing x64 data from WOW64 */
    X64_PEB_TIB_OFFSET        = 0x060,
    X64_LDR_PEB_OFFSET        = 0x018,
};

LDR_MODULE *
get_ldr_module_by_name(wchar_t *name);;

#ifndef X64
HANDLE
get_module_handle_64(wchar_t *name);

void *
get_proc_address_64(HANDLE lib, const char *name);
#endif

IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory(app_pc base_addr, size_t *exports_size /* OPTIONAL OUT */);

IMAGE_EXPORT_DIRECTORY*
get_module_exports_directory_check(app_pc base_addr,
                                   size_t *exports_size /*OPTIONAL OUT*/,
                                   bool check_names);

/***************************************************/


/* Kernel32 uses HMODULE type for Load/FreeLibrary and GetModuleHandle. This
 * is, at least in the platforms we've seen so far, just a pointer to the base
 * address of the dll. We use a separate type here to keep things neat. */
/* FIXME - this duplicates the typedef in os_private.h */
typedef void * module_handle_t;

module_handle_t
load_library(wchar_t *lib_name);

int
free_library(module_handle_t lib);

module_handle_t
get_module_handle(wchar_t *lib_name);

/* from ntddk.h */
typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

NTSTATUS
nt_map_view_of_section(IN HANDLE           SectionHandle,
                       IN HANDLE           ProcessHandle,
                       IN OUT PVOID       *BaseAddress,
                       IN ULONG            ZeroBits,
                       IN SIZE_T           CommitSize,
                       IN OUT PLARGE_INTEGER  SectionOffset OPTIONAL,
                       IN OUT PSIZE_T      ViewSize,
                       IN SECTION_INHERIT  InheritDisposition,
                       IN ULONG            AllocationType,
                       IN ULONG            Protect
                       );
NTSTATUS
nt_unmap_view_of_section(IN HANDLE         ProcessHandle,
                         IN PVOID          BaseAddress
                         );

/* From WINNT.H for .NET 2.0 (Visual Studio.NET VC7)
 * Needed for IMAGE_COR20_HEADER
 */
#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum replaces_cor_hdr_numeric_defines_t
{
// COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

// Version flags for image.
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

// Maximum size of a NativeType descriptor.
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

// #defines for the MIH FLAGS
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

// V-table constants
    COR_VTABLE_32BIT                    =0x01,          // V-table slots are 32-bits in size.   
    COR_VTABLE_64BIT                    =0x02,          // V-table slots are 64-bits in size.   
    COR_VTABLE_FROM_UNMANAGED           =0x04,          // If set, transition from unmanaged.
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,          // Call most derived method described by

// EATJ constants
    IMAGE_COR_EATJ_THUNK_SIZE           =32,            // Size of a jump thunk reserved range.

// Max name lengths    
    //@todo: Change to unlimited name lengths.
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} replaces_cor_hdr_numeric_defines_t;

// COM+ 2.0 header structure.
typedef struct IMAGE_COR20_HEADER
{
    // Header versioning
    DWORD                   cb;              
    WORD                    MajorRuntimeVersion;
    WORD                    MinorRuntimeVersion;
    
    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY    MetaData;        
    DWORD                   Flags;           
    DWORD                   EntryPointToken;
    
    // Binding information
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

    // Regular fixup and binding information
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

    // Precompiled image info (internal use only - set to zero)
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif // __IMAGE_COR20_HEADER_DEFINED__


#ifndef IMAGE_SCN_ALIGN_MASK
#define IMAGE_SCN_ALIGN_MASK 0x00F00000 /* from latest winnt.h */
#endif /* IMAGE_SCN_ALIGN_MASK */

NTSTATUS
nt_initialize_shared_directory(HANDLE *shared_directory /* OUT */, bool permanent);

NTSTATUS
nt_open_object_directory(HANDLE *shared_directory /* OUT */, 
                         PCWSTR object_directory_name,
                         bool allow_creation);

void
nt_close_object_directory(HANDLE hobjdir);

NTSTATUS
nt_get_symlink_target(IN HANDLE              directory_handle,
                      IN PCWSTR              symlink_name,
                      IN OUT UNICODE_STRING* target_name,
                      OUT uint *             returned_byte_length);

#define MAX_OBJECT_NAME_LENGTH MAX_PATH
typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING ObjectName;
    /* note that the nt interface allows callers to specify any size,
     * yet we do not expect needs for longer */
    wchar_t object_name_buffer[MAX_OBJECT_NAME_LENGTH];
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

NTSTATUS
nt_get_object_name(HANDLE handle, OBJECT_NAME_INFORMATION* object_name /* OUT */,
                   uint byte_length, uint *returned_byte_length);

NTSTATUS
nt_create_section(OUT PHANDLE SectionHandle,
                  IN ACCESS_MASK DesiredAccess,
                  IN PLARGE_INTEGER SectionSize OPTIONAL,
                  IN ULONG Protect,
                  IN ULONG section_creation_attributes,
                  IN HANDLE FileHandle,
                  /* object name attributes */
                  IN PCWSTR new_section_name,
                  IN ULONG object_name_attributes,
                  IN HANDLE object_directory,
                  IN PSECURITY_DESCRIPTOR dacl
                  );
NTSTATUS
nt_open_section(OUT PHANDLE SectionHandle,
                IN ACCESS_MASK DesiredAccess,
                /* object name attributes */
                IN PCWSTR section_name,
                IN ULONG object_name_attributes,
                IN HANDLE object_directory);

NTSTATUS
nt_create_module_file(OUT HANDLE* file_handle,
                      const wchar_t *file_path, 
                      IN HANDLE root_directory_handle OPTIONAL,
                      ACCESS_MASK desired_access_rights,
                      uint file_special_attributes,
                      uint file_sharing_flags,
                      uint create_disposition, 
                      size_t allocation_size);

NTSTATUS
nt_query_file_info(IN HANDLE FileHandle,
                   OUT PVOID FileInformation,
                   IN ULONG FileInformationLength,
                   IN FILE_INFORMATION_CLASS FileInformationClass);
NTSTATUS
nt_set_file_info(IN HANDLE FileHandle,
                 IN PVOID FileInformation,
                 IN ULONG FileInformationLength,
                 IN FILE_INFORMATION_CLASS FileInformationClass);

NTSTATUS
nt_query_volume_info(IN HANDLE FileHandle,
                     OUT PVOID FsInformation,
                     IN ULONG FsInformationLength,
                     IN FS_INFORMATION_CLASS FsInformationClass);
NTSTATUS
nt_query_security_object(IN HANDLE Handle,
                         IN SECURITY_INFORMATION RequestedInformation,
                         OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
                         IN ULONG SecurityDescriptorLength,
                         OUT PULONG ReturnLength
                         );


#endif /* _NTDLL_H_ */
