#include <stdlib.h>
#include <string.h>

#include <FL/Fl.H>

#include "MainWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "DisplayConfigWindow.h"

int main(int argc, char **argv)
{
    RNAStructViz::Initialize(argc, argv);
    DisplayConfigWindow::SetupInitialConfig();
    Fl::own_colormap();
    if(!strcmp(FLTK_THEME, "local")) { 
	 uchar rc, bc, gc;
	 Fl::get_color(LOCAL_FGCOLOR, rc, bc, gc); 
	 Fl::foreground(rc, bc, gc);
         Fl::get_color(LOCAL_BGCOLOR, rc, bc, gc); 
	 Fl::background(rc, bc, gc);
         Fl::get_color(LOCAL_BG2COLOR, rc, bc, gc); 
	 Fl::background2(rc, bc, gc);
    }
    else {
         Fl::scheme(FLTK_THEME);
    }
    //fl_font(FL_SCREEN, 18);

    return Fl::run();
}

