/*
 * x86_code.c - auxiliary C routines to assembly routines in x86.asm
 */
#include "../globals.h"
#include "../fragment.h"
#include "../dispatch.h"
#include "../monitor.h"
#include "arch.h"
#include <string.h> /* for memcpy */

/* Initializes a dcontext with the supplied state and calls dispatch */
void
dynamo_start(dr_mcontext_t *mc)
{
}
