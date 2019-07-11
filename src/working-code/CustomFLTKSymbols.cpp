/* CustomFLTKSymbols.cpp : Implementation of the custom symbols core;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.02
 */

#include <FL/Fl_RGB_Image.H>

#include "CustomFLTKSymbols.h"

static const CustomSymbolData_t LOCAL_CUSTOM_FLTK_SYMBOLS[] = {
     {"CustomFolderGraphic", NewFolderDrawItFunc, 1, true}, 
};

bool LoadAllCustomFLTKSymbols() { 
     for(int sidx = 0; sidx < GetArrayLength(LOCAL_CUSTOM_FLTK_SYMBOLS); sidx++) { 
          const CustomSymbolData_t *symbolData = &(LOCAL_CUSTOM_FLTK_SYMBOLS[sidx]);
	  if(symbolData->include && 
	     !fl_add_symbol(symbolData->symbolName, symbolData->drawItFunc, 
		            symbolData->scales)) { 
		  fprintf(stderr, "ERROR: Adding handler for custom symbol \"%s\ ...", 
			  symbolData->symbolName);
		  return false;
	  }
     }
     return true;
}

void DrawFLTKSymbolFromPixelData(CPixelData_t *cpd, Fl_Color flColor, 
		                 bool transform = true) { 
     
     Fl_RGB_Image *rgbImage = new Fl_RGB_Image((unsigned char *) cpd->pixel_data, 
		                               cpd->width, cpd->height, 
					       cpd->bytes_per_pixel);
     if(rgbImage == NULL) {
          return;
     }
     else if(transform) {// transform the image before drawing:
          rgbImage->desaturate();
	  rgbImage->color_average(flColor, 0.5);
     }
     rgbImage->draw(0, 0, cpd->width, cpd->height);
     Delete(rgbImage);

}

void NewFolderDrawItFunc(Fl_Color flColor) { 
     DrawFLTKSymbolFromPixelData(&FLTKSymbolFolder, flColor, true);
}
