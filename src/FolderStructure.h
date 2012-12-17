#ifndef FOLDERSTRUCTURE_H
#define FOLDERSTRUCTURE_H

#include "FolderWindow.h"

struct Folder{
    char* folderName;
    char* folderNameFileCount;
    int* folderStructs;
    int structCount;
    bool selected;
    FolderWindow* folderWindow;
    };

#endif
