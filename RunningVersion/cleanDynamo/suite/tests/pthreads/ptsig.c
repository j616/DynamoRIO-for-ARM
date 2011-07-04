/* **********************************************************
 * Copyright (c) 2003-2008 VMware, Inc.  All rights reserved.
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

/* Build with:
 * gcc -o pthreads pthreads.c -lpthread -D_REENTRANT -I../lib -L../lib -ldynamo -ldl -lbfd -liberty
 */

#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <ucontext.h>
#include <assert.h>

#ifdef USE_DYNAMO
#include "dynamorio.h"
#endif

/* handler with SA_SIGINFO flag set gets three arguments: */
typedef void (*handler_t)(int, struct siginfo *, void *);

volatile double pi = 0.0;  /* Approximation to pi (shared) */
pthread_mutex_t pi_lock;   /* Lock for above */
volatile double intervals; /* How many intervals? */

static void
signal_handler(int sig, siginfo_t *siginfo, ucontext_t *ucxt)
{
#if VERBOSE
    print("thread %d signal_handler: sig=%d, retaddr="PFX", fpregs="PFX"\n",
	    getpid(), sig, *(&sig - 1), ucxt->uc_mcontext.fpregs);
#endif

    switch (sig) {
    case SIGUSR1: {
	struct sigcontext *sc = (struct sigcontext *) &(ucxt->uc_mcontext);
#ifdef X64
	void *pc = (void *) sc->rip;
#else
	void *pc = (void *) sc->eip;
#endif
#if VERBOSE
	print("thread %d got SIGUSR1 @ "PFX"\n", getpid(), pc);
#endif
        break;
    }
    default:
	assert(0);
    }
}

/* set up signal_handler as the handler for signal "sig" */
static void
intercept_signal(int sig, handler_t handler)
{
    int rc;
    struct sigaction act;

    act.sa_sigaction = handler;
#if BLOCK_IN_HANDLER
    rc = sigfillset(&act.sa_mask); /* block all signals within handler */
#else
    rc = sigemptyset(&act.sa_mask); /* no signals are blocked within handler */
#endif
    assert(rc == 0);
    act.sa_flags = SA_SIGINFO | SA_ONSTACK; /* send 3 args to handler */
    
    /* arm the signal */
    rc = sigaction(sig, &act, NULL);
    assert(rc == 0);
}

void *
process(void *arg)
{
    char *id = (char *) arg;
    register double width, localsum;
    register int i;
    register int iproc;

#if VERBOSE
    print("thread %s starting\n", id);
#endif
    if (((char *)arg)[0] == '1') {
	intercept_signal(SIGUSR1, (handler_t) SIG_IGN);
#if VERBOSE
	print("thread %d ignoring SIGUSR1\n", getpid());
#endif
    }
#if VERBOSE
    print("thread %d sending SIGUSR1\n", getpid());
#endif
    kill(getpid(), SIGUSR1);

    iproc = (*((char *) arg) - '0');

    /* Set width */
    width = 1.0 / intervals;

    /* Do the local computations */
    localsum = 0;
    for (i=iproc; i<intervals; i+=2) {
	register double x = (i + 0.5) * width;
	localsum += 4.0 / (1.0 + x * x);
    }
    localsum *= width;

    /* Lock pi for update, update it, and unlock */
    pthread_mutex_lock(&pi_lock);
    pi += localsum;
    pthread_mutex_unlock(&pi_lock);

#if VERBOSE
    print("thread %s exiting\n", id);
#endif
    return(NULL);
}

int
main(int argc, char **argv)
{
    pthread_t thread0, thread1;
    void * retval;

#ifdef USE_DYNAMO
    dynamorio_app_init();
    dynamorio_app_start();
#endif

#if 0
    /* Get the number of intervals */
    if (argc != 2) {
	print("Usage: %s <intervals>\n", argv[0]);
	exit(0);
    }
    intervals = atoi(argv[1]);
#else /* for batch mode */
    intervals = 10;
#endif

    /* Initialize the lock on pi */
    pthread_mutex_init(&pi_lock, NULL);

    intercept_signal(SIGUSR1, (handler_t) signal_handler);

    /* Make the two threads */
    if (pthread_create(&thread0, NULL, process, "0") ||
	pthread_create(&thread1, NULL, process, "1")) {
	print("%s: cannot make thread\n", argv[0]);
	exit(1);
    }
    
    /* Join (collapse) the two threads */
    if (pthread_join(thread0, &retval) ||
	pthread_join(thread1, &retval)) {
	print("%s: thread join failed\n", argv[0]);
	exit(1);
    }

#if VERBOSE
    print("thread %d sending SIGUSR1\n", getpid());
#endif
    kill(getpid(), SIGUSR1);

    /* Print the result */
    print("Estimation of pi is %16.15f\n", pi);

    struct timespec sleeptime;
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 1000*1000*1000; /* 100ms */
    nanosleep(&sleeptime, NULL);

#ifdef USE_DYNAMO
    dynamorio_app_stop();
    dynamorio_app_exit();
#endif
}
