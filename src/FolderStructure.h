#ifndef FOLDERSTRUCTURE_H
#define FOLDERSTRUCTURE_H

#include <FL/Fl_Button.H>

#include "FolderWindow.h"
#include "ConfigOptions.h"

class Folder {

  public:
    char *folderName;
    char *folderNameFileCount;
    int  *folderStructs;
    int  structCount;
    bool selected;
    
    FolderWindow *folderWindow;
    
    Fl_Button *mainWindowFolderBtn;
    static const char *tooltipTextFmt;
    char tooltipText[MAX_BUFFER_SIZE];

};

#endif
