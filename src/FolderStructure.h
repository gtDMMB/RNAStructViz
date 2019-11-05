#ifndef FOLDERSTRUCTURE_H
#define FOLDERSTRUCTURE_H

#include "FolderWindow.h"

typedef struct {
    char* folderName;
    char* folderNameFileCount;
    int* folderStructs;
    int structCount;
    bool selected;
    FolderWindow* folderWindow;
} Folder;

#endif
