// Basic printout that just prints stuff out.
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

  DISPLAY_STRING("Dynamo is running on your program!!!\n");
}

static void event_exit()
{
  DISPLAY_STRING("Dynamo is now exiting as your program has ended!!!\n");
}

static dr_emit_flags_t event_basic_block(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating)
{
  return DR_EMIT_DEFAULT;
}
