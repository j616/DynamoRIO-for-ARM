#include "../globals.h"
#include "proc.h"
#include "instr.h" /* for dr_insert_{save,restore}_fpstate */
#include "instrument.h" /* for dr_insert_{save,restore}_fpstate */
#include "instr_create.h" /* for dr_insert_{save,restore}_fpstate */
#include "decode.h" /* for dr_insert_{save,restore}_fpstate */

static uint vendor   = VENDOR_UNKNOWN;
static uint L1_icache_size = CACHE_SIZE_UNKNOWN;
static uint L1_dcache_size = CACHE_SIZE_UNKNOWN;
static uint L2_cache_size  = CACHE_SIZE_UNKNOWN;
size_t cache_line_size = 32;
static ptr_uint_t mask; /* bits that should be 0 to be cache-line-aligned */

const char *
proc_get_cache_size_str(cache_size_t size)
{
	// COMPLETEDD #255 proc_get_cache_size_str
	printf("Starting proc_get_cache_size_str\n");
    static const char *strings[] = {
        "8 KB",
        "16 KB",
        "32 KB",
        "64 KB",
        "128 KB",
        "256 KB",
        "512 KB",
        "1 MB",
        "2 MB",
        "unknown"
    };
    CLIENT_ASSERT(size <= CACHE_SIZE_UNKNOWN, "proc_get_cache_size_str: invalid size");
    return strings[size];
}

cache_size_t
proc_get_L1_icache_size(void)
{
	// COMPLETEDD #256 proc_get_L1_icache_size
	printf("Starting proc_get_L1_icache_size\n");
    return L1_icache_size;
}

uint
proc_get_vendor(void)
{
	// COMPLETEDD #549 proc_get_vendor
    return vendor;
}

cache_size_t
proc_get_L1_dcache_size(void)
{
	// COMPLETEDD #257 proc_get_L1_dcache_size
	printf("Starting proc_get_L1_dcache_size\n");
    return L1_dcache_size;
}

cache_size_t
proc_get_L2_cache_size(void)
{
	// COMPLETEDD #258 proc_get_L2_cache_size
	printf("Starting proc_get_L2_cache_size\n");
    return L2_cache_size;
}

#ifndef ARM
/* Intel processors:  ebx:edx:ecx spell GenuineIntel */
#define INTEL_EBX /* Genu */ 0x756e6547
#define INTEL_EDX /* ineI */ 0x49656e69
#define INTEL_ECX /* ntel */ 0x6c65746e

/* AMD processors:  ebx:edx:ecx spell AuthenticAMD */
#define AMD_EBX /* Auth */ 0x68747541
#define AMD_EDX /* enti */ 0x69746e65
#define AMD_ECX /* cAMD */ 0x444d4163

/* The brand string is a 48-character, null terminated string.
 * Declare as a 12-element uint so the compiler won't complain
 * when we store GPRs to it.  Initialization is "unknown" .
 */
static uint brand_string[12] = {0x6e6b6e75, 0x006e776f};

static uint family   = 0;
static uint type     = 0;
static uint model    = 0;
static uint stepping = 0;

/* Feature bits in 4 32-bit values: features in edx,
 * features in ecx, extended features in edx, and
 * extended features in ecx
 */
static features_t features = {0, 0, 0, 0};

static void
set_cache_size(uint val, uint *dst)
{
	// COOMPLETEDD #254 set_cache_size
    CLIENT_ASSERT(dst != NULL, "invalid internal param");
    switch (val) {
        case 8:    *dst = CACHE_SIZE_8_KB;   break;
        case 16:   *dst = CACHE_SIZE_16_KB;  break;
        case 32:   *dst = CACHE_SIZE_32_KB;  break;
        case 64:   *dst = CACHE_SIZE_64_KB;  break;
        case 128:  *dst = CACHE_SIZE_128_KB; break;
        case 256:  *dst = CACHE_SIZE_256_KB; break;
        case 512:  *dst = CACHE_SIZE_512_KB; break;
        case 1024: *dst = CACHE_SIZE_1_MB;   break;
        case 2048: *dst = CACHE_SIZE_2_MB;   break;
        default: SYSLOG_INTERNAL_ERROR("Unknown processor cache size"); break;
    }
}

static void
get_cache_sizes_amd(uint max_ext_val)
{
	// COMPLETEDD #253 get_cache_sizes_amd
    uint cpuid_res_local[4]; /* eax, ebx, ecx, and edx registers (in that order) */

    if (max_ext_val >= 0x80000005) {
#ifdef LINUX
        our_cpuid((int*)cpuid_res_local, 0x80000005);
#else
        __cpuid(cpuid_res_local, 0x80000005);
#endif
        set_cache_size((cpuid_res_local[2]/*ecx*/ >> 24), &L1_icache_size);
        set_cache_size((cpuid_res_local[3]/*edx*/ >> 24), &L1_dcache_size);
    }

    if (max_ext_val >= 0x80000006) {
#ifdef LINUX
        our_cpuid((int*)cpuid_res_local, 0x80000006);
#else
        __cpuid(cpuid_res_local, 0x80000006);
#endif
        set_cache_size((cpuid_res_local[2]/*ecx*/ >> 16), &L2_cache_size);
    }
}

static void
get_cache_sizes_intel(uint max_val)
{
	// COMPLETEDD #252 get_cache_sizes_intel
    /* declare as uint so compiler won't complain when we write GP regs to the array */
    uint cache_codes[4];
    int i;

    if (max_val < 2)
        return;

#ifdef LINUX
    our_cpuid((int*)cache_codes, 2);
#else
    __cpuid(cache_codes, 2);
#endif
    /* The lower 8 bits of eax specify the number of times cpuid
     * must be executed to obtain a complete picture of the cache
     * characteristics.
     */
    CLIENT_ASSERT((cache_codes[0] & 0xff) == 1, "cpuid error");
    cache_codes[0] &= ~0xff;

    /* Cache codes are stored in consecutive bytes in the
     * GP registers.  For each register, a 1 in bit 31
     * indicates that the codes should be ignored... zero
     * all four bytes when that happens
     */
    for (i=0; i<4; i++) {
        if (cache_codes[i] & 0x80000000)
            cache_codes[i] = 0;
    }

    /* Table 3-17, pg 3-171 of IA-32 instruction set reference lists
     * all codes.  Omitting L3 cache characteristics for now...
     */
    for (i=0; i<16; i++) {
        switch (((uchar*)cache_codes)[i]) {
            case 0x06: L1_icache_size = CACHE_SIZE_8_KB; break;
            case 0x08: L1_icache_size = CACHE_SIZE_16_KB; break;
            case 0x0a: L1_dcache_size = CACHE_SIZE_8_KB; break;
            case 0x0c: L1_dcache_size = CACHE_SIZE_16_KB; break;
            case 0x2c: L1_dcache_size = CACHE_SIZE_32_KB; break;
            case 0x30: L1_icache_size = CACHE_SIZE_32_KB; break;
            case 0x41: L2_cache_size = CACHE_SIZE_128_KB; break;
            case 0x42: L2_cache_size = CACHE_SIZE_256_KB; break;
            case 0x43: L2_cache_size = CACHE_SIZE_512_KB; break;
            case 0x44: L2_cache_size = CACHE_SIZE_1_MB; break;
            case 0x45: L2_cache_size = CACHE_SIZE_2_MB; break;
            case 0x60: L1_dcache_size = CACHE_SIZE_16_KB; break;
            case 0x66: L1_dcache_size = CACHE_SIZE_8_KB; break;
            case 0x67: L1_dcache_size = CACHE_SIZE_16_KB; break;
            case 0x68: L1_dcache_size = CACHE_SIZE_32_KB; break;
            case 0x78: L2_cache_size = CACHE_SIZE_1_MB; break;
            case 0x79: L2_cache_size = CACHE_SIZE_128_KB; break;
            case 0x7a: L2_cache_size = CACHE_SIZE_256_KB; break;
            case 0x7b: L2_cache_size = CACHE_SIZE_512_KB; break;
            case 0x7c: L2_cache_size = CACHE_SIZE_1_MB; break;
            case 0x7d: L2_cache_size = CACHE_SIZE_2_MB; break;
            case 0x7f: L2_cache_size = CACHE_SIZE_512_KB; break;
            case 0x82: L2_cache_size = CACHE_SIZE_256_KB; break;
            case 0x83: L2_cache_size = CACHE_SIZE_512_KB; break;
            case 0x84: L2_cache_size = CACHE_SIZE_1_MB; break;
            case 0x85: L2_cache_size = CACHE_SIZE_2_MB; break;
            case 0x86: L2_cache_size = CACHE_SIZE_512_KB; break;
            case 0x87: L2_cache_size = CACHE_SIZE_1_MB; break;
            default: break;
        }
    }
}
#endif

void
proc_init(void)
{
	// COMPLETEDD #259 proc_init
  printf("Starting proc_int\n");
  LOG(GLOBAL, LOG_TOP, 1, "Running on a %d CPU machine\n", get_num_processors());

  get_processor_specific_info();
  printf("Returning from get_processor_specific_info\n");
  CLIENT_ASSERT(cache_line_size > 0, "invalid cache line size");
  mask = (cache_line_size - 1);

  LOG(GLOBAL, LOG_TOP, 1, "Cache line size is %d bytes\n", cache_line_size);
  LOG(GLOBAL, LOG_TOP, 1, "L1 icache=%s, L1 dcache=%s, L2 cache=%s\n",
      proc_get_cache_size_str(proc_get_L1_icache_size()),
      proc_get_cache_size_str(proc_get_L1_dcache_size()),
      proc_get_cache_size_str(proc_get_L2_cache_size()));
  LOG(GLOBAL, LOG_TOP, 1, "Processor brand string = %s\n", brand_string);
  LOG(GLOBAL, LOG_TOP, 1, "Type=0x%x, Family=0x%x, Model=0x%x, Stepping=0x%x\n",
      type, family, model, stepping);

#ifdef X64
  CLIENT_ASSERT(proc_has_feature(FEATURE_LAHF),
                "Unsupported processor type - processor must support LAHF/SAHF in "
                "64bit mode.");
  if (!proc_has_feature(FEATURE_LAHF)) {
      FATAL_USAGE_ERROR(UNSUPPORTED_PROCESSOR_LAHF, 2,
                        get_application_name(), get_application_pid());
  }
#endif

#ifdef DEBUG
  /* FIXME: This is a small subset of processor features.  If we
   * care enough to add more, it would probably be best to loop
   * through a const array of feature names.
  */
  if (stats->loglevel > 0 && (stats->logmask & LOG_TOP) != 0) {
      if (proc_has_feature(FEATURE_XD_Bit))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has XD Bit\n");
      if (proc_has_feature(FEATURE_MMX))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has MMX\n");
      if (proc_has_feature(FEATURE_FXSR))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has fxsave/fxrstor\n");
      if (proc_has_feature(FEATURE_SSE))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has SSE\n");
      if (proc_has_feature(FEATURE_SSE2))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has SSE2\n");
      if (proc_has_feature(FEATURE_SSE3))
          LOG(GLOBAL, LOG_TOP, 1, "\tProcessor has SSE3\n");
  }
#endif
  /* PR 264138: for 32-bit CONTEXT we assume fxsave layout */
#ifndef ARM
  CLIENT_ASSERT((proc_has_feature(FEATURE_FXSR) && proc_has_feature(FEATURE_SSE)) ||
                (!proc_has_feature(FEATURE_FXSR) && !proc_has_feature(FEATURE_SSE)),
                "Unsupported processor type: SSE and FXSR must match");
#endif
}

/* check to see if addr is cache aligned */
bool
proc_is_cache_aligned(void *addr)
{
	// COMPLETEDD #307 proc_is_cache_aligned
	printf("Starting proc_is_cache_aligned\n");
    return (((ptr_uint_t)addr & mask) == 0x0);
}

/*
 * Due to the fact that the way to find out processor information in the ARM processor
 * has to be done in privilaged mode I have just hard coded a Cortex A8 Processor in
 * for now.
 */
static void
get_processor_specific_info(void)
{
	// COMPLETEDD #251 get_processor_specific_info
	printf("Starting get_processor_specific_info\n");
#ifdef ARM
    vendor = VENDOR_ARM;
    cache_line_size = 64;
    L1_icache_size = CACHE_SIZE_16_KB;
    L1_dcache_size = CACHE_SIZE_16_KB;
    L2_cache_size = CACHE_SIZE_256_KB;
#else
    /* use cpuid instruction to get processor info.  For details, see
     * http://download.intel.com/design/Xeon/applnots/24161830.pdf
     * "AP-485: Intel Processor Identification and the CPUID
     * instruction", 96 pages, January 2006
     */
    uint res_eax, res_ebx = 0, res_ecx = 0, res_edx = 0;
    uint max_val, max_ext_val;
    int cpuid_res_local[4]; /* eax, ebx, ecx, and edx registers (in that order) */

    /* First check for existence of the cpuid instruction
     * by attempting to modify bit 21 of eflags
     */
    /* FIXME: Perhaps we should abort when the cpuid instruction
     * doesn't exist since the cache_line_size may be incorrect.
     * (see case 463 for discussion)
    */
    if (!cpuid_supported()) {
        ASSERT_CURIOSITY(false && "cpuid instruction unsupported");
        SYSLOG_INTERNAL_WARNING("cpuid instruction unsupported -- cache_line_size "
                                "may be incorrect");
        return;
    }
    printf("Getting to this point\n");

    /* first verify on Intel processor */
#ifdef LINUX
    our_cpuid(cpuid_res_local, 0);
#else
    __cpuid(cpuid_res_local, 0);
#endif
    printf("Getting to this point 2\n");
    fflush(stdout);
    res_eax = cpuid_res_local[0];
    res_ebx = cpuid_res_local[1];
    res_ecx = cpuid_res_local[2];
    res_edx = cpuid_res_local[3];
    max_val = res_eax;

    if (res_ebx == INTEL_EBX) {
        vendor = VENDOR_INTEL;
        CLIENT_ASSERT(res_edx == INTEL_EDX && res_ecx == INTEL_ECX,
                      "unknown Intel processor type");
    } else if (res_ebx == AMD_EBX) {
        vendor = VENDOR_AMD;
        printf("Vendor is AMD\n");
        CLIENT_ASSERT(res_edx == AMD_EDX && res_ecx == AMD_ECX,
                      "unknown AMD processor type");
    } else {
        vendor = VENDOR_UNKNOWN;
        SYSLOG_INTERNAL_ERROR("Running on unknown processor type");
        LOG(GLOBAL, LOG_TOP, 1, "cpuid returned "PFX" "PFX" "PFX" "PFX"\n",
            res_eax, res_ebx, res_ecx, res_edx);
    }

    /* Try to get extended cpuid information */
#ifdef LINUX
    our_cpuid(cpuid_res_local, 0x80000000);
#else
    __cpuid(cpuid_res_local, 0x80000000);
#endif
    max_ext_val = cpuid_res_local[0]/*eax*/;

    /* Extended feature flags */
    if (max_ext_val >= 0x80000001) {
#ifdef LINUX
        our_cpuid(cpuid_res_local, 0x80000001);
#else
        __cpuid(cpuid_res_local, 0x80000001);
#endif
        res_ecx = cpuid_res_local[2];
        res_edx = cpuid_res_local[3];
        features.ext_flags_edx = res_edx;
        features.ext_flags_ecx = res_ecx;
    }

    /* now get processor info */
#ifdef LINUX
    our_cpuid(cpuid_res_local, 1);
#else
    __cpuid(cpuid_res_local, 1);
#endif
    res_eax = cpuid_res_local[0];
    res_ebx = cpuid_res_local[1];
    res_ecx = cpuid_res_local[2];
    res_edx = cpuid_res_local[3];
    /* eax contains basic info:
     *   extended family, extended model, type, family, model, stepping id
     *   20:27,           16:19,          12:13, 8:11,  4:7,   0:3
     */
    type   = (res_eax >> 12) & 0x3;
    family = (res_eax >>  8) & 0xf;
    model  = (res_eax >>  4) & 0xf;
    stepping = res_eax & 0xf;

    /* Pages 3-164 and 3-165 of the IA-32 instruction set
     * reference instruct us to adjust the family and model
     * numbers as follows.
     */
    if (family == 0x6 || family == 0xf) {
        uint ext_model = (res_eax >> 16) & 0xf;
        model += (ext_model << 4);

        if (family == 0xf) {
            uint ext_family = (res_eax >> 20) & 0xff;
            family += ext_family;
        }
    }

    features.flags_edx = res_edx;
    features.flags_ecx = res_ecx;

    /* Now features.* are complete and we can query */
    if (proc_has_feature(FEATURE_CLFSH)) {
        /* The new manuals imply ebx always holds the
         * cache line size for clflush, not just on P4
         */
        cache_line_size = (res_ebx & 0x0000ff00) >> 5; /* (x >> 8) * 8 == x >> 5 */
    } else if (vendor == VENDOR_INTEL &&
               (family == FAMILY_PENTIUM_3 || family == FAMILY_PENTIUM_2)) {
        /* Pentium III, Pentium II */
        cache_line_size = 32;
    } else if (vendor == VENDOR_AMD && family == FAMILY_ATHLON) {
        /* Athlon */
        cache_line_size = 64;
#ifdef IA32_ON_IA64
    } else if (vendor == VENDOR_INTEL && family == FAMILY_IA64) {
        /* Itanium */
        cache_line_size = 32;
#endif
    } else {
        LOG(GLOBAL, LOG_TOP, 1, "Warning: running on unsupported processor family %d\n",
            family);
        cache_line_size = 32;
    }
    /* people who use this in ALIGN* macros are assuming it's a power of 2 */
    CLIENT_ASSERT((cache_line_size & (cache_line_size - 1)) == 0,
                  "invalid cache line size");

    /* get L1 and L2 cache sizes */
    if (vendor == VENDOR_AMD)
        get_cache_sizes_amd(max_ext_val);
    else
        get_cache_sizes_intel(max_val);

    /* Processor brand string */
    if (max_ext_val >= 0x80000004) {
#ifdef LINUX
        our_cpuid((int*)&brand_string[0], 0x80000002);
        our_cpuid((int*)&brand_string[4], 0x80000003);
        our_cpuid((int*)&brand_string[8], 0x80000004);
#else
        __cpuid(&brand_string[0], 0x80000002);
        __cpuid(&brand_string[4], 0x80000003);
        __cpuid(&brand_string[8], 0x80000004);
#endif
    }
#endif
}

size_t
proc_get_cache_line_size(void)
{
	// COMPLETEDD #271 proc_get_cache_line_size
	printf("starting proc_get_cache_line_size\n");
    return cache_line_size;
}

/* Given an address or number of bytes sz, return a number >= sz that is divisible
   by the cache line size. */
ptr_uint_t
proc_bump_to_end_of_cache_line(ptr_uint_t sz)
{
	// COMPLETEDD #341 proc_bump_to_end_of_cache_line
	printf("proc_bump_to_end_of_cache_line\n");
  if ((sz & mask) == 0x0)
      return sz;      /* sz already a multiple of the line size */

  return ((sz + cache_line_size) & ~mask);
}

/* No synchronization routines necessary.  The Pentium hardware
 * guarantees that the i and d caches are consistent. */
void
machine_cache_sync(void *pc_start, void *pc_end, bool flush_icache)
{
    /* empty */
}

bool
proc_has_feature(feature_bit_t f)
{
	// COMPLETEDD #254 proc_has_feature
	printf("starting proc_has_feature\n");
#ifdef ARM
	return NO_FEATURE;
#else
  uint bit, val = 0;

  if (f >= 0 && f <= 31) {
      val = features.flags_edx;
  } else if (f >= 32 && f <= 63) {
      val = features.flags_ecx;
  } else if (f >= 64 && f <= 95) {
      val = features.ext_flags_edx;
  } else if (f >= 96 && f <= 127) {
      val = features.ext_flags_ecx;
  } else {
      CLIENT_ASSERT(false, "proc_has_feature: invalid parameter");
  }

  bit = f % 32;
  return TEST((1 << bit), val);
#endif
}
