#include "MainWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "DisplayConfigWindow.h"

#include <FL/Fl.H>

int main(int argc, char **argv)
{
    RNAStructViz::Initialize(argc, argv);
    DisplayConfigWindow::SetupInitialConfig();
    Fl::scheme(FLTK_THEME);

    return Fl::run();
}

