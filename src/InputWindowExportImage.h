/* InputWindowExportImage.h : Subclass of InputWindow to handle selecting the image type users 
 *                            wants to export a cairo buffer to (and relevant image type definitions);
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2020.01.17
 */

#ifndef __INPUTWINDOW_EXPORTIMAGE_H__
#define 

#include "InputWindow.h" 

typedef enum {
     IMGTYPE_OTHER    = 0, 
     IMGTYPE_PNG      = 1, 
     IMGTYPE_JPEG     = 2, 
     IMGTYPE_GIF      = 3, 
     IMGTYPE_SVG      = 4, 
     IMGTYPE_CSRCHDR  = 5, 
} ImageType_t;

typedef struct {
     ImageType_t imgTypeID;
     const char *imgTypeDesc;
     const char *imgTypeLongDesc;
     const char *imgTypeFileExt;
     bool defaultEnabled;
} ImageTypeSpec_t;

static inline const ImageTypeSpec_t IMAGE_TYPE_SPECS[] = {
     { IMGTYPE_PNG,     "PNG",     "PNG     -- Standard and portable with transparency",                             "png", true }, 
     { IMGTYPE_JPEG,    "JPEG",    "JPEG    -- Less standard, but portable",                                         "jpg", false }, 
     { IMGTYPE_GIF,     "GIF",     "GIF     -- Less standard, but still portable",                                   "gif", false }, 
     { IMGTYPE_SVG,     "SVG",     "SVG     -- Standard, text-based, and easy to import",                            "svg", true }, 
     { IMGTYPE_CSRCHDR, "CSRCHDR", "CSRCHDR -- GIMP's output format style with C-char pixel buffers encoded in hex", "c",   false }, 
};

class InputWindowExportImage : public InputWindow {

     protected:
          typedef enum {
               CBNAME_OVERWRITE_DISK        = 0, 
	       CBNAME_UPDATE_SETTINGS       = 1, 
	       CBNAME_INCLUDE_FOOTER_DATA   = 2, 
	       CBNAME_WRITE_FOOTER_DATA_TXT = 3, 
	  } LocalCheckboxName_t;

	  Fl_Check_Button *cbOverwriteSetting;
	  Fl_Check_Button *updateCfgSetting;
	  Fl_Check_Button *includeFooterSetting;
	  Fl_Check_Button *writeFooterTextSetting;
	  
	  Fl_Box   *outputFileDirLabel;
	  Fl_Input *outputFileDirText;
          Fl_Box   *outputFileBasenameLabel;


};


#endif
