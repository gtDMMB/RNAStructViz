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
          0x16161600, 
	  0x39393900, 
	  0x00da0000, 
	  0xd3d3b200,
          0x16161600, 
	  "Green On Black", // standard Linux / command line colors
	  true
     }, 
     {
          0xc3b68900, 
	  0x00305700, 
	  0xffffee00, 
	  0x0f0f0e00,
          Darker(0xc3b68900, 0.5f), 
	  "Blue on Gold", 
	  true
     }, 
     {
          0xeeb50000,
	  0xff6d0000,
	  0x3e3c4000,
	  0x604f0900,
	  Darker(0xeeb50000, 0.7f), 
	  "Sunshine", // very vibrant and bright
	  true
     },
     {
          0x98a8a800,
	  0x3a6ea500,
	  0xdffff300,
	  0x24241f00,
	  Darker(0x98a8a800, 0.65f), 
	  "Redmond", // old school Windows 2000 era color scheme
	  true
     },
     {
          0xf7efd400,
	  0xefda8d00,
	  0x0f0f0e00,
          0x0f0f0e00,
	  0xf7efd400, 
	  "Parchment",  // black and off white to gold 
	  true
     },
     {
          0xf7f7f900,
	  0xcfd2d600,
	  0x00000000,
	  0x00000000,
	  0xf7f7f900, 
	  "Gray on white", 
	  true
     },
     {
          0xeff2f200,
	  0xd1d0d800,
	  0x0e014400,
          0x0e0144,
	  0xeff2f200, 
	  "Structviz default", 
	  true
     },
    {
          0, 
	  0, 
	  0, 
	  0, 
	  0, 
	  "-- Select Theme --", 
	  false
     }
};

#endif
