/* ConfigOptions.h : Header for the user-configurable program options; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com) 
 * Created: 2018.10.20
 */

#ifndef _CONFIG_OPTIONS_H_
#define _CONFIG_OPTIONS_H_

#define RNASTRUCTVIZ_VERSION_STRING    ("RNAStructViz -- v1.0")

#define FLTK_USE_CAIRO 1
#define FLTK_HAVE_CAIRO 1

#define MAX_BUFFER_SIZE                 (256)

#define DEFAULT_CTFILE_SEARCH_DIRECTORY ("./")
#define DEFAULT_PNG_OUTPUT_DIRECTORY    ("./")
#define DEFAULT_PNG_OUTPUT_PATH         ("RNAStructViz-GUIView-%F-%H%M%S.png") 
                                        /* As a strftime format string */
#define DEFAULT_FLTK_THEME              ("gleam")
#define FLTK_THEME_COUNT                (4)
#define USER_CONFIG_PATH                ("~/.RNAStructViz/config.cfg")

extern char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
extern char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
extern char FLTK_THEME[MAX_BUFFER_SIZE];
extern const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT];
extern const char *FLTK_THEME_HELP[FLTK_THEME_COUNT];

#endif
