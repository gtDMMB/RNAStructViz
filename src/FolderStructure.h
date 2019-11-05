#ifndef FOLDERSTRUCTURE_H
#define FOLDERSTRUCTURE_H

#include "FolderWindow.h"

#ifdef __cplusplus
     extern "C" {
#endif

typedef struct {
    char* folderName;
    char* folderNameFileCount;
    int* folderStructs;
    int structCount;
    bool selected;
    FolderWindow* folderWindow;
} Folder;

#ifdef __cplusplus
} // extern "C"
#endif

#endif
