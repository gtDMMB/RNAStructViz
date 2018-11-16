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

extern char *EXEPATH;

#ifdef __APPLE__
     #define ADDR2LINE              ("/usr/local/bin/gaddr2line")
#else
     #define ADDR2LINE              ("/usr/bin/addr2line")
#endif

const char *ADDR2LINE_FORMAT = "%s --exe=%s --inlines --pretty-print --functions --demangle %p";

const char * SegfaultCode2String(int scode) {
     switch(scode) {
	  case SEGV_MAPERR:
	       return "SEGV_MAPERR: address not mapped to object";
	  case SEGV_ACCERR:
	       return "SEGV_ACCERR: invalid permissions for mapped to object";
	  default:
               return "SEGV_NOOP: if only I knew...";
     }
}

bool PrintStackTraceString(void *staddr) {
     
     if(EXEPATH == NULL || staddr == NULL) {
          return false;
     }
     char runCmd[MAX_BUFFER_SIZE];
     snprintf(runCmd, MAX_BUFFER_SIZE - 1, ADDR2LINE_FORMAT, 
		      ADDR2LINE, EXEPATH, staddr);
     if(system(runCmd)) {
          return false;
     }
     return true;
}

void SegfaultSignalHandler(int signum, siginfo_t *sinfo, void *scontext) {

     void *callerAddress = NULL;
     void *backtraceArr[BACKTRACE_DEPTH];
     char **stacktraceMsgs = NULL;
     
     // TODO get the user callback address on Linux:
     
     fprintf(stderr, "\n\n========================================================\n\n");
     
     int btSize = backtrace(backtraceArr, BACKTRACE_DEPTH);
     if(callerAddress != NULL) { 
          backtraceArr[1] = callerAddress;
     }
     stacktraceMsgs = backtrace_symbols(backtraceArr, btSize);

     fprintf(stderr, "A critical error occurred that the application cannot recover from.\n");
     fprintf(stderr, "Terminating now with detailed debugging information.\n");
     fprintf(stderr, "Consider submitting a bug report if the problem persists.\n\n");
     fprintf(stderr, "%s raised from address %p at caller address %p.\n", 
		     strsignal(signum), sinfo->si_addr, (void *) callerAddress);
     fprintf(stderr, "SIGNAL CODE: %s\n", SegfaultCode2String(sinfo->si_code));
     fprintf(stderr, "ERRNO CODE RECEIVED: %s\n", strerror(sinfo->si_errno));
     #ifndef __APPLE__
          fprintf(stderr, "ADDITIONAL SIGNAL INFO RECEIVED:\n");
	  psiginfo(sinfo, NULL);
     #endif
     
     fprintf(stderr, "\nSTACKTRACE:\n\n");
     for(int st = 1; st < btSize; st++) {
          if(stacktraceMsgs == NULL) {
               break;
	  }
          PrintStackTraceString(backtraceArr[st]);
          fprintf(stderr, "[% 2d] %s\n", st, stacktraceMsgs[st]);
     }
     free(stacktraceMsgs);
     
     fprintf(stderr, "\n");
     ApplicationBuildInfo::PrintAboutListing(stderr);
     
     fprintf(stderr, "\n\n========================================================\n\n");
     exit(EXIT_FAILURE);

}

#endif
