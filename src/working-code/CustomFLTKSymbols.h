/* CustomFLTKSymbols.h : Define the mechanisms to implement our custom 
 *                       FLTK @... symbols in strings and box labels;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.02
 */

#ifndef __CUSTOM_FLTK_SYMBOLS__
#define __CUSTOM_FLTK_SYMBOLS__

#include <FL/fl_draw.H>
#include <FL/x.H>

#include "ConfigOptions.h"

typedef struct {
     unsigned int width;
     unsigned int height;
     unsigned int bytes_per_pixel;
     const char *pixel_data;
} CPixelData_t;

#include "pixmaps/FLTKSymbolFolder.c"

typedef struct { 
     const char *symbolName;
     void (*drawItFunc)(Fl_Color);
     int scales;
     bool include;
} CustomSymbolData_t;

bool LoadAllCustomFLTKSymbols();

void DrawFLTKSymbolFromPixelData(CPixelData_t *cpd, Fl_Color flColor);
void NewFolderDrawItFunc(Fl_Color flColor);

#endif
