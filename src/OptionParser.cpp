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
     RNAStructViz::BackupAndUnlinkLocalConfigFiles();
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

     int option_index = 0, optch = -1;
     bool doneParsingStructViz = false;
     while(true) {
	  optch = getopt_long(argc, argv, shortarg_options, longarg_options, &option_index);
	  if(optch == -1) {
	       break;
	  }
	  else if(optch == 0) {
               switch(longarg_options[option_index].val) {
                    case SETVAR:
                         *(longarg_options[option_index].flag) = 1;
		         break;
		    case NOSETVAR:
                         *(longarg_options[option_index].flag) = 0;
			 break;
		    case PRINT_ABOUT:
			 ProcessAboutOption();
			 break;
		    case PRINT_HELP:
			 ProcessHelpOption();
			 break;
		    case PRINT_DEBUG:
			 ProcessDebugOption();
			 break;
		    case NEW_CONFIG:
			 ProcessNewConfigOption();
			 break;
		    default:
			 break;
	       }
	  }
	  switch(optch) {
               case 'h':
	            ProcessHelpOption();
		    break;
	       case 'q':
		    ProcessQuietOption();
		    break;
	       case 'v':
		    ProcessVerboseOption();
		    break;
	       case 'V':
		    ProcessDebugOption();
		    break;
	       default:
		    doneParsingStructViz = true;
		    break;
	  }
	  if(doneParsingStructViz) {
               break;
	  }
     }
     int numOptionsParsed = optind;
     if(REMOVE_STRUCTVIZ_OPTIONS) {
          argc -= numOptionsParsed;
	  argv[numOptionsParsed] = argv[0];
     }

     return EXIT_SUCCESS;

}
