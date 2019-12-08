/* StructureType.h : Analogous to the structures for storing folders found in FolderStructure.h. 
 *                   The classes and data defined here should make it easy to keep track of the 
 *                   individual structure widgets and display data on the RHS of the 
 *                   FolderWindow instances in the GUI;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.27
 */

#ifndef __STRUCTURE_TYPE_STORAGE_H__
#define __STRUCTURE_TYPE_STORAGE_H__

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Enumerations.H>

#include "ConfigOptions.h"
#include "ConfigExterns.h"
#include "RNAStructVizTypes.h"
#include "AutoloadIndicatorButton.h"
#include "XMLExportButton.h"

#define STRUCTURE_WIDGET_WIDTH               (30)
#define STRUCTURE_WIDGET_HEIGHT              (30)
#define STRUCTURE_NAVBTN_SIZE                (20)

#define TOOLTIP_DATA_SPACING_INTERVAL        (8)    /* Insert a space between consecutive pairing / base data chars for readability */
#define TOOLTIP_PREVIEW_CHARS_LENGTH         (56)
#define MAX_FOLDER_LABEL_CHARS               (22)

class StructureData {

     protected:
	  friend class FolderWindow;
	  friend class XMLExportButton;

	  std::string origFolderWinLabel;
	  std::string structFileBaseDir;
	  std::string structFileBaseName;
	  std::string mainButtonLabel;
	  std::string mainButtonTooltip;
          
	  RNAStructure *structure;
	  FolderWindow *parentMainFolderWin;
	  int index;

	  Fl_Pack   *guiPackingContainerRef;
	  Fl_Group  *guiPackingGroup;
	  Fl_Button *mainViewerDisplayBtn;
          Fl_Button *navCloseBtn;
	  AutoloadIndicatorButton *autoloadToggleBtn;
	  XMLExportButton *xmlExportBtn;

          static inline const char *navCloseBtnLabel = "@1+";
	  static inline std::string spaceBuffer      = std::string("                                                    ");

     public:
          inline StructureData() : structure(NULL), parentMainFolderWin(NULL), 
	                           index(-1), guiPackingContainerRef(NULL), guiPackingGroup(NULL), 
	                           mainViewerDisplayBtn(NULL), navCloseBtn(NULL), 
			           autoloadToggleBtn(NULL), xmlExportBtn(NULL) {} 
	  
	  inline ~StructureData() {
               DeleteGUIWidgets();
	       structure = NULL;
	       parentMainFolderWin = NULL;
	       index = -1;
	  }

	  inline void MarkForDeletion() {
	       if(mainViewerDisplayBtn != NULL) {
	            mainViewerDisplayBtn->label("");
		    mainViewerDisplayBtn->tooltip("");
	       }
	       if(navCloseBtn != NULL) {
		    navCloseBtn->label("");
		    navCloseBtn->tooltip("");
	       }
	       //Delete(guiPackingGroup, Fl_Group);
	       Fl::delete_widget(guiPackingGroup);
	       guiPackingGroup = NULL;
	  }

     protected:
	  inline void DeleteGUIWidgets() {
               /*if(mainViewerDisplayBtn != NULL && navCloseBtn != NULL && guiPackingGroup != NULL) {
	            mainViewerDisplayBtn->hide();
	            navCloseBtn->hide();
	            guiPackingGroup->hide();
	            while(mainViewerDisplayBtn->visible() || navCloseBtn->visible() || guiPackingGroup->visible()) {
	                 Fl::wait(1.0);
	            }
	       }*/
               /*if(guiPackingContainerRef != NULL && guiPackingGroup != NULL) {
	            guiPackingContainerRef->remove(guiPackingGroup);
	       }*/
	       /*if(mainViewerDisplayBtn != NULL) {
	            mainViewerDisplayBtn->label("");
		    mainViewerDisplayBtn->tooltip("");
	       }
	       if(navCloseBtn != NULL) {
		    navCloseBtn->label("");
		    navCloseBtn->tooltip("");
	       }*/
	       /*Delete(mainViewerDisplayBtn, Fl_Button);
	       Delete(navCloseBtn, Fl_Button);
	       Delete(autoloadToggleBtn, AutoloadIndicatorButton);
	       Delete(xmlExportBtn, XMLExportButton);*/
	       Delete(guiPackingGroup, Fl_Group);
	  }    

	  inline void SetLabelText() {
	       std::string buttonLabel = "";
	       const char *fullFileName = structFileBaseName.c_str();
	       size_t fileNameCopyChars = (strrchr(fullFileName, '.') ? strrchr(fullFileName, '.') : 
			                  fullFileName + strlen(fullFileName)) - fullFileName;
               std::string fileNameNoExt = structFileBaseName.substr(0, fileNameCopyChars);
               bool truncFileNameStr = fileNameNoExt.length() > MAX_FOLDER_LABEL_CHARS;
	       fileNameNoExt = fileNameNoExt.substr(0, MIN(fileNameNoExt.length(), MAX_FOLDER_LABEL_CHARS));
	       if(truncFileNameStr) {
	            fileNameNoExt += "...";
	       }
	       std::string insertSpacesStr = StructureData::spaceBuffer.substr(0, 
			                     MAX(0, MAX_FOLDER_LABEL_CHARS - ((int) strlen(fileNameNoExt.c_str()))));
	       buttonLabel = std::string("@filenew   ") + fileNameNoExt + insertSpacesStr;
	       mainButtonLabel = buttonLabel;
	       mainViewerDisplayBtn->label(mainButtonLabel.c_str());
	  }

	  void SetTooltipText(); 
	  void CreateGUIElementsDisplay(Fl_Pack *pack); 
     
     public:
	  static StructureData* AddStructureFromData(FolderWindow *fwinRef, const char *fileName, const int index); 

     private:
	  static void PinStructureToAutoloadCallback(Fl_Widget *btn, void *udata);

};

#endif
