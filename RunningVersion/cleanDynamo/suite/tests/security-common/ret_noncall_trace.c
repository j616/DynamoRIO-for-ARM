/* **********************************************************
 * Copyright (c) 2004 VMware, Inc.  All rights reserved.
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

/* Description:	This test is intended to check if the -ibl_table_per_type 
 *   option works, which is done by generating a bogus-return-address-security-
 *   violation.  The nature of the violation is  such that it won't be caught
 *   by DR unless the -ibl_table_per_type option is turned on.  The program
 *   tries to return to a fragment that is classified as a tracehead and isn't
 *   the return site of a call.
 *   
 * Assumption: This test program will always be compiled /Od on Windows and 
 *   with default optimization on Linux.
 *
 * Notes: If the compilers change, their default optimization levels are 
 *   changed or if this code is optimized, the offset 6, used in next_num() for 
 *   saved_eip will change.  The change may be different on Windows and Linux.  
 *   The offset value in the code has to be modified.
 */

#include "tools.h"
 
#define NUM_TIMES 100
#define INNER_LOOP_COUNT 4
#define MAX_SUM (NUM_TIMES * (NUM_TIMES + 1) / 2 * INNER_LOOP_COUNT)

static ptr_uint_t saved_eip;

int
#ifdef X64
# ifdef WINDOWS  /* 5th param is on the stack */
next_num(int x1, int x2, int x3, int x4, int max_val)
# else  /* 7th param is on the stack */
next_num(int x1, int x2, int x3, int x4, int x5, int x6, int max_val)
# endif
#else
next_num(int max_val)
#endif
{
  static int counter;

  counter++;
  saved_eip = *(((ptr_uint_t*)&max_val) - IF_X64_ELSE(IF_WINDOWS_ELSE(5, 1), 1));
  saved_eip += 6;		/* Set rp to main()'s do-while loop. */
  return counter;
}

int
#ifdef X64
# ifdef WINDOWS  /* 5th param is on the stack */
check_sum(int x1, int x2, int x3, int x4, int sum)
# else  /* 7th param is on the stack */
check_sum(int x1, int x2, int x3, int x4, int x5, int x6, int sum)
# endif
#else
check_sum (int sum)
#endif
{
  *(((ptr_uint_t*)&sum) - IF_X64_ELSE(IF_WINDOWS_ELSE(5, 1), 1)) = saved_eip;

  return 1;			/* Make the bogus transition here. */
}

int main ()
{
  int i, val, inner_loop, sum = 0;

  INIT();

  print ("I think, therefore I am\n");

  for (i = 0; i < NUM_TIMES; i++)
  {
    inner_loop = INNER_LOOP_COUNT;
    /* Get next_num() into a trace. */
#ifdef X64
# ifdef WINDOWS
    val = next_num(1, 2, 3, 4, NUM_TIMES);
# else
    val = next_num(1, 2, 3, 4, 5, 6, NUM_TIMES);
# endif
#else
    val = next_num(NUM_TIMES);
#endif
    do {
      sum += val;
      if (sum > MAX_SUM)
      {
	print (" ... in serious trouble!\n");
	exit (-1);
      }
    } while (--inner_loop);
  }

#ifdef X64
# ifdef WINDOWS
    val = check_sum(1, 2, 3, 4, sum);
# else
    val = check_sum(1, 2, 3, 4, 5, 6, sum);
# endif
#else
    val = check_sum(sum);
#endif
}
