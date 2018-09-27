#include "MainWindow.h"
#include "RNAStructViz.h"

#include <FL/Fl.H>

int main(int argc, char **argv)
{
    RNAStructViz::Initialize(argc, argv);
    //Fl::scheme("gtk+");
    Fl::scheme("gleam");

    return Fl::run();
}

