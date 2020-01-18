/* OpenWebLinkWithBrowser.h : Browser and command args definitions for common web browsers apps on Linux/Mac/Unix, 
 *                            code to configure, and code to subsequently launch the browser 
 *                            targeted at a specific web-http(s)-link on the running system. 
 *                            Should be platform independent up to which browsers are actually installed on the 
 *                            Unix-ish system in question... 
 * Author: Maxie D. Schmidt (maxieds@gmail.com) 
 * Created: 2020.01.17 
 */

#ifndef __OPENWEBLINKWITHBROWSER_H__
#define __OPENWEBLINKWITHBROWSER_H__

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <vector>

#include "RNAStructVizTypes.h"

namespace WebBrowserUtils {

     OSPlatformType_t GetActiveOSPlatform();
     
     static inline const char *TERMINAL_SYNTAX_COMMAND_PLACEHOLDER = "@@#1@@";
     std::string GetActiveTerminalCommand();

     typedef struct {
          const char *webBrowserName; 
          const char *launchArgs;
          const char *linkSyntaxReplaceText;
          bool standAloneAppLaunch;
          OSPlatformType_t platformList; 
          unsigned int preferenceRanking;
     } WebBrowserCmdSpec_t;

     static inline const char *WEBBROWSER_SYNTAX_COMMAND_PLACEHOLDER = "@@#1@@";
     static inline const char *WEBLINK_SYNTAX_COMMAND_PLACEHOLDER =    "@@#2@@";

     static inline const WebBrowserCmdSpec_t WEBBROWSER_SPECS[] = {
          { 
	        "python",                  
                "@@#1@@ -m webbrowser -t \"@@#2@@\"", 
	        WEBLINK_SYNTAX_COMMAND_PLACEHOLDER, 
	        false, 
	        OSTYPE_ALL, 
	        0, 
          },
          { 
	        "chrome",                  
                "",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                true,   
                OSTYPE_ALL,          
                1 
          }, 
	  { 
	        "google-chrome",                  
                "",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                true,   
                OSTYPE_ALL,          
                1 
          }, 
          { 
	        "chromium-browser",        
                "@@#1@@ --new-window @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                true,   
                OSTYPE_UNIX_ALL,     
                1 
          }, 
          { 
                "firefox",                 
                "@@#1@@ -new-window @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                true,   
                OSTYPE_UNIX_ALL,     
                2 
          }, 
          { 
	        "opera",                   
                "",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                true,   
                OSTYPE_UNIX_ALL,     
                9 
          },
          { 
                "epiphany",                
     	        "",          
		WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
		true,   
		OSTYPE_UNIX_ALL,     
		4 
	  },
          { 
		"konqueror",               
	        "",          
		WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
		true,   
		OSTYPE_UNIX_ALL,     
		5 
	  }, 
          { 
		"lynx",                    
                "@@#1@@ @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                false,  
                OSTYPE_UNIX_ALL,     
                8 
          }, 
          { 
		"elinks",                  
                "@@#1@@ @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                false,  
                OSTYPE_UNIX_ALL,     
                6 
          },
          { 
		"links",                   
                "@@#1@@ @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                false,  
	        OSTYPE_UNIX_ALL,     
                7 
          },
          { 
		"links2",                   
                "@@#1@@ @@#2@@",          
                WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
                false,  
		OSTYPE_UNIX_ALL,     
                7 
          },
          { 
		"safari",                  
                "open -b @@#1@@ @@#2@@ TODO",          
		WEBLINK_SYNTAX_COMMAND_PLACEHOLDER,  
		true,   
		OSTYPE_MACOS_DARWIN, 
		3 
	  },  
     };
     
     char * GetWebBrowserLaunchCommand(bool includePath = true);
     bool LaunchWebBrowserAtLink(const char *linkURL, bool copyToClipboard = false, bool copyToClipboardOnFail = true);
     
} // namespace WebBrowserUtils

#endif
