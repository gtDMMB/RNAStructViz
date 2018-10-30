/* DisplayConfigWindow.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_RGB_Image.H>

#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

#include "pixmaps/ConfigPathsIcon.c"
#include "pixmaps/ConfigThemesIcon.c"

/* Setup initial definitions of the extern'ed variables here: */
#ifndef _SETUP_GLOBAL_EXTERNS_
#define _SETUP_GLOBAL_EXTERNS_
     
     char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE]; 
     char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
     char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
     char FLTK_THEME[MAX_BUFFER_SIZE]; 

     const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT] = { 
     	     "local",
	     "base", 
	     "plastic", 
	     "gtk+", 
	     "gleam",
	     "custom"
     }; 

     const char *FLTK_THEME_HELP[FLTK_THEME_COUNT] = {
             "A sane choice for the local RNAStructViz application", 
	     "Default old Windows (95/98/Me/NT/2000), old GTK/KDE", 
	     "Aqua theme inspired controls like in Max OSX", 
	     "RedHat Bluecurve like theme", 
	     "Scheme inspired by the Clearlooks Glossy theme",
	     "Something else (user defined)"
     };

     char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

     Fl_Color GUI_WINDOW_BGCOLOR;
     Fl_Color GUI_BGCOLOR;
     Fl_Color GUI_BTEXT_COLOR;
     Fl_Color GUI_TEXT_COLOR;

#endif

bool DisplayConfigWindow::SetupInitialConfig() { 

     strncpy(CTFILE_SEARCH_DIRECTORY, DEFAULT_CTFILE_SEARCH_DIRECTORY, 
             MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(CTFILE_SEARCH_DIRECTORY); 
     strncpy(PNG_OUTPUT_DIRECTORY, DEFAULT_PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString(PNG_OUTPUT_DIRECTORY);
     strncpy(PNG_OUTPUT_PATH, DEFAULT_PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(PNG_OUTPUT_PATH);
     strncpy(FLTK_THEME, DEFAULT_FLTK_THEME, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString(FLTK_THEME); 
     snprintf(LIBFLTK_VERSION_STRING, MAX_BUFFER_SIZE - 1, 
              "%d.%d.%d (%g) - API %g", FL_MAJOR_VERSION, FL_MINOR_VERSION, 
	      FL_PATCH_VERSION, FL_VERSION, FL_API_VERSION);
     ConfigParser::nullTerminateString(LIBFLTK_VERSION_STRING); 

     GUI_WINDOW_BGCOLOR = LOCAL_WINDOW_BGCOLOR;
     GUI_BGCOLOR = LOCAL_BGCOLOR;
     GUI_BTEXT_COLOR = LOCAL_BUTTON_COLOR;
     GUI_TEXT_COLOR = LOCAL_TEXT_COLOR;

     ConfigParser cfgParser(USER_CONFIG_PATH);
     cfgParser.storeVariables();      
     
     return true;

}

DisplayConfigWindow::DisplayConfigWindow() : 
     Fl_Cairo_Window(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT),
     finished(false) { 

     label(CONFIG_WINDOW_TITLE);
     color(GUI_WINDOW_BGCOLOR); 
     size_range(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT);
     set_draw_cb(Draw); 
     callback(WindowCloseCallback);

     imageStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, CONFIG_WINDOW_WIDTH);
     imageData = new uchar[imageStride * CONFIG_WINDOW_WIDTH];
     memset(imageData, 0, imageStride * CONFIG_WINDOW_WIDTH);
     cairo_surface_t *crSurface = cairo_image_surface_create_for_data( 
			          imageData, CAIRO_FORMAT_ARGB32, 
                                  CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT, 
				  imageStride);
     crDraw = cairo_create(crSurface);   
     Fl::cairo_cc(crDraw, false);
     
     // place the widgets in the window:
     int workingYOffset = CFGWIN_WIDGET_OFFSETY + CFGWIN_SPACING;
     
     themesIcon = new Fl_RGB_Image(ConfigThemesIcon.pixel_data, 
		  ConfigThemesIcon.width, ConfigThemesIcon.height, 
		  ConfigThemesIcon.bytes_per_pixel);
     Fl_Box *themesIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
		             themesIcon->w(), themesIcon->h());
     themesIconBox->image(themesIcon);
     windowWidgets.push_back(themesIconBox);
     Fl_Box *themesDescLabel = new Fl_Box(CFGWIN_WIDGET_OFFSETX + 
		               ConfigThemesIcon.width + CFGWIN_SPACING, 
		               workingYOffset, CFGWIN_LABEL_WIDTH, themesIcon->h(), 
			       "Application Theme Settings:");
     themesDescLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     themesDescLabel->labelcolor(GUI_TEXT_COLOR);
     windowWidgets.push_back(themesDescLabel); 
     workingYOffset += themesIcon->h() + CFGWIN_SPACING;
     
} 

DisplayConfigWindow::~DisplayConfigWindow() {
     for(int w = 0; w < windowWidgets.size(); w++) {
          delete windowWidgets[w];
	  windowWidgets[w] = NULL;
     }
     delete themesIcon;
     delete pathsIcon;
}

bool DisplayConfigWindow::ApplyConfigOptions() { return true; }

bool DisplayConfigWindow::isDone() const {
     return finished;
}

void DisplayConfigWindow::drawWidgets() {
     for(int w = 0; w < windowWidgets.size(); w++) {
          windowWidgets[w]->redraw();
     }
}

void DisplayConfigWindow::Draw(Fl_Cairo_Window *crWin, cairo_t *cr) {

    DisplayConfigWindow *thisWin = (DisplayConfigWindow *) crWin;
    cairo_set_source_rgb(thisWin->crDraw, 
		         GetRed(GUI_WINDOW_BGCOLOR) / 255.0f,
		         GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
			 GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
    cairo_scale(thisWin->crDraw, thisWin->w(), thisWin->h());
    cairo_fill(thisWin->crDraw);
    thisWin->drawWidgets();

}

void DisplayConfigWindow::ThemeButtonCallback(Fl_Button *rb, void *userData) {
     
     const char *themeName = ALL_FLTK_THEMES[rb->value()];
     if(Fl::is_scheme(themeName)) { 
          Fl::scheme(themeName);
     }

}

void DisplayConfigWindow::WindowCloseCallback(Fl_Widget *win, void *udata) {
     DisplayConfigWindow *thisWin = (DisplayConfigWindow *) win;
     thisWin->finished = true;
}
