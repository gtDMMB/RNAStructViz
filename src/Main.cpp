#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <FL/Fl.H>

#include "MainWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "DisplayConfigWindow.h"

int main(int argc, char **argv) {

    if(argc > 1 && !strcmp(argv[1], "--about")) {
        ApplicationBuildInfo::PrintAboutListing(stdout);
        return 0;
    }
	
    DisplayConfigWindow::SetupInitialConfig();
    RNAStructViz::Initialize(argc, argv);
    
    Fl::own_colormap();
    uchar rc, bc, gc;
    Fl::get_color(LOCAL_FGCOLOR, rc, bc, gc); 
    Fl::foreground(rc, bc, gc);
    Fl::get_color(LOCAL_BGCOLOR, rc, bc, gc); 
    Fl::background(rc, bc, gc);
    Fl::get_color(LOCAL_BG2COLOR, rc, bc, gc); 
    Fl::background2(rc, bc, gc);
    
    fl_color(GUI_WINDOW_BGCOLOR);     
    fl_font(LOCAL_RMFONT, LOCAL_TEXT_SIZE);

    return Fl::run();

}

