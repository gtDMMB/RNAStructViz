/* RNAStructVizTypes.h : Quick pre-instantiation listing of core class names, typedefs, and related data 
 *                       for the RNAStructViz application;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.27
 */

#ifndef __RNASTRUCTVIZ_TYPES_H__
#define __RNASTRUCTVIZ_TYPES_H__

#include <FL/Enumerations.H>

/* Core application data structures and widget subclasses: */
#ifndef RNASTRUCTVIZ_H
    class RNAStructViz;
#endif
#ifndef MAINWINDOW_H
    class MainWindow;
#endif
#ifndef FOLDERWINDOW_H
    class FolderWindow;
#endif
#ifndef FOLDERSTRUCTURE_H
    class Folder;
#endif
#ifndef RNASTRUCTURE_H
class RNAStructure;
#endif

/* Other smaller-order types defined, and configuration settings includes: */
#include "BuildTargetInfo.h"

typedef struct {
     Fl_Color windowBGColor;
     Fl_Color widgetBGColor;
     Fl_Color widgetTextColor;
     Fl_Color printTextColor;
     Fl_Color ctFileDisplayColor;
     const char *themeName;
     bool isValid;
     volatile Fl_Color *bwImageAvgColor;
} ColorTheme_t;

typedef enum {
     FILETYPE_CT, 
     FILETYPE_NOPCT,
     FILETYPE_DOTBRACKET, 
     FILETYPE_BPSEQ,
     FILETYPE_GTB,
     FILETYPE_HLXTRIPLE,
     FILETYPE_RSVLOCALXML,
     FILETYPE_FASTA_ONLY,
     FILETYPE_NONE,
} InputFileTypeSpec;

#endif
