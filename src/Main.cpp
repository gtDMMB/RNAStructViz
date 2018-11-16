#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "MainWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "DisplayConfigWindow.h"

int main(int argc, char **argv) {

    DisplayConfigWindow::SetupInitialConfig();
    if(argc > 1 && 
       (!strcmp(argv[1], "--about") || !strcmp(argv[0], "--help") || 
	!strcmp(argv[1], "-h"))) {
        ApplicationBuildInfo::PrintAboutListing(stdout);
        return 0;
    }
    else if(argc > 1 && !strcmp(argv[1], "--debug")) {
        DEBUGGING_ON = true;
	fprintf(stderr, "THEME WINDOW BGCOLOR: 0x%08x\n", GUI_WINDOW_BGCOLOR);
	fprintf(stderr, "THEME WIDGET COLOR:   0x%08x\n", GUI_BGCOLOR);
	fprintf(stderr, "THEME BTEXT COLOR:    0x%08x\n", GUI_BTEXT_COLOR);
	fprintf(stderr, "THEME TEXT COLOR:     0x%08x\n", GUI_TEXT_COLOR);
    }	
    RNAStructViz::Initialize(argc, argv);
   
    Fl::option(Fl::OPTION_VISIBLE_FOCUS, false);
    Fl::option(Fl::OPTION_SHOW_TOOLTIPS, true);
    Fl::option(Fl::OPTION_FNFC_USES_GTK, true);

    //Fl::own_colormap();
    //uchar rc, bc, gc;
    //Fl::get_color(GUI_BGCOLOR, rc, bc, gc); 
    //Fl::background(rc, bc, gc);
    //Fl::get_color(LOCAL_FGCOLOR, rc, bc, gc); 
    //Fl::foreground(rc, bc, gc);
    //Fl::get_color(LOCAL_BG2COLOR, rc, bc, gc); 
    //Fl::background2(rc, bc, gc); 

    //Fl::get_system_colors();
    Fl_File_Icon::load_system_icons();
    Fl::scheme((char *) FLTK_THEME);
    MainWindow::ResetThemeColormaps();
    fl_font(LOCAL_RMFONT, LOCAL_TEXT_SIZE);

    int flRunCode = Fl::run();
    ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
    return flRunCode;

}

