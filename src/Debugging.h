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

#ifndef __APPLE__
     #include <inttypes.h>
#endif

#include <string>
#include <regex>

#include "ConfigOptions.h"

#define STRUCTVIZ_DEBUGGING         (true)
#define BACKTRACE_DEPTH             (64)

extern char *EXEPATH;

#ifdef __APPLE__
     #define ADDR2LINE              ("/usr/local/bin/gaddr2line")
#else
     #define ADDR2LINE              ("/usr/bin/addr2line")
#endif

const char *ADDR2LINE_FORMAT = "%s --exe=%s --inlines --pretty-print --functions --demangle %s";

#ifndef __APPLE__
/* Solution for finding the *physical* address of a pointer taken from:
 * https://stackoverflow.com/questions/2440385/how-to-find-the-physical-address-of-a-variable-from-user-space-in-linux/28987409#28987409
 */
uintptr_t vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    int offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e[1];
    // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e[0], sizeof(uint64_t), 1, pagemap)) {
                if (e[0] & (1ULL << 63)) { // page present ?
                    paddr = e[0] & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }   
            }   
        }   
        fclose(pagemap);
    }   
    return paddr;
}
#endif

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

bool PrintStackTraceString(const char *offsetAddr) {
     
     if(EXEPATH == NULL || offsetAddr == NULL) {
          return false;
     }
     char runCmd[MAX_BUFFER_SIZE];
     snprintf(runCmd, MAX_BUFFER_SIZE - 1, ADDR2LINE_FORMAT, 
		      ADDR2LINE, EXEPATH, offsetAddr);
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
	  std::string offsetAddr(stacktraceMsgs[st]);
	  std::regex offsetAddrRegex(R"(0[xX][[:xdigit:]][[:xdigit:]]*)");
	  std::smatch matchingAddr;
	  regex_search(offsetAddr, matchingAddr, offsetAddrRegex);
	  std::sregex_token_iterator addrMatchesIter( 
	       offsetAddr.begin(), offsetAddr.end(), offsetAddrRegex, -1);
	  offsetAddr = matchingAddr.str();
          #ifndef __APPLE__
	       uintptr_t physAddr = vtop((uintptr_t) backtraceArr[st]);
               char phyAddrStr[MAX_BUFFER_SIZE];
	       snprintf(phyAddrStr, MAX_BUFFER_SIZE - 1, "%p", physAddr);
	       if(physAddr != NULL) 
	            offsetAddr = string(phyAddrStr);
          #endif
	  PrintStackTraceString(offsetAddr.c_str());
          fprintf(stderr, "[% 2d] %s\n", st, stacktraceMsgs[st]);
     }
     free(stacktraceMsgs);
     
     fprintf(stderr, "\n");
     ApplicationBuildInfo::PrintAboutListing(stderr);
     
     fprintf(stderr, "\n\n========================================================\n\n");
     exit(EXIT_FAILURE);

}

#endif
