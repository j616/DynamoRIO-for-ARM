// Program counts the number of multiplication and divisions that could be changed to a shift

#include "dr_api.h" 

#define DISPLAY_STRING(msg) dr_printf("%s\n", msg);
#define NULL_TERMINATE(buf) buf[(sizeof(buf)/sizeof(buf[0])) - 1] = '\0'

static dr_emit_flags_t bb_event(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating);

static void event_exit();

static int div_count = 0, div_p2_count = 0;
static void *count_mutex; // For multithread supprt

DR_EXPORT void dr_init(client_id_t id)
{
  dr_register_exit_event(event_exit);
  dr_register_bb_event(bb_event);
  count_mutex = dr_mutex_create();
}

static void event_exit(void)
{
  char msg[512];
  int len;
  len = dr_snprintf(msg, sizeof(msg)/sizeof(msg[0]), "Instrumentation results:\n"
    "  saw %d div instructions\n"
    "  of which %d were powers of 2\n",
    div_count, div_p2_count);
  DR_ASSERT(len > 0);
  NULL_TERMINATE(msg);
  DISPLAY_STRING(msg);

  dr_mutex_destroy(count_mutex);
}

static void callback(app_pc addr, uint divisor)
{
  // Instead of a lock could use atomic operations to increment the counters
  dr_mutex_lock(count_mutex);

  div_count++;

  // Check for power of 2
  if ((divisor & (divisor - 1)) == 0)
    div_p2_count++;

  dr_mutex_unlock(count_mutex);
}

static dr_emit_flags_t bb_event(void* drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating)
{
  instr_t *instr, *next_instr;
  int opcode;

  for (instr = instrlist_first(bb); instr != NULL; instr = next_instr)
  {
    next_instr = instr_get_next(instr);
    opcode = instr_get_opcode(instr);

    // If find div, insert a clean call to our instrumentation routine
    if(opcode == OP_div)
    {
      dr_insert_clean_call(drcontext, bb, instr, (void *)callback, false, 2, OPND_CREATE_INTPTR(instr_get_app_pc(instr)),
        instr_get_src(instr, 0)); // Divisor is the first src*/);
    }
  }
  return DR_EMIT_DEFAULT;
}
