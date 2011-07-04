#include "../globals.h"   /* just to disable warning C4206 about an empty file */


#include "instrument.h"
#include "../instrlist.h"
#include "arch.h"
#include "instr.h"
#include "instr_create.h"
#include "decode.h"
#include "disassemble.h"
#include "../fragment.h"
#include "../emit.h"
#include "../link.h"
#include "../monitor.h" /* for mark_trace_head */
#include <string.h> /* for strstr */
#include <stdarg.h> /* for varargs */
#include "../nudge.h" /* for nudge_internal() */
#include "../synch.h"
# include <sys/time.h> /* ITIMER_* */

/* This is a little convoluted.  The following is a macro to iterate
 * over a list of callbacks and call each function.  We use a macro
 * instead of a function so we can pass the function type and perform
 * a typecast.  We need to copy the callback list before iterating to
 * support the possibility of one callback unregistering another and
 * messing up the list while we're iterating.  We'll optimize the case
 * for 5 or fewer registered callbacks and stack-allocate the temp
 * list.  Otherwise, we'll heap-allocate the temp.
 *
 * We consider the first registered callback to have the highest
 * priority and call it last.  If we gave the last registered callback
 * the highest priority, a client could re-register a routine to
 * increase its priority.  That seems a little weird.
 */

#define FAST_COPY_SIZE 5
#define call_all_ret(ret, retop, postop, vec, type, ...)                \
    do {                                                                \
        size_t idx, num;                                                \
        mutex_lock(&callback_registration_lock);                        \
        num = (vec).num;                                                \
        if (num == 0) {                                                 \
            mutex_unlock(&callback_registration_lock);                  \
        }                                                               \
        else if (num <= FAST_COPY_SIZE) {                               \
            callback_t tmp[FAST_COPY_SIZE];                             \
            memcpy(tmp, (vec).callbacks, num * sizeof(callback_t));     \
            mutex_unlock(&callback_registration_lock);                  \
            for (idx=0; idx<num; idx++) {                               \
                ret retop (((type)tmp[num-idx-1])(__VA_ARGS__)) postop; \
            }                                                           \
        }                                                               \
        else {                                                          \
            callback_t *tmp = HEAP_ARRAY_ALLOC(GLOBAL_DCONTEXT, callback_t, num, \
                                               ACCT_OTHER, UNPROTECTED); \
            memcpy(tmp, (vec).callbacks, num * sizeof(callback_t));     \
            mutex_unlock(&callback_registration_lock);                  \
            for (idx=0; idx<num; idx++) {                               \
                ret retop ((type)tmp[num-idx-1])(__VA_ARGS__) postop;   \
            }                                                           \
            HEAP_ARRAY_FREE(GLOBAL_DCONTEXT, tmp, callback_t, num,      \
                            ACCT_OTHER, UNPROTECTED);                    \
        }                                                               \
    } while (0)

#define call_all(vec, type, ...)                        \
    do {                                                \
        int dummy;                                      \
        call_all_ret(dummy, =, , vec, type, __VA_ARGS__); \
    } while (0)

/* Acquire when registering or unregistering event callbacks */
DECLARE_CXTSWPROT_VAR(static mutex_t callback_registration_lock,
                      INIT_LOCK_FREE(callback_registration_lock));

/* An array of client libraries.  We use a static array instead of a
 * heap-allocated list so we can load the client libs before
 * initializing DR's heap.
 */
/* Structures for maintaining lists of event callbacks */
typedef void (*callback_t)(void);
typedef struct _callback_list_t {
    callback_t *callbacks;      /* array of callback functions */
    size_t num;                 /* number of callbacks registered */
    size_t size;                /* allocated space (may be larger than num) */
} callback_list_t;

/* Lists of callbacks for each event type.  Note that init and nudge
 * callback lists are kept in the client_lib_t data structure below.
 * We could store all lists on a per-client basis, but we can iterate
 * over these lists slightly more efficiently if we store all
 * callbacks for a specific event in a single list.
 */
static callback_list_t exit_callbacks = {0,};
static callback_list_t thread_init_callbacks = {0,};
static callback_list_t thread_exit_callbacks = {0,};
#ifdef LINUX
static callback_list_t fork_init_callbacks = {0,};
#endif
static callback_list_t bb_callbacks = {0,};
static callback_list_t trace_callbacks = {0,};
#ifdef CUSTOM_TRACES
static callback_list_t end_trace_callbacks = {0,};
#endif
static callback_list_t fragdel_callbacks = {0,};
static callback_list_t restore_state_callbacks = {0,};
static callback_list_t restore_state_ex_callbacks = {0,};
static callback_list_t module_load_callbacks = {0,};
static callback_list_t module_unload_callbacks = {0,};
static callback_list_t filter_syscall_callbacks = {0,};
static callback_list_t pre_syscall_callbacks = {0,};
static callback_list_t post_syscall_callbacks = {0,};
static callback_list_t signal_callbacks = {0,};

typedef struct _client_lib_t {
    client_id_t id;
    char path[MAXIMUM_PATH];
    /* PR 366195: dlopen() handle truly is opaque: != start */
    shlib_handle_t lib;
    app_pc start;
    app_pc end;
    char options[MAX_OPTION_LENGTH];
    /* We need to associate nudge events with a specific client so we
     * store that list here in the client_lib_t instead of using a
     * single global list.
     */
    callback_list_t nudge_callbacks;
} client_lib_t;

#ifdef CLIENT_SIDELINE
/* # of sideline threads */
DECLARE_CXTSWPROT_VAR(static int num_client_sideline_threads, 0);
#endif

static size_t num_client_libs = 0;
static client_lib_t client_libs[MAX_CLIENT_LIBS] = {{0,}};

#define USES_DR_VERSION_NAME "_USES_DR_VERSION_"
/* Should we expose this for use in samples/tracedump.c?
 * Also, if we change this, need to change the symlink generation
 * in core/CMakeLists.txt: at that point should share single define.
 */
#define OLDEST_COMPATIBLE_VERSION  96 /* 0.9.6 == 1.0.0 through 1.2.0 */
/* The 3rd version number, the bugfix/patch number, should not affect
 * compatibility, so our version check number simply uses:
 *   major*100 + minor
 * Which gives us room for 100 minor versions per major.
 */
#define NEWEST_COMPATIBLE_VERSION CURRENT_API_VERSION

void
instrument_load_client_libs(void)
{
	// COMPLETEDD #239 instrument_load_client_libs
  printf("INSTRUMENT.C instrument_load_client_libs_ FULLY DEVELOPEND\n");
  if (!IS_INTERNAL_STRING_OPTION_EMPTY(client_lib)) {
       char buf[MAX_LIST_OPTION_LENGTH];
       char *path;

       string_option_read_lock();
       strncpy(buf, INTERNAL_OPTION(client_lib), BUFFER_SIZE_ELEMENTS(buf));
       string_option_read_unlock();
       NULL_TERMINATE_BUFFER(buf);

       /* We're expecting path;ID;options triples */
       path = buf;
       do {
           char *id = NULL;
           char *options = NULL;
           char *next_path = NULL;

           id = strstr(path, ";");
           if (id != NULL) {
               id[0] = '\0';
               id++;

               options = strstr(id, ";");
               if (options != NULL) {
                   options[0] = '\0';
                   options++;

                   next_path = strstr(options, ";");
                   if (next_path != NULL) {
                       next_path[0] = '\0';
                       next_path++;
                   }
               }
           }

           add_client_lib(path, id, options);
           path = next_path;
       } while (path != NULL);
   }
}

/* This should only be called prior to instrument_init(),
 * since no readers of the client_libs array use synch
 * and since this routine assumes .data is writable.
 */
static void
add_client_lib(char *path, char *id_str, char *options)
{
	// COMPLETEDD #238 add_client_lib
  printf("INSTRUMENT.C add_client_lib FULLY DEVELOPED\n");
  client_id_t id;
  shlib_handle_t client_lib;
  DEBUG_DECLARE(size_t i);

  ASSERT(!dynamo_initialized);

  /* if ID not specified, we'll default to 0 */
  id = (id_str == NULL) ? 0 : strtoul(id_str, NULL, 16);

#ifdef DEBUG
  /* Check for conflicting IDs */
  for (i=0; i<num_client_libs; i++) {
      CLIENT_ASSERT(client_libs[i].id != id, "Clients have the same ID");
  }
#endif

  if (num_client_libs == MAX_CLIENT_LIBS) {
      CLIENT_ASSERT(false, "Max number of clients reached");
      return;
  }

  LOG(GLOBAL, LOG_INTERP, 4, "about to load client library %s\n", path);

  client_lib = load_shared_library(path);
  if (client_lib == NULL) {
      char msg[MAXIMUM_PATH*4];
      char err[MAXIMUM_PATH*2];
      shared_library_error(err, BUFFER_SIZE_ELEMENTS(err));
      snprintf(msg, BUFFER_SIZE_ELEMENTS(msg),
               "\n\tError opening instrumentation library %s:\n\t%s",
               path, err);
      NULL_TERMINATE_BUFFER(msg);

      /* PR 232490 - malformed library names or incorrect
       * permissions shouldn't blow up an app in release builds as
       * they may happen at customer sites with a third party
       * client.
       */
#ifdef LINUX
      /* PR 408318: 32-vs-64 errors should NOT be fatal to continue
       * in debug build across execve chains
       */
      if (strstr(err, "wrong ELF class") == NULL)
#endif
          CLIENT_ASSERT(false, msg);
      SYSLOG(SYSLOG_ERROR, CLIENT_LIBRARY_UNLOADABLE, 3,
             get_application_name(), get_application_pid(), msg);
  }
  else {
      /* PR 250952: version check */
      int *uses_dr_version = (int *)
          lookup_library_routine(client_lib, USES_DR_VERSION_NAME);
      if (uses_dr_version == NULL ||
          *uses_dr_version < OLDEST_COMPATIBLE_VERSION ||
          *uses_dr_version > NEWEST_COMPATIBLE_VERSION) {
          /* not a fatal usage error since we want release build to continue */
          CLIENT_ASSERT(false,
                        "client library is incompatible with this version of DR");
          SYSLOG(SYSLOG_ERROR, CLIENT_VERSION_INCOMPATIBLE, 2,
                 get_application_name(), get_application_pid());
      }
      else {
          size_t idx = num_client_libs++;
          DEBUG_DECLARE(bool ok;)
          client_libs[idx].id = id;
          client_libs[idx].lib = client_lib;
          DEBUG_DECLARE(ok =)
              shared_library_bounds(client_lib, (byte *) uses_dr_version,
                                    &client_libs[idx].start, &client_libs[idx].end);
          ASSERT(ok);

          LOG(GLOBAL, LOG_INTERP, 1, "loaded %s at "PFX"-"PFX"\n",
              path, client_libs[idx].start, client_libs[idx].end);
#ifdef X64
          /* provide a better error message than the heap assert */
          CLIENT_ASSERT(client_libs[idx].start < (app_pc)(ptr_int_t)INT_MAX,
                        "64-bit client library must have base in lower 2GB");
          request_region_be_heap_reachable(client_libs[idx].start,
                                           client_libs[idx].end -
                                           client_libs[idx].start);
#endif
          strncpy(client_libs[idx].path, path,
                  BUFFER_SIZE_ELEMENTS(client_libs[idx].path));
          NULL_TERMINATE_BUFFER(client_libs[idx].path);

          if (options != NULL) {
              strncpy(client_libs[idx].options, options,
                      BUFFER_SIZE_ELEMENTS(client_libs[idx].options));
              NULL_TERMINATE_BUFFER(client_libs[idx].options);
          }

          /* We'll look up dr_init and call it in instrument_init */
      }
  }
}

void
instrument_init(void)
{
}

void
instrument_fork_init(dcontext_t *dcontext)
{
}

void
instrument_exit(void)
{
}

/* Notify user when a module is unloaded */
void
instrument_module_unload(module_data_t *data)
{
}

DR_API
/* Used to free a module_data_t created by dr_copy_module_data() */
void
dr_free_module_data(module_data_t *data)
{
}

bool
dr_thread_exit_hook_exists(void)
{
  return 0;
}

void
instrument_thread_exit_event(dcontext_t *dcontext)
{
}

bool
dr_fragment_deleted_hook_exists(void)
{
	// COMPLETEDD #526 dr_fragment_deleted_hook_exists
	printf("Starting dr_fragment_deleted_hook_exists\n");
  return (fragdel_callbacks.num > 0);
}

int
get_num_client_threads(void)
{
	return 0;
}

void
instrument_thread_init(dcontext_t *dcontext, bool client_thread, bool valid_mc)
{
	// COMPLETEDD #417 instrument_thread_init
  /* Note that we're called twice for the initial thread: once prior
   * to instrument_init() (PR 216936) to set up the dcontext client
   * field (at which point there should be no callbacks since client
   * has not had a chance to register any), and once after
   * instrument_init() to call the client event.
   */
	printf("Starting Instrument_thread_init\n");
  if (dcontext->client_data == NULL) {
      dcontext->client_data = HEAP_TYPE_ALLOC(dcontext, client_data_t,
                                              ACCT_OTHER, UNPROTECTED);
      memset(dcontext->client_data, 0x0, sizeof(client_data_t));

#ifdef CLIENT_SIDELINE
      ASSIGN_INIT_LOCK_FREE(dcontext->client_data->sideline_mutex, sideline_mutex);
#endif
      CLIENT_ASSERT(dynamo_initialized || thread_init_callbacks.num == 0 ||
                    client_thread,
                    "1st call to instrument_thread_init should have no cbs");
  }

#ifdef CLIENT_SIDELINE
  if (client_thread) {
#ifndef ARM
      ATOMIC_INC(int, num_client_sideline_threads);
#else
      ATOMIC_INC(int, &num_client_sideline_threads);
#endif
      /* We don't call dynamo_thread_not_under_dynamo() b/c we want itimers. */
      dcontext->thread_record->under_dynamo_control = false;
      dcontext->client_data->is_client_thread = true;
      /* no init event */
      return;
  }
#endif /* CLIENT_SIDELINE */

  /* i#117/PR 395156: support dr_get_mcontext() from the thread init event */
  if (valid_mc)
      dcontext->client_data->mcontext_in_dcontext = true;
  call_all(thread_init_callbacks, int (*)(void *), (void *)dcontext);
  if (valid_mc)
      dcontext->client_data->mcontext_in_dcontext = false;
}

void
instrument_thread_exit(dcontext_t *dcontext)
{
}

bool
dr_exit_hook_exists(void)
{
	return 0;
}

void
instrument_fragment_deleted(dcontext_t *dcontext, app_pc tag, uint flags)
{
	// COMPLETEDD #527 instrument_fragment_deleted
	printf("Starting instrument_fragment_deleted\n");
  if (fragdel_callbacks.num == 0)
      return;

#ifdef WINDOWS
  /* Case 10009: We don't call the basic block hook for blocks that
   * are jumps to the interception buffer, so we'll hide them here
   * as well.
   */
  if (!TEST(FRAG_IS_TRACE, flags) && hide_tag_from_client(tag))
      return;
#endif

  /* PR 243008: we don't expose GLOBAL_DCONTEXT, so change to NULL.
   * Our comments warn the user about this.
   */
  if (dcontext == GLOBAL_DCONTEXT)
      dcontext = NULL;

  call_all(fragdel_callbacks, int (*)(void *, void *),
           (void *)dcontext, (void *)tag);
}

/* returns whether this syscall should execute */
bool
instrument_pre_syscall(dcontext_t *dcontext, int sysnum)
{
	return 0;
}

void
instrument_post_syscall(dcontext_t *dcontext, int sysnum)
{
}

bool
instrument_invoke_another_syscall(dcontext_t *dcontext)
{
	return 0;
}


/* Give the user the completely mangled and optimized trace just prior
 * to emitting into code cache, user gets final crack at it
 */
dr_emit_flags_t
instrument_trace(dcontext_t *dcontext, app_pc tag, instrlist_t *trace,
                 bool translating)
{
	return 0;
}

bool
dr_end_trace_hook_exists(void)
{
	return 0;
}

/* Ask whether to end trace prior to adding next_tag fragment.
 * Return values:
 *   CUSTOM_TRACE_DR_DECIDES = use standard termination criteria
 *   CUSTOM_TRACE_END_NOW    = end trace
 *   CUSTOM_TRACE_CONTINUE   = do not end trace
 */
dr_custom_trace_action_t
instrument_end_trace(dcontext_t *dcontext, app_pc trace_tag, app_pc next_tag)
{
	return 0;
}

bool
dr_bb_hook_exists(void)
{
	return 0;
}

bool
dr_trace_hook_exists(void)
{
	return 0;
}

module_data_t *
copy_module_area_to_module_data(const module_area_t *area)
{
	return 0;
}
/* Notify the client of a nudge. */
void
instrument_nudge(dcontext_t *dcontext, client_id_t id, uint64 arg)
{
}

bool
is_in_client_lib(app_pc addr)
{
	// COMPLETEDD #213 is_in_client_lib
  /* NOTE: we use this routine for detecting exceptions in
   * clients. If we add a callback on that event we'll have to be
   * sure to deliver it only to the right client.
   */
  size_t i;
  printf("INSTRUMENT.C IS_IN_CLIENT_LIB FULLY DEVELOPED\n");
  for (i=0; i<num_client_libs; i++) {
      if ((addr >= (app_pc)client_libs[i].start) &&
          (addr < client_libs[i].end)) {
          return true;
      }
  }

  return false;
}

/* Looks up the being-loaded module at modbase and invokes the client event */
void
instrument_module_load_trigger(app_pc modbase)
{
}

bool
dr_signal_hook_exists(void)
{
	return 0;
}

bool
hide_tag_from_client(app_pc tag)
{
	return 0;
}

dr_signal_action_t
instrument_signal(dcontext_t *dcontext, dr_siginfo_t *siginfo)
{
	return 0;
}

app_pc
get_client_base(client_id_t client_id)
{
	return 0;
}
