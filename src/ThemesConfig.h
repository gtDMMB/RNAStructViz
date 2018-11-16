/* ThemesConfig.h : Definitions and static themes for use in the application;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.11.15
 */

#ifndef __THEMES_CONFIG_H__
#define __THEMES_CONFIG_H__

#include <FL/Enumerations.H>
#include <FL/Fl_Text_Display.H>

#include "ConfigOptions.h"

/*
 * See the following link for the built-in FLTK indexed colors:
 * http://www.fltk.org/doc-1.3/drawing.html#drawing_colors
 */
static const Fl_Text_Display::Style_Table_Entry TEXT_BUFFER_STYLE_TABLE[] = {
     
     {36,               FL_SCREEN_BOLD,        12}, // A -- default
     {60,               FL_SCREEN_BOLD,        12}, // B -- pair A
     {160,              FL_SCREEN_BOLD,        12}, // C -- pair C
     {93,               FL_SCREEN_BOLD,        12}, // D -- pair G
     {80,               FL_SCREEN_BOLD,        12}, // E -- pair U
     {223,              FL_SCREEN_BOLD,        12}, // F -- first pairing
     {219,              FL_SCREEN_BOLD,        12}, // G -- second pairing
     {0,                0,                     0},  // NULL end of array

};

typedef struct {
     Fl_Color windowBGColor;
     Fl_Color widgetBGColor;
     Fl_Color widgetTextColor;
     Fl_Color printTextColor;
     Fl_Color ctFileDisplayColor;
     const char *themeName;
     bool isValid;
} ColorTheme_t;

static const ColorTheme_t PRESET_COLOR_THEMES[] = {
     {
	  Lighter(Fl::get_color(91), 0.95f), 
	  Fl::get_color(62), 
	  Fl::get_color(58), 
	  Fl::get_color(18), 
	  Fl::get_color(132), 
	  "Green On Orange",
	  true
     },
     {
          0x16161600, 
	  0x39393900, 
	  0x00da0000, 
	  0xd3d3b200,
          0x16161600, 
	  "Green On Black", 
	  true
     }, 
     {
          0, 
	  0, 
	  0, 
	  0, 
	  0, 
	  "-- Select Theme--", 
	  false
     }
};

#endif
