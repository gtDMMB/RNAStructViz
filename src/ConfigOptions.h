/* ConfigOptions.h : Header for the user-configurable program options; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com) 
 * Created: 2018.10.20
 */

#ifndef _CONFIG_OPTIONS_H_
#define _CONFIG_OPTIONS_H_

#include <FL/Enumerations.H>

#define RNASTRUCTVIZ_VERSION_STRING    ("RNAStructViz -- v1.0")

#define FLTK_USE_CAIRO 1
#define FLTK_HAVE_CAIRO 1

#define MAX_BUFFER_SIZE                 (256)

#define DEFAULT_CTFILE_SEARCH_DIRECTORY ("./")
#define DEFAULT_PNG_OUTPUT_DIRECTORY    ("./")
#define DEFAULT_PNG_OUTPUT_PATH         ("RNAStructViz-GUIView-%F-%H%M%S.png") 
                                        /* As a strftime format string */
#define DEFAULT_FLTK_THEME              ("local")
#define FLTK_THEME_COUNT                (6)
#define USER_CONFIG_PATH                ("~/.RNAStructViz/config.cfg")

/* Local "theme" defines for RNAStructViz: */
#define LOCAL_FGCOLOR                   (FL_DARK1)
#define LOCAL_BGCOLOR                   (FL_DARK_CYAN)
#define LOCAL_BG2COLOR                  (FL_LIGHT2)
#define LOCAL_BUTTON_COLOR              (FL_LIGHT2)
#define LOCAL_TEXT_COLOR                (FL_DARK1)
#define LOCAL_TEXT_SIZE                 (24)
#define LOCAL_RMFONT                    (FL_SCREEN)
#define LOCAL_BFFONT                    (FL_SCREEN_BOLD)

extern char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern char FLTK_THEME[MAX_BUFFER_SIZE];
extern const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT];
extern const char *FLTK_THEME_HELP[FLTK_THEME_COUNT];
extern char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

#endif
