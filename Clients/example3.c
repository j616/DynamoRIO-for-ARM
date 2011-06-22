// Adds the dynamic counts of blocks using a clean call.
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
void *count_lock;
// End of code added for Example 3

static void event_exit(void);

static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating);

// Code added for Example 3
static void clean_call(uint instruction_count);
// End of code added for Example 3.

DR_EXPORT void dr_init(client_id_t id)
{
  /* Register Events */
  dr_register_exit_event(event_exit);
  dr_register_bb_event(event_basic_block);

  /* Initalize lock */
  as_built_lock = dr_mutex_create();

  // Code added for Example 3
  count_lock = dr_mutex_create();
  // End of code added for Example 3
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

  // Code added for Example 3
  dr_mutex_destroy(count_lock);
  // End of code added for Example 3
}

// Code added for Example 3
static void clean_call(uint instruction_count)
{
  dr_mutex_lock(count_lock);
  counts_dynamic.blocks++;
  counts_dynamic.total_size += instruction_count;
  dr_mutex_unlock(count_lock);
}
// End of code added for Example 3

static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating)
{
  uint num_instructions = 0;
  instr_t *instr;

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

  // Code added for Example 3
  // Insert clean call
  dr_insert_clean_call(drcontext, bb, instrlist_first(bb), clean_call, false, 1, OPND_CREATE_INT32(num_instructions));
  // End of code added for Example 3

  return DR_EMIT_DEFAULT;
}
