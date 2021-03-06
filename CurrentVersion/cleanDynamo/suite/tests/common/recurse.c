/* **********************************************************
 * Copyright (c) 2003 VMware, Inc.  All rights reserved.
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

#include <stdio.h>
#include <assert.h>

#ifdef USE_DYNAMO
#include "dynamorio.h"
#endif

#define GOAL 256

int bar(int n)
{
    printf("bar %d\n", n);
    if (n==0) return 1;
    if (n % 2 == 0)
	return n + foo(n-1);
    if (n % 2 == 1)
	return n + bar(n-1);
    printf("\tdone with bar %d\n", n);
    return 0;
}

int foo(int n)
{
    printf("foo %d\n", n);
    if (n==0) return 1;
    if (n % 2 == 0)
	return n + foo(n-1);
    if (n % 2 == 1)
	return n + bar(n-1);
    printf("\tdone with foo %d\n", n);
    return 0;
}


main()
{
    int i, t = 0;

#ifdef USE_DYNAMO
    int rc = dynamorio_app_init();
    assert(rc == 0);
    dynamorio_app_start();
#endif

    for (i=GOAL; i<=GOAL; i++) {
	t = foo(i);
	printf("%d %d\n", i, t);
    }

#ifdef USE_DYNAMO
    dynamorio_app_stop();
    dynamorio_app_exit();
#endif

    return 0;
}
	
