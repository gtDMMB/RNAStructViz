/* RNAStructVizTypes.h : Quick pre-instantiation listing of core class names, typedefs, and related data 
 *                       for the RNAStructViz application;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.27
 */

#ifndef __RNASTRUCTVIZ_TYPES_H__
#define __RNASTRUCTVIZ_TYPES_H__

#include <FL/Enumerations.H>

/* Core application data structures and widget subclasses: */
class RNAStructViz;
class StructureManager;
class MainWindow;
class Folder;
class FolderWindow;
class RNAStructure;
class StructureData;

/* Other smaller-order types defined, and configuration settings includes: */
#include "BuildInclude/BuildTargetInfo.h"

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

typedef enum {
     OSTYPE_UNKNOWN          = 0, 
     OSTYPE_MACOSX_DARWIN    = 1, 
     OSTYPE_LINUX_GENERIC    = 2, 
     OSTYPE_BSDUNIX_GENERIC  = 4, 
     OSTYPE_OPENBSD          = 8,
     OSTYPE_FREEBSD          = 32, 
     OSTYPE_GNUHURD          = 64, 
     OSTYPE_UNIX_GENERIC     = 128, 
     OSTYPE_WINDOWS          = 256, 
     OSTYPE_UNIX_ALL         = 0xffff & ~0x100, 
     OSTYPE_ALL              = 0xffff,
} OSPlatformType_t; 

/* Custom widget classes: */
class AutoloadIndicatorButton;
class XMLExportButton;

#endif
