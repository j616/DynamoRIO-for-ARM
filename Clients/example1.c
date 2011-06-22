// Basic counting of Blocks Example
#include "dr_api.h"

#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)

typedef struct 
{
  uint64 blocks;
  uint64 total_size;
} bb_counts;

static bb_counts counts_as_built;
void *as_built_lock;

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
  len = snprintf(msg, sizeof(msg) / sizeof(msg[0]), "Number of basi blocks built : %"UINT64_FORMAT_CODE"\n"
  "    Average size        : %5.2lf instructions\n",
  counts_as_built.blocks, counts_as_built.total_size / (double)counts_as_built.blocks);
  DR_ASSERT(len > 0);
  msg[sizeof(msg)/sizeof(msg[0])-1] = '\0'; /*NULL Terminate */
  DISPLAY_STRING(msg);

  /* Free mutex */
  dr_mutex_destroy(as_built_lock);
}

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

  return DR_EMIT_DEFAULT;
}
