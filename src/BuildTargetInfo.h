/* BuildTargetInfo.h(.in) : Local platform specific definitions inserted at compile time; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.31
 */

#ifndef __BUILD_TARGET_INFO_H__
#define __BUILD_TARGET_INFO_H__

#ifdef TARGETOS_LINUX
     #define __TARGETOS_LINUX__
     #define TARGETOS "GNU/Linux"
#endif
#ifdef TARGETOS_MACOSX
     #define __TARGETOS_APPLE__
     #define TARGETOS "Mac/OSX"
     #define __APPLE__
#endif
#ifdef TARGETOS_GENERIC_UNIX
     #define __TARGETOS_UNIX__
     #define TARGETOS "Unix"
#endif
#ifndef TARGETOS
     #define TARGETOS "Unknown Build Target"
#endif

#define FLTK_USE_CAIRO 1
#define FLTK_HAVE_CAIRO 1

#define RNASTRUCTVIZ_VSTRING         ("RNAStructViz v1.1 -- CT File Viewer and Structure Comparison Tool")

#define GIT_COMMIT_HASH              ("c50ea6d57a10319d5c2a95722fcfd367abf61347")
#define GIT_COMMIT_HASH_SHORT        ("c50ea6d57a10")
#define GIT_COMMIT_DATE              ("Wed Nov 14 20:17:06 2018 -0500")
#define GIT_BRANCH_REVSTRING         ("heads/master-118-g51d50d")
#define BUILD_PLATFORM_ID            ("Linux (4.15.0-20-generic) [x86_64] @ penguinboxhp")
#define BUILD_DATETIME               ("Wed 14 Nov 2018 08:24:14 PM EST")
#define BUILD_FLTK_CONFIG            ("/home/maxie/RNA-projects/RNAStructViz/src/fltk-config")

#define HUGE_BUFFER_LINE_SIZE        (2048)

#include <stdlib.h>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include <string>
using std::string;

class ApplicationBuildInfo {

     public:
          static inline string UserManualWikiLink() {
               return string("User Manual: https://github.com/gtDMMB/RNAStructViz/wiki");
	  }

	  static inline string GitRevisionInfo() {
               char revString[HUGE_BUFFER_LINE_SIZE];
               snprintf(revString, HUGE_BUFFER_LINE_SIZE, 
	                "Git Revision: %s (%s) [%s]", 
	                GIT_COMMIT_HASH_SHORT, GIT_COMMIT_HASH, GIT_BRANCH_REVSTRING);
               revString[HUGE_BUFFER_LINE_SIZE - 1] = '\0';
               return string(revString);
          }

          static inline string GitRevisionDate() {
               return string("Revision Date: ") + string(GIT_COMMIT_DATE);
          }

          static inline string BuildPlatform() {
	       return string("Target Platform: ") + string(BUILD_PLATFORM_ID);
	  }

	  static inline string LocalBuildDateTime() {
	       return string("Local Build Time: ") + string(BUILD_DATETIME);
	  }

	  static inline string BuildFLTKConfig() {
               return string("Compile-Time FLTK-Config: ") + 
		      string(BUILD_FLTK_CONFIG);
	  }

	  static inline string CairoVersionString() {
               return string("Cairo Library Version: ") + string(CAIRO_VERSION_STRING);
	  }

	  static inline string FLTKVersionString() {
               char fltkVersionStr[HUGE_BUFFER_LINE_SIZE];
	       snprintf(fltkVersionStr, HUGE_BUFFER_LINE_SIZE - 1, 
			"%d.%d.%d (API #%d)", 
			FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION, FL_API_VERSION);
	       fltkVersionStr[HUGE_BUFFER_LINE_SIZE - 1] = '\0';
	       return string("FLTK Library Version: ") + string(fltkVersionStr);
	  }

	  static inline void PrintAboutListing(FILE *whichOut = stderr) {
               if(whichOut == NULL) {
                    whichOut = stderr;
	       }
	       fprintf(whichOut, "==== %s ====\n\n", RNASTRUCTVIZ_VSTRING);
	       fprintf(whichOut, "ABOUT THIS APPLICATION:\n");
               fprintf(whichOut, "   >> %s\n   >> %s\n   >> %s\n   >> %s\n   >> %s\n   >> %s\n   >> %s\n   >> %s\n\n", 
			  UserManualWikiLink().c_str(), GitRevisionInfo().c_str(), 
			  GitRevisionDate().c_str(), FLTKVersionString().c_str(),
			  BuildFLTKConfig().c_str(),  
			  CairoVersionString().c_str(), BuildPlatform().c_str(), 
			  LocalBuildDateTime().c_str());
	       const char *bugReportMsg = "Please include a screenshot along with this information in any bug report you submit. New issues with the application can be submitted at https://github.com/gtDMMB/RNAStructViz/issues.";
	       fprintf(whichOut, "BUG REPORT INFORMATION:\n**%s**\n", bugReportMsg);
	  }

};

#endif
