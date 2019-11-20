/* ConfigExterns.h : A place to put (most of) the configuration setting extern'ed variables; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.20
 */

#ifndef __CONFIG_EXTERNS_H__
#define __CONFIG_EXTERNS_H__

extern volatile char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE];
extern volatile char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
extern volatile char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern volatile char PNG_RADIAL_LAYOUT_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern volatile char FLTK_THEME[MAX_BUFFER_SIZE];
extern const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT];
extern const char *FLTK_THEME_HELP[FLTK_THEME_COUNT];
extern volatile char LOCAL_THEME_NAME[MAX_BUFFER_SIZE];
extern char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

extern volatile Fl_Color GUI_WINDOW_BGCOLOR;
extern volatile Fl_Color GUI_BGCOLOR;
extern volatile Fl_Color GUI_BTEXT_COLOR;
extern volatile Fl_Color GUI_TEXT_COLOR;
extern volatile Fl_Color GUI_CTFILEVIEW_COLOR;

extern volatile Fl_Color STRUCTURE_DIAGRAM_COLORS[3][7];

extern bool GUI_USE_DEFAULT_FOLDER_NAMES;
extern bool GUI_KEEP_STICKY_FOLDER_NAMES;
extern int  DEBUGGING_ON;
extern bool DISPLAY_FIRSTRUN_MESSAGE;

extern char rnaStructVizExecPath[MAX_BUFFER_SIZE];
extern char runtimeCWDPath[MAX_BUFFER_SIZE];
extern char activeSystemUserFromEnv[MAX_BUFFER_SIZE];

#endif
