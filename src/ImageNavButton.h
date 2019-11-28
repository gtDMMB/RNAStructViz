/* ImageNavButton.h : Defines a covenient, re-usable interface to (re)creating image buttons in FLTK 
 *                    for use as navigation-like buttons for folders and structures in the GUI. 
 *                    This is basically just a wrapper around the stock FLTK image button creation 
 *                    routines found elsewhere in the code, except that this interface is coded with the 
 *                    intention that these common button types be reused many places in the GUI. 
 *                    Thus we make it easy to specify the image pixel data for the button, 
 *                    custom callback functions and button user_data() specs, etc.; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.27
 */

#ifndef __IMAGE_NAVBUTTONS_H__
#define __IMAGE_NAVBUTTONS_H__

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <string>

#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>

#include "ConfigOptions.h"
#include "ConfigExterns.h"
#include "ThemesConfig.h"

#include "pixmaps/SmallInfoButtonIcon.c"

class ImageNavButton;

class ImagePixelData {

     protected:
          unsigned int width;
          unsigned int height;
          unsigned int bytes_per_pixel;
          unsigned char *pixel_data;

     public:
	  static const ImagePixelData & ConvertFromStructureType(auto &hdrStructInst) {
               ImagePixelData imgPixelData;
	       imgPixelData.width = hdrStructInst.width;
	       imgPixelData.height = hdrStructInst.height;
	       imgPixelData.bytes_per_pixel = hdrStructInst.bytes_per_pixel;
	       imgPixelData.pixel_data = (unsigned char *) hdrStructInst.pixel_data;
	       return imgPixelData;
	  }

	  unsigned int GetHeight() const { return height; }
	  unsigned int GetWidth() const { return width; }
	  unsigned int GetPixelBufferSize() const { return height * width * bytes_per_pixel + 1; }

          Fl_RGB_Image* GetFLTKRGBImage() const {
               return new Fl_RGB_Image(pixel_data, width, height, bytes_per_pixel);
	  }

     protected:
	  friend class ImageNavButton;
	  static Fl_RGB_Image *INFO_BUTTON_IMAGE;
          static Fl_RGB_Image *INFO_BUTTON_IMAGE2;
          static Fl_RGB_Image *EXPORT2XML_IMAGE;
	  static Fl_RGB_Image *CTFILE_IMAGE;
          static Fl_RGB_Image *NOPCTFILE_IMAGE;
          static Fl_RGB_Image *DOTFILE_IMAGE;
          static Fl_RGB_Image *BOLTZFILE_IMAGE;
          static Fl_RGB_Image *HLX3FILE_IMAGE;
	  static Fl_RGB_Image *FASTA_FILE_IMAGE;

	  static inline bool FLRGB_IMAGES_LOADED = false;
	  static inline bool LoadStaticFLRGBImages() {
	       if(ImagePixelData::FLRGB_IMAGES_LOADED) {
	            return false;
	       }
	       ImagePixelData::INFO_BUTTON_IMAGE = ImagePixelData::ConvertFromStructureType(SmallInfoButtonIcon).GetFLTKRGBImage();
	       ImagePixelData::FLRGB_IMAGES_LOADED = true;
	       return true;
	  }
	  
	  static inline bool IMAGES_ADJUSTED_FOR_THEME = false;
	  static inline bool AdjustImagesForTheme() {
               if(ImagePixelData::IMAGES_ADJUSTED_FOR_THEME) {
                    return false;
	       }
               Fl_RGB_Image **imagesList[] = {
                    &(ImagePixelData::INFO_BUTTON_IMAGE),
		    &(ImagePixelData::INFO_BUTTON_IMAGE2),
		    &(ImagePixelData::EXPORT2XML_IMAGE),
		    &(ImagePixelData::CTFILE_IMAGE),
		    &(ImagePixelData::NOPCTFILE_IMAGE),
		    &(ImagePixelData::DOTFILE_IMAGE),
		    &(ImagePixelData::BOLTZFILE_IMAGE),
		    &(ImagePixelData::HLX3FILE_IMAGE),
	       };
	       bool modifyImageColors[] = {
		    true, 
		    true,
		    true,
		    true,
		    true,
		    true,
		    true,
		    true,
	       };
	       unsigned int numImages = sizeof(imagesList) / sizeof(imagesList[0]);
	       for(int idx = 0; idx < numImages; idx++) {
                    if(!modifyImageColors[idx] || *(imagesList[idx]) == NULL) { continue; }
	            (*imagesList[idx])->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
	       }
	       ImagePixelData::IMAGES_ADJUSTED_FOR_THEME = true;
	       return true;
	  }

};

typedef void CallbackFunction(Fl_Widget*, void *);

class ImageNavButton : public Fl_Button {

     public:
          inline ImageNavButton(Fl_RGB_Image* &pixelDataSpec, const char *tooltipText, 
			        CallbackFunction callbackFunct, 
			        int x, int y, const char *labelStr = "") : 
		 Fl_Button(x, y, pixelDataSpec->h(), pixelDataSpec->w(), labelStr) {
               ImagePixelData::LoadStaticFLRGBImages();
	       ImageNavButton::AdjustImagesForTheme();
               color(GUI_WINDOW_BGCOLOR);
	       labelcolor(GUI_BTEXT_COLOR);
	       labelsize(2 * LOCAL_TEXT_SIZE);
	       image(pixelDataSpec);
	       deimage(pixelDataSpec);
	       align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
               labeltype(_FL_ICON_LABEL);
               box(FL_NO_BOX);
	       copy_label(labelStr);
	       copy_tooltip(tooltipText);
	       callback(callbackFunct);
	       if(visible()) {
	            redraw();
	       }
	  }

	  inline ~ImageNavButton() {
               hide();
	       while(visible()) {
	            Fl::wait(1.0);
	       }
	  }

          static inline bool AdjustImagesForTheme() {
               return ImagePixelData::AdjustImagesForTheme();
	  }

	  inline bool SetUserData(void *udata) {
               user_data((void *) udata);
	  }

	  inline bool SetActiveEnabled(bool isEnabled, bool hideWhenDisabled = false) {
	       value(isEnabled);
	       if(hideWhenDisabled && !isEnabled) {
	            hide();
	       }
	       else {
		    show();
	       }
	  }

	  inline explicit operator Fl_Widget*() {
	       return (Fl_Button *) this;
	  }

     protected:
	  static void ShowFolderInfoButtonCallback(Fl_Widget *btn, void *udata);
	  static void ShowStructureInfoButtonCallback(Fl_Widget *btn, void *udata);
	  static void ExportStructureToXMLCallback(Fl_Widget *btn, void *udata);
          
	  static inline void NoActionCallback(Fl_Widget *btn, void *udata) {}

     public:
          static inline const ImageNavButton* GetFileTypeButtonFactory(int x, int y, InputFileTypeSpec fileType) {
               switch(fileType) {
	            case FILETYPE_CT:
		         return new ImageNavButton(ImagePixelData::CTFILE_IMAGE, "File Type: CT", NoActionCallback, x, y);
		    case FILETYPE_NOPCT:
		          return new ImageNavButton(ImagePixelData::NOPCTFILE_IMAGE, "File Type: NOPCT", NoActionCallback, x, y);
		    case FILETYPE_DOTBRACKET:
                          return new ImageNavButton(ImagePixelData::DOTFILE_IMAGE, "File Type: DOTBracket", NoActionCallback, x, y);
		    case FILETYPE_GTB:
	                  return new ImageNavButton(ImagePixelData::BOLTZFILE_IMAGE, "File Type: Boltzmann", NoActionCallback, x, y);
		    case FILETYPE_HLXTRIPLE:
		          return new ImageNavButton(ImagePixelData::HLX3FILE_IMAGE, "File Type: Helix Triple", NoActionCallback, x, y);
		    case FILETYPE_RSVLOCALXML:
			  return NULL; // NOT YET IMPLEMENTED!! 
		    default:
			 break;
	       }
	       return NULL;
	  }

	  typedef enum {
	       NAVBTN_TYPE_FOLDERINFO     = 1, 
	       NAVBTN_TYPE_STRUCTINFO     = 2,
	       NAVBTN_TYPE_EXPORT2XML     = 3,
	  } ImageNavButtonPresetType_t;

	  static const ImageNavButton* GetNavButtonByTypeFactory(ImageNavButtonPresetType_t btnType, int x = 0, int y = 0) {
	       switch(btnType) {
	            case NAVBTN_TYPE_FOLDERINFO:
		         return new ImageNavButton(ImagePixelData::INFO_BUTTON_IMAGE, "Click for folder information", 
				                   ImageNavButton::ShowFolderInfoButtonCallback, x, y);
		    case NAVBTN_TYPE_STRUCTINFO:
			 return new ImageNavButton(ImagePixelData::INFO_BUTTON_IMAGE2, "Click for complete structure information", 
				                   ImageNavButton::ShowStructureInfoButtonCallback, x, y);
		    case NAVBTN_TYPE_EXPORT2XML:
			 return new ImageNavButton(ImagePixelData::EXPORT2XML_IMAGE, "Export structure data to XML file", 
				                   ImageNavButton::ExportStructureToXMLCallback, x, y);
		    default:
			 break;
	       }
	       return NULL;
	  }

};

#endif
