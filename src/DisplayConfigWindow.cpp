/* DisplayConfigWindow.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Group.H>

#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

/* Setup initial definitions of the extern'ed variables here: */
#ifndef _SETUP_GLOBAL_EXTERNS_
#define _SETUP_GLOBAL_EXTERNS_
     char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE]; 
     char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
     char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
     char FLTK_THEME[MAX_BUFFER_SIZE]; 

     const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT] = { 
     	     "base", 
	     "plastic", 
	     "gtk+", 
	     "gleam"
     }; 

     const char *FLTK_THEME_HELP[FLTK_THEME_COUNT] = {
             "Default old Windows (95/98/Me/NT/2000), old GTK/KDE", 
	     "Aqua theme inspired controls like in Max OSX", 
	     "RedHat Bluecurve like theme", 
	     "Scheme inspired by the Clearlooks Glossy theme"
     };
#endif

bool DisplayConfigWindow::SetupInitialConfig() { 

     snprintf(CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1, 
              DEFAULT_CTFILE_SEARCH_DIRECTORY);
     ConfigParser::nullTerminateString(CTFILE_SEARCH_DIRECTORY); 
     snprintf(PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1, 
              DEFAULT_PNG_OUTPUT_DIRECTORY); 
     ConfigParser::nullTerminateString(PNG_OUTPUT_DIRECTORY);
     snprintf(PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1, DEFAULT_PNG_OUTPUT_PATH);
     ConfigParser::nullTerminateString(PNG_OUTPUT_PATH);
     snprintf(FLTK_THEME, MAX_BUFFER_SIZE - 1, DEFAULT_FLTK_THEME); 
     ConfigParser::nullTerminateString(FLTK_THEME); 

     ConfigParser cfgParser(USER_CONFIG_PATH);
     cfgParser.storeVariables();      
     
     return true;

}

DisplayConfigWindow::DisplayConfigWindow() : 
     Fl_Cairo_Window(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT) { 

     Fl::visual(FL_RGB);
     label(CONFIG_WINDOW_TITLE);
     color(FL_WHITE); 
     size_range(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT);
     box(FL_NO_BOX);

     Fl_Box *appThemesLabel = new Fl_Box(CFGWIN_WIDGET_OFFSETX, 
            CFGWIN_WIDGET_OFFSETY, CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
	    "@FLTK Application Theme:\nMouseover to see theme descriptions.");
     appThemesLabel->labelfont(FL_COURIER_BOLD_ITALIC);
     appThemesLabel->labelsize(32);
     appThemesLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

     int workingYOffset = CFGWIN_WIDGET_OFFSETY + CFGWIN_LABEL_HEIGHT + 
	                  CFGWIN_SPACING;
     int radioButtonHeight = 30; 
     Fl_Group *themeRadioButtons = new Fl_Group(CFGWIN_WIDGET_OFFSETX, 
	      workingYOffset, CFGWIN_LABEL_WIDTH, 
	      FLTK_THEME_COUNT * (radioButtonHeight + CFGWIN_SPACING));
     {
          for(int t = 0; t < FLTK_THEME_COUNT; t++) { 
               Fl_Round_Button *rb = new Fl_Round_Button( 
			       CFGWIN_WIDGET_OFFSETX + CFGWIN_SPACING, 
			       workingYOffset += CFGWIN_SPACING, 
			       CFGWIN_LABEL_WIDTH - 2 * CFGWIN_SPACING, 
			       radioButtonHeight, 
			       ALL_FLTK_THEMES[t]);
	       rb->tooltip(FLTK_THEME_HELP[t]);
	       rb->type(102);
	       rb->down_box(FL_ROUND_DOWN_BOX); 
	       rb->callback((Fl_Callback *) ThemeButtonCallback); 
	       rb->value(t); 
	  }
     } 
     


} 

DisplayConfigWindow::~DisplayConfigWindow() {}

bool DisplayConfigWindow::ApplyConfigOptions() { return true; }

bool DisplayConfigWindow::CloseWindow() { return true; } 

void DisplayConfigWindow::ThemeButtonCallback(Fl_Button *rb, void *userData) {
     
     const char *themeName = ALL_FLTK_THEMES[rb->value()];
     if(Fl::is_scheme(themeName)) { 
          Fl::scheme(themeName);
     }

}
