/* AutoloadIndicatorButton.h : Subclass of Fl_Button that supports toggling of whether a given 
 *                             structure is contained in the autoload directory at start up;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.28
 */

#ifndef __AUTOLOAD_INDICATOR_BUTTON_H__
#define __AUTOLOAD_INDICATOR_BUTTON_H__

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <string>

#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>

#include "pixmaps/LinkSetIcon.c"
#include "pixmaps/LinkUnsetIcon.c"

#include "RNAStructVizTypes.h"
#include "ThemesConfig.h"
#include "ConfigExterns.h"

#define AUTOLOAD_BUTTON_SIZE           (24)

class AutoloadIndicatorButton : public Fl_Button {

     public:
	  AutoloadIndicatorButton(std::string diskPath, std::string baseNamePath, bool initStatus);

     private:
          std::string filePathOnDisk;
	  std::string filePathBaseName;
	  bool isAutoloadedState;

	  void ResetButton(int stateValue);

	  static void ToggleAutoloadStatusCallback(Fl_Widget *btn, void *udata);

     protected:
          static inline Fl_RGB_Image *LINK_SET_ICON = new Fl_RGB_Image(
               LinkSetIcon.pixel_data, 
	       LinkSetIcon.width, 
	       LinkSetIcon.height,
	       LinkSetIcon.bytes_per_pixel
	  );
          static inline Fl_RGB_Image *LINK_UNSET_ICON = new Fl_RGB_Image(
               LinkUnsetIcon.pixel_data, 
	       LinkUnsetIcon.width, 
	       LinkUnsetIcon.height,
	       LinkUnsetIcon.bytes_per_pixel
	  );
	  
     public:
	  static inline void InitStaticData() {
               if(isStaticDataInit) {
	            return;
	       }
	       LINK_SET_ICON->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
	       LINK_UNSET_ICON->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.25);
               isStaticDataInit = true;
	  }

     protected:
	  static inline bool isStaticDataInit = false;
	  static inline const char *linkSetTooltipText = "@> : AUTOLOADING STRUCTURE";
	  static inline const char *linkUnsetTooltipText = "@1+ : NOT AUTOLOADING STRUCTURE";

};

#endif
