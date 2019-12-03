#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#ifdef WITHGPERFTOOLS
#include <gperftools/heap-profiler.h>
#endif

#include "MainWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "DisplayConfigWindow.h"
#include "TerminalPrinting.h"
#include "OptionParser.h"
#include "ConfigExterns.h"

#ifdef __STRUCTVIZ_INSTALL_SIGNAL_HANDLERS
     #include <signal.h>
#endif

void RNAStructViz_SignalHandler(int signum);

char rnaStructVizExecPath[MAX_BUFFER_SIZE];
char runtimeCWDPath[MAX_BUFFER_SIZE];
char activeSystemUserFromEnv[MAX_BUFFER_SIZE];

int main(int argc, char **argv) {

    #ifdef WITHGPERFTOOLS
         HeapProfilerStart("RNAStructViz.log");
    #endif
   
    #ifdef __STRUCTVIZ_INSTALL_SIGNAL_HANDLERS
         if(__STRUCTVIZ_INSTALL_SIGSEGV_HANDLER) {
          signal(SIGSEGV, RNAStructViz_SignalHandler);
     }
     if(__STRUCTVIZ_INSTALL_CTRLC_HANDLER) {
          signal(SIGINT, RNAStructViz_SignalHandler);
     }
    #endif
 
    DisplayConfigWindow::SetupInitialConfig();
    strncpy(rnaStructVizExecPath, argv[0], MAX_BUFFER_SIZE);
    rnaStructVizExecPath[MAX_BUFFER_SIZE - 1] = '\0';
    getcwd(runtimeCWDPath, MAX_BUFFER_SIZE);
    runtimeCWDPath[MAX_BUFFER_SIZE - 1] = '\0';
    getUserNameFromEnv(activeSystemUserFromEnv, MAX_BUFFER_SIZE);
    
    ParseStructVizCommandOptions(argc, argv);
    RNAStructViz::Initialize(argc, argv);
    
    Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
    Fl::option(Fl::OPTION_SHOW_TOOLTIPS, true);
    Fl::option(Fl::OPTION_FNFC_USES_GTK, false);
    
    Fl_File_Icon::load_system_icons();
    Fl::scheme((char *) FLTK_THEME);
    MainWindow::ResetThemeColormaps();
    fl_font(LOCAL_BFFONT, LOCAL_TEXT_SIZE);
    //Fl::add_handler(RNAStructViz::HandleGlobalKeypressEvent);

    if(DISPLAY_FIRSTRUN_MESSAGE) {
         MainWindow::GetInstance()->DisplayFirstTimeUserInstructions();
    }
    while(MainWindow::IsRunning()) {
        Fl::wait();
    }
    //Fl::remove_handler(RNAStructViz::HandleGlobalKeypressEvent);
    RNAStructViz::Shutdown();
    
    #ifdef WITHGPERFTOOLS
         HeapProfilerStop();
    #endif
    
    return EXIT_SUCCESS;

}

void RNAStructViz_SignalHandler(int signum) {
     
     if(signum == SIGINT) {
          fprintf(stderr, "\n");
          TerminalText::PrintInfo("Handling <CTRL+C> (SIGINT) signal ... Saving config and exiting.\n");
          //ConfigParser::WriteUserConfigFile();
	  RNAStructViz::Shutdown();
     }
     else if(signum == SIGSEGV) {
          TerminalText::PrintError("Handling unexpected SEGFAULT (SIGSEGV) signal.\n\n");
          std::string aboutInfoMsg = CommonDialogs::GetInfoAboutMessageString();
      ApplicationBuildInfo::PrintAboutListing(aboutInfoMsg.c_str(), stderr);
     }
     exit(signum);

}

