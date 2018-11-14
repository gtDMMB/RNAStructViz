/* ConfigOptions.h : Header for the user-configurable program options; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com) 
 * Created: 2018.10.20
 */

#ifndef _CONFIG_OPTIONS_H_
#define _CONFIG_OPTIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/x.H>

#include <string>
using std::string;

#include "BuildTargetInfo.h"

#define PERFORM_BRANCH_TYPE_ID          (false)
#define MAX_BUFFER_SIZE                 (256)

static inline const char * GetUserHome() {
     const char *userHomeDir = getenv("HOME");
     if(userHomeDir == NULL) {
          struct passwd *uhdPasswd = getpwuid(getuid());
	  if(uhdPasswd) 
               userHomeDir = uhdPasswd->pw_dir;
	  else
	       userHomeDir = "";
     }
     return userHomeDir;
}

#define DEFAULT_CTFILE_SEARCH_DIRECTORY (GetUserHome())
#define DEFAULT_PNG_OUTPUT_DIRECTORY    (GetUserHome())
#define DEFAULT_PNG_OUTPUT_PATH         ("RNAStructViz-GUIView-%F-%H%M%S.png") 
                                        /* As a strftime format string */
#define DEFAULT_FLTK_THEME              ("gleam")
#define FLTK_THEME_COUNT                (6)
#define USER_CONFIG_DIR                 ((string(GetUserHome()) + string("/.RNAStructViz/")).c_str())
#define USER_CONFIG_PATH                ((USER_CONFIG_DIR + string("config.cfg")).c_str())

extern char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern char FLTK_THEME[MAX_BUFFER_SIZE];
extern const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT];
extern const char *FLTK_THEME_HELP[FLTK_THEME_COUNT];
extern char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

/* Some basic color operations for getting shades of color: */
#define RGBColor(r, g, b)               (((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8))
#define Lighter(color, alpha)           (fl_color_average(color, FL_WHITE, alpha))
#define Darker(color, alpha)            (fl_color_average(color, FL_BLACK, alpha))
#define Inactive(color)                 (fl_inactive(color))
#define Contrast(color)                 (fl_contrast(color, FL_WHITE))
#define GetRed(flc)                     ((flc >> 24) & 0x000000ff)
#define GetGreen(flc)                   ((flc >> 16) & 0x000000ff)
#define GetBlue(flc)                    ((flc >> 8) & 0x000000ff)

/* Local "theme" defines for RNAStructViz: */
#define LOCAL_WINDOW_BGCOLOR            (0xffffff00)
#define LOCAL_FGCOLOR                   (FL_DARK1)
#define LOCAL_BG2COLOR                  (FL_LIGHT2)
#define LOCAL_HIGHLIGHT_COLOR           (FL_LIGHT3)
#define LOCAL_BGCOLOR                   (RGBColor(123, 77, 211))
#define LOCAL_BUTTON_COLOR              (Lighter(RGBColor(210, 194, 240), 0.5f))
#define LOCAL_TEXT_COLOR                (RGBColor(52, 25, 102))
#define LOCAL_TEXT_SIZE                 (8)
#define LOCAL_RMFONT                    (FL_COURIER_BOLD_ITALIC)
#define LOCAL_BFFONT                    (FL_COURIER_BOLD_ITALIC)

extern Fl_Color GUI_WINDOW_BGCOLOR;
extern Fl_Color GUI_BGCOLOR;
extern Fl_Color GUI_BTEXT_COLOR;
extern Fl_Color GUI_TEXT_COLOR;

extern bool GUI_USE_DEFAULT_FOLDER_NAMES;

#endif
