/* Debugging.h : Static functions designed to track down runtime errors and 
 *               assist users with submitting any bug reports they run into;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.11.16
 */

#ifndef __DEBUGGING_H__
#define __DEBUGGING_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include "ConfigOptions.h"

#define STRUCTVIZ_DEBUGGING         (true)
#define BACKTRACE_DEPTH             (64)

/* This structure is similar to the one found in /usr/include/asm/ucontext.h */
typedef struct {
     unsigned long     uc_flags;
     struct ucontext   *uc_link;
     stack_t           uc_stack;
     struct sigcontext uc_mcontext;
     sigset_t          uc_sigmask;
} SigContext_t;

inline void SegfaultSignalHandler(int signum, siginfo_t *sinfo, void *scontext) {

     SigContext_t *sc = (SigContext_t *) scontext;
     void *callerAddress = NULL;
     void *backtraceArr[BACKTRACE_DEPTH];
     char **stacktraceMsgs = NULL;
     
     #if defined(__i386__)                                         // gcc specific
          callerAddress = (void *) sc->uc_mcontext.eip;            // x86 specific
     #elif defined(__x86_64__)                                     // gcc specific
          callerAddress = (void *) sc->uc_mcontext.rip;            //x86_64 specific
     #else
     #error Unsupported architecture. 
     #endif

     int btSize = backtrace(backtraceArr, BACKTRACE_DEPTH);
     backtraceArr[1] = callerAddress;
     stacktraceMsgs = backtrace_symbols(backtraceArr, btSize);

     fprintf(stderr, "========================================================\n\n");
     fprintf(stderr, "A critical error occurred that the application \n");
     fprintf(stderr, "cannot recover from. Terminating now with detailed logs.\n\n");
     fprintf(stderr, "%s raised from address %P at caller address %p.\n", 
		     strsignal(signum), sinfo->si_addr, 
		     (void *) callerAddress);
     fprintf(stderr, "STACKTRACE:\n\n");

     for(int st = 1; st < btSize; st++) {
          if(stacktraceMsgs == NULL) {
               break;
	  }
	  fprintf(stderr, "   => (% 2d) %s\n", st, stacktraceMsgs[st]);
     }
     fprintf(stderr, "\n");
     ApplicationBuildInfo::PrintAboutListing(stderr);
     fprintf(stderr, "\n");
     fprintf(stderr, "========================================================\n\n");

     free(stacktraceMsgs);
     exit(EXIT_FAILURE);

}

#endif
