/* DisplayConfigWindow.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"

/* Setup initial definitions of the extern'ed variables here: */
#ifndef _SETUP_GLOBAL_EXTERNS_
#define _SETUP_GLOBAL_EXTERNS_
     char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE]; 
     char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
     char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
     char FLTK_THEME[MAX_BUFFER_SIZE]; 

     const char *ALL_FLTK_THEMES[5] = { 
     	     "base", 
	     "plastic", 
	     "gtk+", 
	     "gleam"
     }; 

     const char *FLTK_THEME_HELP[5] = {
             "Default old Windows (95/98/Me/NT/2000), old GTK/KDE", 
	     "Aqua theme inspired controls like in Max OSX", 
	     "RedHat Bluecurve like theme", 
	     "Scheme inspired by the Clearlooks Glossy theme"
     };
#endif

bool DisplayConfigWindow::SetupInitialConfig() { 

     snprintf(CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1, 
              DEFAULT_CTFILE_SEARCH_DIRECTORY);
     snprintf(PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1, 
              DEFAULT_PNG_OUTPUT_DIRECTORY); 
     snprintf(PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1, DEFAULT_PNG_OUTPUT_PATH);
     snprintf(FLTK_THEME, MAX_BUFFER_SIZE - 1, DEFAULT_FLTK_THEME); 
     return true;
}

DisplayConfigWindow::DisplayConfigWindow() : 
     Fl_Cairo_Window(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT) { 



} 

DisplayConfigWindow::~DisplayConfigWindow() {}

bool DisplayConfigWindow::ApplyConfigOptions() { return true; }

bool DisplayConfigWindow::CloseWindow() { return true; } 

bool DisplayConfigWindow::ChangeConfigWindowTheme(const char *themeName) { 
		return true; 
}

