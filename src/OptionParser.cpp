/* OptionParser.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.31
 */

#include <stdlib.h>
#include <stdio.h>

#include <string>

#include "OptionParser.h"
#include "ConfigOptions.h"
#include "CommonDialogs.h"
#include "RNAStructViz.h"
#include "DisplayConfigWindow.h"

void ProcessAboutOption() {
     std::string infoAboutMsg = CommonDialogs::GetInfoAboutMessageString();
     ApplicationBuildInfo::PrintAboutListing(infoAboutMsg.c_str(), stdout);
     exit(EXIT_SUCCESS);
}

void ProcessHelpOption() {
     std::string helpInstMsg = CommonDialogs::GetHelpInstructionsMessageString();
     fprintf(stdout, "%s\n\n", helpInstMsg.c_str());
     fprintf(stdout, "For a complete list of command line options that can be passed to RNAStructViz, \n");
     fprintf(stdout, "see the following link:\n");
     fprintf(stdout, "« https://github.com/gtDMMB/RNAStructViz/wiki/CommandLineOptions »\n");
     exit(EXIT_SUCCESS);
}

void ProcessDebugOption() {
     DEBUGGING_ON = true;
     CFG_DEBUG_MODE = CFG_VERBOSE_MODE = 1;
     TerminalText::PrintDebug("THEME WINDOW BGCOLOR: 0x%08x\n", GUI_WINDOW_BGCOLOR);
     TerminalText::PrintDebug("THEME WIDGET COLOR:   0x%08x\n", GUI_BGCOLOR);
     TerminalText::PrintDebug("THEME BTEXT COLOR:    0x%08x\n", GUI_BTEXT_COLOR);
     TerminalText::PrintDebug("THEME TEXT COLOR:     0x%08x\n\n", GUI_TEXT_COLOR);
}

void ProcessNewConfigOption() {
     RNAStructViz::BackupAndUnlinkLocalConfigFiles(false);
     DisplayConfigWindow::SetupInitialConfig();
     DISPLAY_FIRSTRUN_MESSAGE = true;
}

void ProcessQuietOption() {
     CFG_QUIET_MODE = 1;
}

void ProcessVerboseOption() {
     CFG_VERBOSE_MODE = 1;
}

int ParseStructVizCommandOptions(int &argc, char ** &argv) {

     int argcInput = argc; 
     char **argvInput = argv;
     bool doneParsingStructViz = false;
     while(true) {
          
	  static struct option longarg_options[] = {
               { "about",           no_argument,      NULL,                      PRINT_ABOUT },
               { "debug",           no_argument,      NULL,                      PRINT_DEBUG }, 
               { "help",            no_argument,      NULL,                      PRINT_HELP  },
               { "new-config",      no_argument,      NULL,                      NEW_CONFIG  },
               { "no-ansi-color",   no_argument,      &PRINT_ANSI_COLOR,         SETVAR      },
               { "no-unicode",      no_argument,      &PRINT_TERMINAL_UNICODE,   SETVAR      },
               { "quiet",           no_argument,      &CFG_QUIET_MODE,           SETVAR      },
               { "verbose",         no_argument,      &CFG_VERBOSE_MODE,         SETVAR      },
               { NULL,              NULL,             NULL,                      NULL        },
          };
	     
	  int option_index = 0, optch;
	  optch = getopt_long(argcInput, argvInput, "hqvV", longarg_options, &option_index);
	  if(optch == -1) {
	       break;
	  }
          switch(optch) {
               case 0:  
		    switch(longarg_options[option_index].val) {
                         case SETVAR:
                              *(longarg_options[option_index].flag) = 1;
		              break;
		         case NOSETVAR:
                              *(longarg_options[option_index].flag) = 0;
			      break;
			 default:
			      break;
		    }
	       case PRINT_ABOUT:
                    ProcessAboutOption();
	            break;
	       case PRINT_HELP:
	       case 'h':
		    ProcessHelpOption();
		    break;
	       case PRINT_DEBUG:
	       case 'V':
		    ProcessDebugOption();
	            break;
	       case NEW_CONFIG:
	            ProcessNewConfigOption();
		    break;
	       case 'q':
		    ProcessQuietOption();
		    break;
	       case 'v':
		    ProcessVerboseOption();
		    break;
	       default:
		    doneParsingStructViz = true;
		    break;
	  }
	  if(doneParsingStructViz) {
               break;
	  }
     }
     int numOptionsParsed = optind - 1;
     if(REMOVE_STRUCTVIZ_OPTIONS) {
          argc -= numOptionsParsed;
	  argv[numOptionsParsed] = argv[0];
	  argv += numOptionsParsed;
     }

     return EXIT_SUCCESS;

}
