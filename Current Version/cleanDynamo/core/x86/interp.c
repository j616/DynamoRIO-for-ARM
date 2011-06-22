#include "../globals.h"
#include "../link.h"
#include "../fragment.h"
#include "../emit.h"
#include "../dispatch.h"
#include "../instrlist.h"
#include "../fcache.h"
#include "../monitor.h" /* for trace_abort and monitor_data_t */
#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "decode_fast.h"
#include "disassemble.h"
#include <string.h> /* for memcpy */
#include "instrument.h"
#include "../hotpatch.h"
#include "../perscache.h"
/* Just like decode_fragment() but marks any instrs missing in the cache
 * as do-not-emit
 */
instrlist_t *
decode_fragment_exact(dcontext_t *dcontext, fragment_t *f, byte *buf,
                      /*IN/OUT*/uint *bufsz, uint target_flags,
                      /*OUT*/uint *dir_exits, /*OUT*/uint *indir_exits)
{
	return 0;
}

instrlist_t *
build_app_bb_ilist(dcontext_t *dcontext, byte *start_pc, file_t outf)
{
	return 0;
}

/* Used when the code cache is enlarged by copying to a larger space,
 * and all of the relative ctis that target outside the cache need
 * to be shifted. Additionally, sysenter-related patching for ignore-syscalls
 * on XP/2003 is performed here, as the absolute code cache address pushed
 * onto the stack must be updated.
 * Assumption: old code cache has been copied to TOP of new cache, so to
 * detect for ctis targeting outside of old cache can look at new cache
 * start plus old cache size.
 */
void
shift_ctis_in_fragment(dcontext_t *dcontext, fragment_t *f, ssize_t shift,
                       cache_pc fcache_start, cache_pc fcache_end,
                       size_t old_size)
{
}

bool
app_bb_overlaps(dcontext_t *dcontext, byte *start_pc, uint flags,
                byte *region_start, byte *region_end, overlap_info_t *info_res)
{
	return 0;
}

/* Interprets the application's instructions until the end of a basic
 * block is found, and then creates a fragment for the basic block.
 * DOES NOT look in the hashtable to see if such a fragment already exists!
 */
fragment_t *
build_basic_block_fragment(dcontext_t *dcontext, app_pc start, uint initial_flags,
                           bool link, bool visible _IF_CLIENT(bool for_trace)
                           _IF_CLIENT(instrlist_t **unmangled_ilist))
{
  return 0;
}

/* Call when about to throw exception or other drastic action in the
 * middle of bb building, in order to free resources
 */
void
bb_build_abort(dcontext_t *dcontext, bool clean_vmarea)
{
}

app_pc
find_app_bb_end(dcontext_t *dcontext, byte *start_pc, uint flags)
{
	return 0;
}

/* Add a speculative counter on last IBL exit
 * Returns additional size to add to trace estimate.
 */
int
append_trace_speculate_last_ibl(dcontext_t *dcontext, instrlist_t *trace,
                                app_pc speculate_next_tag,
                                bool record_translation)
{
	return 0;
}

/* initialization */
void
interp_init()
{
	// COMPLETEDD #267 interp_init
	printf("Starting interp_init\n");
#ifdef INTERNAL
    if (INTERNAL_OPTION(bbdump_tags)) {
        bbdump_file = open_log_file("bbs", NULL, 0);
        ASSERT(bbdump_file != INVALID_FILE);
    }
#endif
}

/* Add the fragment f to the end of the trace instrlist_t kept in dcontext
 *
 * Note that recreate_fragment_ilist() is making assumptions about its operation
 * synchronize changes
 *
 * Returns the size change in the trace from mangling the previous block
 * (assumes the caller has already calculated the size from adding the new block)
 */
uint
extend_trace(dcontext_t *dcontext, fragment_t *f, linkstub_t *prev_l)
{
	return 0;
}

bool
mangle_trace(dcontext_t *dcontext, instrlist_t *ilist, monitor_data_t *md)
{
	return 0;
}

/* we limit total bb size to handle cases like infinite loop or sequence
 * of calls.
 * also, we have a limit on fragment body sizes, which should be impossible
 * to break since x86 instrs are max 17 bytes and we only modify ctis.
 * Although...selfmod mangling does really expand fragments!
 * -selfmod_max_writes helps for selfmod bbs (case 7893/7909).
 * System call mangling is also large, for degenerate cases like tests/linux/infinite.
 * PR 215217: also client additions: we document and assert.
 * FIXME: need better way to know how big will get, b/c we can construct
 * cases that will trigger the size assertion!
 */
/* define replaced by -max_bb_instrs option */

/* exported so micro routines can assert whether held */
DECLARE_CXTSWPROT_VAR(mutex_t bb_building_lock, INIT_LOCK_FREE(bb_building_lock));
