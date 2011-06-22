// Demonstrates dynamically adding instructions to do the count instead of functions
#include "dr_api.h"

#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)

typedef struct 
{
  uint64 blocks;
  uint64 total_size;
} bb_counts;

static bb_counts counts_as_built;
void *as_built_lock;

// Code added for Example 3
static bb_counts counts_dynamic;
// End of code added for Example 3

static void event_exit(void);

static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating);

DR_EXPORT void dr_init(client_id_t id)
{
  /* Register Events */
  dr_register_exit_event(event_exit);
  dr_register_bb_event(event_basic_block);

  /* Initalize lock */
  as_built_lock = dr_mutex_create();

}

static void event_exit()
{
  char msg[512];
  int len;

  // Code edited for Example 3
  len = snprintf(msg, sizeof(msg) / sizeof(msg[0]), "Number of basic blocks built : %"UINT64_FORMAT_CODE"\n"
  "    Average size        : %5.2lf instructions\n"
  "Number of blocks Executed : %"UINT64_FORMAT_CODE"\n"
  "    Average weighted size: %5.2lf instructions\n",
  counts_as_built.blocks, counts_as_built.total_size / (double)counts_as_built.blocks,
    counts_dynamic.blocks, counts_dynamic.total_size / (double) counts_dynamic.blocks);
  // End of code added for Example 3

  DR_ASSERT(len > 0);
  msg[sizeof(msg)/sizeof(msg[0])-1] = '\0'; /*NULL Terminate */
  DISPLAY_STRING(msg);

  /* Free mutex */
  dr_mutex_destroy(as_built_lock);

}

static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating)
{
  uint num_instructions = 0;

  // Code edited in Example 4
  instr_t *instr, *where = NULL;
  // End of code added for Example 4

  /* Count the number of instructions in this block */
  for(instr = instrlist_first(bb); instr != NULL; instr = instr_get_next(instr))
  {
    num_instructions++;
  }

  /* Update the as-built counts */
  dr_mutex_lock(as_built_lock);
  counts_as_built.blocks++;
  counts_as_built.total_size += num_instructions;
  dr_mutex_unlock(as_built_lock);

  // Code added in Example 4
  where = instrlist_first(bb);
  dr_save_arith_flags(drcontext, bb, where, SPILL_SLOT_1);

  // Rest is X86 32 bit code only
  instrlist_meta_preinsert(bb, where,
    LOCK(INSTR_CREATE_add(drcontext,
      OPND_CREATE_ABSMEM((byte *)&counts_dynamic.blocks, OPSZ_4),
      OPND_CREATE_INT8(1))));
  instrlist_meta_preinsert(bb, where,
    LOCK(INSTR_CREATE_adc(drcontext,
      OPND_CREATE_ABSMEM((byte *)&counts_dynamic.blocks +4, OPSZ_4),
      OPND_CREATE_INT8(0))));

  instrlist_meta_preinsert(bb, where,
    LOCK(INSTR_CREATE_add(drcontext,
      OPND_CREATE_ABSMEM((byte *) &counts_dynamic.total_size, OPSZ_4),
      OPND_CREATE_INT_32OR8(num_instructions))));
  instrlist_meta_preinsert(bb, where,
    LOCK(INSTR_CREATE_adc(drcontext,
      OPND_CREATE_ABSMEM((byte *)&counts_dynamic.total_size +4, OPSZ_4),
      OPND_CREATE_INT8(0))));

  dr_restore_arith_flags(drcontext, bb, where, SPILL_SLOT_1);

  // End of code added in Example 4

  return DR_EMIT_DEFAULT;
}
