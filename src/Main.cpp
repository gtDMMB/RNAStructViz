#include "MainWindow.h"
#include "RNAStructViz.h"

int main(int argc, char **argv)
{
    RNAStructViz::Initialize(argc, argv);

    return Fl::run();
}

