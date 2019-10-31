/* ConfigOptions.h : Header for the user-configurable program options; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com) 
 * Created: 2018.10.20
 */

#ifndef _CONFIG_OPTIONS_H_
#define _CONFIG_OPTIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

#include <string>
using std::string;

#include "BuildTargetInfo.h"
#include "CairoDrawingUtils.h"

#ifndef PERFORM_BRANCH_TYPE_ID
     #define PERFORM_BRANCH_TYPE_ID          (false)
#endif

#define MAX_BUFFER_SIZE                 (768)
#define MAX_SEQUENCE_SIZE               (12000)

#define DEFAULT_CTFILE_SEARCH_DIRECTORY (GetUserHome())
#define DEFAULT_PNG_OUTPUT_DIRECTORY    (GetUserHome())
#define DEFAULT_PNG_OUTPUT_PATH         ("RNAStructViz-GUIView-%F-%H%M%S.png") 
                                        /* As a strftime format string */
#define DEFAULT_FLTK_THEME              ("gtk+")
#define FLTK_THEME_COUNT                (6)
#define USER_CONFIG_DIR                 ((string(GetUserHome()) + string("/.RNAStructViz/")).c_str())
#define USER_CONFIG_PATH                ((USER_CONFIG_DIR + string("config.cfg")).c_str())
#define USER_SAMPLE_STRUCTS_BASE_PATH   (string(GetUserHome()) + string("/RNAStructViz"))
#define USER_SAMPLE_STRUCTS_PATH        (USER_SAMPLE_STRUCTS_BASE_PATH + string("/sample_structures"))

extern volatile char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE];
extern volatile char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
extern volatile char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern volatile char FLTK_THEME[MAX_BUFFER_SIZE];
extern const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT];
extern const char *FLTK_THEME_HELP[FLTK_THEME_COUNT];
extern char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

#define MAX(x, y)                    ((x) <= (y) ? (y) : (x))
#define MIN(x, y)                    ((x) <= (y) ? (x) : (y))

/* Some basic color operations for getting shades of color: */
#define RGBColor(r, g, b)               ColorUtil::GetRGBColor(r, g, b)
#define Lighter(color, alpha)           ColorUtil::Lighter(color, alpha)
#define Darker(color, alpha)            ColorUtil::Darker(color, alpha)
#define Inactive(color)                 ColorUtil::Inactive(color)
#define Contrast(color)                 ColorUtil::Contrast(color)
#define GetRed(flc)                     ColorUtil::RGBGetRed(flc)
#define GetGreen(flc)                   ColorUtil::RGBGetGreen(flc)
#define GetBlue(flc)                    ColorUtil::RGBGetBlue(flc) 

/* Local "theme" defines for RNAStructViz: */
#define LOCAL_WINDOW_BGCOLOR            (0xffffff00)
#define LOCAL_FGCOLOR                   (FL_DARK1)
#define LOCAL_BG2COLOR                  (FL_LIGHT2)
#define LOCAL_HIGHLIGHT_COLOR           (FL_LIGHT3)
#define LOCAL_BGCOLOR                   (RGBColor(123, 77, 211))
#define LOCAL_BUTTON_COLOR              (Lighter(RGBColor(210, 194, 240), 0.5f))
#define LOCAL_TEXT_COLOR                (RGBColor(52, 25, 102))
#define LOCAL_TEXT_SIZE                 (12)
#define LOCAL_RMFONT                    (FL_SCREEN_BOLD)
#define LOCAL_BFFONT                    (FL_SCREEN_BOLD)

extern volatile Fl_Color GUI_WINDOW_BGCOLOR;
extern volatile Fl_Color GUI_BGCOLOR;
extern volatile Fl_Color GUI_BTEXT_COLOR;
extern volatile Fl_Color GUI_TEXT_COLOR;
extern volatile Fl_Color GUI_CTFILEVIEW_COLOR;

extern volatile Fl_Color STRUCTURE_DIAGRAM_COLORS[3][7];

extern bool GUI_USE_DEFAULT_FOLDER_NAMES;
extern bool GUI_KEEP_STICKY_FOLDER_NAMES;
extern bool DEBUGGING_ON;
extern bool DISPLAY_FIRSTRUN_MESSAGE;

extern char rnaStructVizExecPath[MAX_BUFFER_SIZE];
extern char runtimeCWDPath[MAX_BUFFER_SIZE];
extern char activeSystemUserFromEnv[MAX_BUFFER_SIZE];

/* Helper functions and inline utilities we do not place elsewhere: */
#include "RNACUtils.cpp"

#endif
