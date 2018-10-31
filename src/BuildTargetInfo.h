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
#endif
#ifdef TARGETOS_GENERIC_UNIX
     #define __TARGETOS_UNIX__
     #define TARGETOS "Unix"
#endif
#ifndef TARGETOS
     #define TARGETOS "Unknown Build Target"
#endif

#define GIT_COMMIT_HASH              ("b487ff95edd5")
#define GIT_COMMIT_HASH_SHORT        ("b487ff95edd595fa3d4becbfb5215ce935b86326")
#define GIT_COMMIT_DATE              ("Mon Oct 29 20:41:36 2018 -0400")
#define GIT_BRANCH_REVSTRING         ("heads/master-62-gb90bfc")
#define BUILD_PLATFORM_ID            ("Linux (4.15.0-20-generic) [x86_64] @ penguinboxhp")
#define BUILD_DATETIME               ("Wed 31 Oct 2018 05:45:47 PM EDT")

#define HUGE_BUFFER_LINE_SIZE        (2048)

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

	  static inline void DisplayAboutMessage() {
	       fl_message_hotspot(1);
	       fl_message_title("About This Application ...");
	       const char *bugReportMsg = "Please include a screenshot containing this information with any bug report you submit. New issues with the application can be submitted at https://github.com/gtDMMB/RNAStructViz/issues.";
	       fl_message("%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n(**%s**)", 
			  UserManualWikiLink().c_str(), GitRevisionInfo().c_str(), 
			  GitRevisionDate().c_str(), FLTKVersionString().c_str(), 
			  CairoVersionString().c_str(), BuildPlatform().c_str(), 
			  LocalBuildDateTime().c_str(), bugReportMsg);
	  }

};

#endif
