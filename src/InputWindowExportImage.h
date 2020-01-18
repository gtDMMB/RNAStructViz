/* InputWindowExportImage.h : Subclass of InputWindow to handle selecting the image type users 
 *                            wants to export a cairo buffer to (and relevant image type definitions);
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2020.01.17
 */

#ifndef __INPUTWINDOW_EXPORTIMAGE_H__
#define __INPUTWINDOW_EXPORTIMAGE_H__

#include <string.h>

#include <string>

#include <cairo.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>

#include "InputWindow.h" 

typedef enum {
     IMGTYPE_OTHER    = 0, 
     IMGTYPE_PNG      = 1, 
     IMGTYPE_SVG      = 2, 
     IMGTYPE_CSRCHDR  = 4, 
} ImageType_t;

typedef struct {
     ImageType_t imgTypeID;
     const char *imgTypeDesc;
     const char *imgTypeLongDesc;
     const char *imgTypeFileExt;
     bool defaultEnabled;
} ImageTypeSpec_t;

static inline const ImageTypeSpec_t IMAGE_TYPE_SPECS[] = {
     { IMGTYPE_PNG,     "PNG",     "PNG -- Standard and portable with transparency",                                 "png", true }, 
     { IMGTYPE_SVG,     "SVG",     "SVG -- Standard, text-based, and easy to import",                                "svg", true }, 
     { IMGTYPE_CSRCHDR, "CSRCHDR", "CSRCHDR -- GIMP's output format style with C-char pixel buffers encoded in hex", "c",   true }, 
};

#define IWIN_WIDTH               (700)
#define IWIN_HEIGHT              (230)
#define IWIN_WIDGET_HEIGHT       (22)
#define IWIN_LABEL_WIDTH         (150)
#define IWIN_INPUT_WIDTH         (440)
#define IWIN_BUTTON_WIDTH        (100)
#define IWIN_SPACING             (10)

class InputWindowExportImage : public InputWindow {

     protected:
          typedef enum {
	       CBNAME_NONE                  = 0,
               CBNAME_OVERWRITE_DISK        = 1, 
	       CBNAME_UPDATE_SETTINGS       = 2, 
	       CBNAME_INCLUDE_FOOTER_DATA   = 3, 
	       CBNAME_WRITE_FOOTER_DATA_TXT = 4, 
	  } LocalCheckboxName_t;

	  Fl_Check_Button *cbOverwriteSetting;
	  Fl_Check_Button *cbUpdateCfgSetting;
	  Fl_Check_Button *cbIncludeFooterSetting;
	  Fl_Check_Button *cbWriteFooterTextSetting;
	  
	  static Fl_Check_Button * GetCheckButtonReference(const InputWindowExportImage * &inputWin, LocalCheckboxName_t cbType);

	  Fl_Box    *mainWinInfoBox;
	  Fl_Box    *outputFileDirLabel;
	  Fl_Input  *outputFileDirText;
          Fl_Box    *outputFileBasenameLabel;
	  Fl_Input  *outputFileBasenameText;
	  Fl_Box    *fileFormatLabel;
	  Fl_Choice *fileFormatDropDownSelector;

	  Fl_Button *okButton;
	  Fl_Button *cancelButton;
	  Fl_Button *openDocsLinkButton;

	  std::string dirPath, basePath;

     public:
	  InputWindowExportImage(std::string fullSavePath);
	  ~InputWindowExportImage();

	  ImageType_t GetImageOutputFormat() const;
	  std::string GetOutputFileSavePath() const;

	  bool WriteImageToFile(cairo_surface_t *imageDataSurface);
	  bool WriteImageToFile(std::string outputPath, ImageType_t imageOutputFormat, cairo_surface_t *imageDataSurface);

     private:
	  static void FileFormatDropDownCallback(Fl_Widget *ddWidget, void *udata);
          static void OKButtonCallback(Fl_Widget *okBtn, void *udata);
	  
	  static int WriteCairoToPNGImage(std::string outputPath, cairo_surface_t *imageDataSurface);
	  static int WriteCairoToSVGImage(std::string outputPath, cairo_surface_t *imageDataSurface);
	  static int WriteCairoToCStyleHeaderFile(std::string outputPath, cairo_surface_t *imageDataSurface);

};


#endif
