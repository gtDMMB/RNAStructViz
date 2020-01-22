/* OpenWebLinkWithBrowser.cpp : C++ implementation of the header file specs;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2020.01.17
 */

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <array>
#include <vector>

#include "OpenWebLinkWithBrowser.h"
#include "ConfigOptions.h"

int WebBrowserUtils::GetActiveOSPlatform() {
     if(!strcmp(TARGETOS, "GNU/Linux")) {
          return OSTYPE_LINUX_GENERIC | OSTYPE_UNIX_GENERIC;
     }
     else if(!strcmp(TARGETOS, "OpenBSD")) {
	  return OSTYPE_OPENBSD | OSTYPE_BSDUNIX_GENERIC | OSTYPE_UNIX_GENERIC;
     }
     else if(!strcmp(TARGETOS, "FreeBSD")) {
	  return OSTYPE_FREEBSD | OSTYPE_BSDUNIX_GENERIC | OSTYPE_UNIX_GENERIC;
     }
     else if(!strcmp(TARGETOS, "GNU/Hurd")) {
	  return OSTYPE_GNUHURD | OSTYPE_UNIX_GENERIC;
     }
     else if(!strcmp(TARGETOS, "MacOSX/Darwin")) {
	  return OSTYPE_MACOSX_DARWIN | OSTYPE_UNIX_GENERIC;
     }
     else {
          return OSTYPE_UNIX_GENERIC;
     }
     return OSTYPE_UNKNOWN;
}

std::string WebBrowserUtils::GetActiveTerminalCommand() {
     int osPlatform = WebBrowserUtils::GetActiveOSPlatform();
     if(osPlatform & OSTYPE_MACOSX_DARWIN) {
          static const char *macosTerminalCmd = "open -b com.apple.terminal @@#1@@";
	  return std::string(macosTerminalCmd);
     }
     char *termFromEnv = NULL;
     if((termFromEnv = getenv("TERM")) == NULL && 
	(termFromEnv = getenv("SHELL")) == NULL) {
          return "@@#1@@";
     }
     return std::string(termFromEnv) + "@@#1@@";
}

char * WebBrowserUtils::GetWebBrowserLaunchCommand(bool includePath) {
     unsigned int numDefinedBrowsers = GetArrayLength(WebBrowserUtils::WEBBROWSER_SPECS);
     std::vector< std::pair<int, int> > availWebBrowserData;
     std::vector<string> availWebBrowserCmdPaths;
     std::string cmdOutputStr = "";
     for(unsigned int wbi = 0; wbi < numDefinedBrowsers; wbi++) {
          if(system(WebBrowserUtils::WEBBROWSER_SPECS[wbi].webBrowserName) == -1) {
	       continue;
	  }
	  char whichCmdSyntax[MAX_BUFFER_SIZE];
	  snprintf(whichCmdSyntax, MAX_BUFFER_SIZE, "which %s", WebBrowserUtils::WEBBROWSER_SPECS[wbi].webBrowserName);
	  char cmdOutputBuf[MAX_BUFFER_SIZE];
          FILE* pipe = popen(whichCmdSyntax, "r");
          if (!pipe) throw std::runtime_error("popen() failed!");
          try {
               while (fgets(cmdOutputBuf, sizeof(cmdOutputBuf), pipe) != NULL) {
                    cmdOutputStr += cmdOutputBuf;
               }
          } catch(std::runtime_error stdoutReadExcpt) {
               pclose(pipe);
               throw stdoutReadExcpt;
          }
          pclose(pipe);
	  fprintf(stderr, "WHICH CMD OUTPUT [% 2d]: %s\n", wbi + 1, cmdOutputStr.c_str());
	  if(cmdOutputStr.empty() && includePath) {
	       continue;
	  }
	  else if(!includePath) {
	       std::string webBrowserCmd = WebBrowserUtils::WEBBROWSER_SPECS[wbi].webBrowserName;
	       availWebBrowserCmdPaths.push_back(webBrowserCmd);
	  }
	  else {
	       availWebBrowserCmdPaths.push_back(cmdOutputStr);
	  }
	  availWebBrowserData.push_back(std::pair(wbi, wbi));
     }
     if(availWebBrowserData.empty()) {
          return NULL;
     }
     auto wbSortCompareFunc = [](const std::pair<int, int> &i, const std::pair<int, int> &j) { return i.first < j.first; };
     std::sort(availWebBrowserData.begin(), availWebBrowserData.end(), wbSortCompareFunc);
     int wbSpecsDefaultIdx = availWebBrowserData[0].second;

     std::string wbBinaryCmd = std::string(includePath ? WebBrowserUtils::WEBBROWSER_SPECS[wbSpecsDefaultIdx].webBrowserName : 
		                                         cmdOutputStr.c_str());
     std::string wbRuntimeCmd = std::string(WebBrowserUtils::WEBBROWSER_SPECS[wbSpecsDefaultIdx].launchArgs);
     if(WebBrowserUtils::WEBBROWSER_SPECS[wbSpecsDefaultIdx].standAloneAppLaunch) {
          std::string termRunCmdSyntax = WebBrowserUtils::GetActiveTerminalCommand();
	  wbBinaryCmd = StringTextReplace(termRunCmdSyntax, TERMINAL_SYNTAX_COMMAND_PLACEHOLDER, wbRuntimeCmd.c_str());
     }
     wbRuntimeCmd = StringTextReplace(wbRuntimeCmd, WEBBROWSER_SYNTAX_COMMAND_PLACEHOLDER, wbBinaryCmd.c_str());

     size_t browserCmdNumChars = wbRuntimeCmd.length();
     char *returnBrowserRunCmd = (char *) malloc((browserCmdNumChars + 1) * sizeof(char));
     sprintf(returnBrowserRunCmd, "%s", 
	     includePath ? WebBrowserUtils::WEBBROWSER_SPECS[wbSpecsDefaultIdx].webBrowserName : cmdOutputStr.c_str());
     return returnBrowserRunCmd;
}

bool WebBrowserUtils::LaunchWebBrowserAtLink(const char *linkURL, bool copyToClipboard, bool copyToClipboardOnFail) {
     char *wbLaunchSyntax = WebBrowserUtils::GetWebBrowserLaunchCommand(true);
     bool writeClipboard = copyToClipboard || (wbLaunchSyntax == NULL && copyToClipboardOnFail);
     if(writeClipboard) {
          Fl::copy(linkURL, strlen(linkURL), 1, Fl::clipboard_plain_text);
     }
     if(wbLaunchSyntax == NULL) {
          return false;
     }
     std::string wbLaunchCmd = std::string(wbLaunchSyntax);
     wbLaunchCmd = StringTextReplace(wbLaunchCmd, WEBBROWSER_SYNTAX_COMMAND_PLACEHOLDER, linkURL);
     fprintf(stderr, "WEB START CMD: %s\n", wbLaunchCmd.c_str());
     std::vector< std::string > commandLineArgs = SplitStringAt(wbLaunchCmd, ' ');
     char **execArgv = (char **) malloc(commandLineArgs.size() * sizeof(char *));
     for(int cli = 0; cli < commandLineArgs.size(); cli++) {
          execArgv[cli] = (char *) malloc((commandLineArgs[cli].length() + 1) * sizeof(char));
	  sprintf(execArgv[cli], "%s", commandLineArgs[cli].c_str());
     }
     int newProcessStatus = execvp(execArgv[0], execArgv);
     for(int cli = 0; cli < commandLineArgs.size(); cli++) {
          Free(execArgv[cli]);
     }
     Free(execArgv);
     Free(wbLaunchSyntax);
     return newProcessStatus >= 0;
}
