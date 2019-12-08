#ifndef FOLDERSTRUCTURE_H
#define FOLDERSTRUCTURE_H

#include <string.h>

#include <FL/Fl_Button.H>

#include "MainWindow.h"
#include "FolderWindow.h"
#include "RNAStructure.h"
#include "ConfigOptions.h"
#include "ConfigExterns.h"
#include "BaseSequenceIDs.h"

#define DEFAULT_FOLDER_NAME_BUFSIZE        (64)
#define DEFAULT_FOLDER_NAME_LBLSIZE        (72)
#define FOLDER_LABEL_TRUNC_LENGTH          (25)

#define FOLDER_WIDGET_WIDTH                (30)
#define FOLDER_WIDGET_HEIGHT               (30)
#define FOLDER_ACTION_BUTTON_SIZE          (20)

class Folder {

  public:
    inline Folder() : folderName(NULL), folderNameFileCount(NULL), folderStructs(NULL), 
	              structCount(NULL), selected(false), fileType(FILETYPE_NONE), 
		      capacity(2 * DEFAULT_FOLDER_NAME_BUFSIZE), folderWindow(NULL), 
		      mainWindowFolderBtn(NULL), navUpBtn(NULL), navDownBtn(NULL), 
		      navCloseBtn(NULL), 
		      doWidgetDeletion(true) {
        folderStructs = (int *) malloc(capacity * sizeof(int));
        for(int fsi = 0; fsi < capacity; fsi++) {
	     folderStructs[fsi] = -1;
	}
	tooltipText[0] = '\0';
    }
    
    inline ~Folder() {
        Free(folderName);
        /*if(mainWindowFolderBtn != NULL) {
	    mainWindowFolderBtn->label("");
	    mainWindowFolderBtn->tooltip("");
	}*/
	Free(folderNameFileCount);
        Free(folderStructs); 
        if(doWidgetDeletion) {
	     DeleteGUIWidgetData();
	     if(folderWindow != NULL) {
	          folderWindow->HideFolderWindowGUIDisplay(true);
	     }
	     Delete(folderWindow, FolderWindow);
	}
	structCount = 0;
        selected = false;
    }

    inline void MarkForDeletion() {
	if(mainWindowFolderBtn != NULL) {
	    mainWindowFolderBtn->label("");
	    mainWindowFolderBtn->tooltip("");
	}
    }

    static inline void WaitForHiddenWidget(Fl_Widget *widgetToHide) {
         if(widgetToHide != NULL) {
	      widgetToHide->hide();
	      while(widgetToHide->visible()) {
	           Fl::wait(1.0);
	      }
	 }
    }

    inline void HideGUIWidgets(bool waitForNoVisible = true) {
	 //Folder::WaitForHiddenWidget(guiPackingContainerRef);
	 Folder::WaitForHiddenWidget(guiPackingGroup);
         Folder::WaitForHiddenWidget(mainWindowFolderBtn);
	 Folder::WaitForHiddenWidget(navUpBtn);
	 Folder::WaitForHiddenWidget(navDownBtn);
	 Folder::WaitForHiddenWidget(navCloseBtn);
    }

    inline void DeleteGUIWidgetData() {
	 /*if(mainWindowFolderBtn != NULL) {
              mainWindowFolderBtn->label("");
              mainWindowFolderBtn->tooltip("");
	 }*/
         HideGUIWidgets(true);
	 /*Delete(mainWindowFolderBtn, Fl_Button);
	 Delete(navUpBtn, Fl_Button);
	 Delete(navDownBtn, Fl_Button);
         Delete(navCloseBtn, Fl_Button);*/
	 //if(guiPackingContainerRef != NULL) {
	 //     guiPackingContainerRef->remove(guiPackingGroup);
	 //}
	 Delete(guiPackingGroup, Fl_Group);
    }

    static Folder * AddNewFolderFromData(const RNAStructure *structure, const int index, bool isSelected = false) {
         Folder *nextFolder = new Folder();
	 nextFolder->folderName = (char *) malloc(DEFAULT_FOLDER_NAME_BUFSIZE * sizeof(char));
	 nextFolder->fileType = ClassifyInputFileType(structure->GetFilename());
         if(strlen(structure->GetFilename()) < DEFAULT_FOLDER_NAME_BUFSIZE - 4) {
              strcpy(nextFolder->folderName, structure->GetFilename());
         }
         else {
              strncpy(nextFolder->folderName, structure->GetFilename(), DEFAULT_FOLDER_NAME_BUFSIZE - 4);
              nextFolder->folderName[DEFAULT_FOLDER_NAME_BUFSIZE - 4] = '\0';
         }
         nextFolder->folderNameFileCount = (char *) malloc(DEFAULT_FOLDER_NAME_LBLSIZE * sizeof(char));
	 nextFolder->folderStructs[0] = index;
         nextFolder->structCount = 1;
	 nextFolder->CreateFolderGUIElements();
	 nextFolder->SetFolderLabel();
	 nextFolder->SetTooltipTextData();
         nextFolder->SetSelected(isSelected);
	 nextFolder->folderWindow = NULL;
	 return nextFolder;

    }

    inline void AddStructToFolderByIndex(int index) {
         if(structCount >= capacity) {
	      folderStructs = (int *) realloc(folderStructs, 2 * capacity);
	      capacity *= 2;
	 }
	 folderStructs[structCount++] = index;
    }

    inline void SetFolderLabel() {
         sprintf(folderNameFileCount, Folder::folderLabelFmt,
                 structCount, folderName,
                 strlen(folderName) > FOLDER_LABEL_TRUNC_LENGTH ? "..." : "");
	 if(mainWindowFolderBtn != NULL) {
	      mainWindowFolderBtn->copy_label(folderNameFileCount);
	 }
    }

    inline void SetTooltipTextData() {
         snprintf(tooltipText, MAX_BUFFER_SIZE - 1, Folder::tooltipTextFmt, folderName, structCount);
	 if(mainWindowFolderBtn != NULL) {
	      mainWindowFolderBtn->copy_tooltip(tooltipText);
	 }
    }

    inline void SetSelected(bool isSelected) {
         selected = isSelected;
	 if(selected) {
              mainWindowFolderBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
	 }
	 else {
              mainWindowFolderBtn->color(GUI_BGCOLOR);
	 }
    }

    inline void CreateFolderGUIElements(Fl_Pack *pack = NULL, int x = 0, int y = 0, 
		                        bool doPreviousWidgetDeletion = true) {
         if(pack == NULL) {
	      pack = MainWindow::ms_instance->m_packedInfo;
	 }
	 else if(doPreviousWidgetDeletion) {
	      DeleteGUIWidgetData();
	 }
	 guiPackingContainerRef = pack;
	 pack->hide();
	 pack->begin();
	 guiPackingGroup = new Fl_Group(pack->x() + x, pack->y() + y, pack->w(), FOLDER_WIDGET_HEIGHT);
	 mainWindowFolderBtn = new Fl_Button(pack->x() + x + 8, pack->y() + y, pack->w() - 70, FOLDER_WIDGET_HEIGHT, "");
	 mainWindowFolderBtn->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	 mainWindowFolderBtn->callback(MainWindow::ShowFolderCallback);
	 mainWindowFolderBtn->user_data((void *) folderName);
	 mainWindowFolderBtn->labelcolor(GUI_BTEXT_COLOR);
	 mainWindowFolderBtn->labelsize(10);
	 mainWindowFolderBtn->labelfont(FL_HELVETICA_BOLD_ITALIC);
	 mainWindowFolderBtn->box(FL_UP_BOX);
         navUpBtn = new Fl_Button(pack->x() + x + pack->w() - 58, pack->y() + y + 5, 
			          FOLDER_ACTION_BUTTON_SIZE, FOLDER_ACTION_BUTTON_SIZE, Folder::navUpBtnLabel);
	 navUpBtn->tooltip("Move folder up in list");
	 navUpBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.50));
	 navUpBtn->box(FL_PLASTIC_UP_BOX);
	 navUpBtn->callback(MainWindow::MoveFolderUp);
         navDownBtn = new Fl_Button(pack->x() + x + pack->w() - 38, pack->y() + y + 5, 
			            FOLDER_ACTION_BUTTON_SIZE, FOLDER_ACTION_BUTTON_SIZE, Folder::navDownBtnLabel);
	 navDownBtn->tooltip("Move folder down in list");
	 navDownBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.50));
	 navDownBtn->box(FL_PLASTIC_UP_BOX);
	 navDownBtn->callback(MainWindow::MoveFolderDown);
         navCloseBtn = new Fl_Button(pack->x() + x + pack->w() - 18, pack->y() + y + 5, 
			             FOLDER_ACTION_BUTTON_SIZE, FOLDER_ACTION_BUTTON_SIZE, Folder::navCloseBtnLabel);
	 navCloseBtn->tooltip("Remove folder and all its stored structures");
	 navCloseBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.50));
	 navCloseBtn->box(FL_PLASTIC_UP_BOX);
	 navCloseBtn->callback(MainWindow::RemoveFolderCallback);
         guiPackingGroup->resizable(mainWindowFolderBtn);
	 guiPackingGroup->redraw();
	 pack->end();
	 pack->show();
	 pack->redraw();
    }

    /* Folder data accounting fields: */
    char *folderName;
    char *folderNameFileCount;
    int  *folderStructs;
    int  capacity;
    int  structCount;
    bool selected;
    InputFileTypeSpec fileType;

    /* Reference to main parent structure: */ 
    FolderWindow *folderWindow;
    
    /* Keep track of the GUI widgets associated with the display for this folder: */
    Fl_Pack   *guiPackingContainerRef;
    Fl_Group  *guiPackingGroup;
    Fl_Button *mainWindowFolderBtn;
    Fl_Button *navUpBtn, *navDownBtn, *navCloseBtn;

  protected:
    bool doWidgetDeletion;

  public:
    void SetPerformWidgetDeletion(bool enable) {
         doWidgetDeletion = enable;
    }

    /* Tooltip formatting of the main button: */
    char tooltipText[MAX_BUFFER_SIZE];
    inline static const char *tooltipTextFmt = "%s with %d structures available";

    /* Constant strings to be reused as labels (formatting for labels): */
    inline static const char *folderLabelFmt = "(+% 2d) %-.25s%s";
    inline static const char *navUpBtnLabel = "@8>";
    inline static const char *navDownBtnLabel = "@2>";
    inline static const char *navCloseBtnLabel = "@1+";

    bool operator==(Fl_Widget *widgetLabelToCmp) {
        if(widgetLabelToCmp == NULL) {
	     return false;
	}
	else if(widgetLabelToCmp->user_data() != NULL && !strcmp((char *) widgetLabelToCmp->user_data(), folderName)) {
	     return true;
	}
	return false;
    }

    explicit operator Fl_Widget*() const {
         return guiPackingContainerRef;
    }

    explicit operator Fl_Pack*() const {
	 return guiPackingContainerRef;
    }

    explicit operator Fl_Group*() const {
	 return guiPackingGroup;
    }

    explicit operator Fl_Button*() const {
	 return mainWindowFolderBtn;
    }

    typedef enum {
         PACKING_CONTAINER = 0, 
	 PACKING_GROUP     = 1,
	 MAINWIN_BUTTON    = 2,
	 NAVUP_BUTTON      = 3, 
	 NAVDOWN_BUTTON    = 4,
	 NAVCLOSE_BUTTON   = 5,
    } WidgetAccessorIndexType_t;

    Fl_Widget* GetGUIWidgetByIndexType(WidgetAccessorIndexType_t widgetIndex) {
         switch(widgetIndex) {
	      case PACKING_CONTAINER:
	           return guiPackingContainerRef;
	      case PACKING_GROUP:
		   return guiPackingGroup;
	      case MAINWIN_BUTTON:
		   return mainWindowFolderBtn;
	      case NAVUP_BUTTON:
		   return navUpBtn;
	      case NAVDOWN_BUTTON:
		   return navDownBtn;
	      case NAVCLOSE_BUTTON:
		   return navCloseBtn;
	      default:
		   break;
	 }
	 return NULL;
    }

    inline bool IsEnabled() const {
         return (mainWindowFolderBtn->value() == 0 && mainWindowFolderBtn->type() == FL_NORMAL_BUTTON) || 
		mainWindowFolderBtn->value();
    }

    inline unsigned int GetSize() const {
	 return structCount;
    }

    inline bool IsEmpty() const {
	 return structCount == 0;
    }
    
    inline bool IsFASTAFormatOnly() const {
	 return fileType == FILETYPE_FASTA_ONLY;
    }

    inline Fl_Widget* GetGUIParent() const {
	 return guiPackingContainerRef;
    }

    inline bool FolderLabelIsSticky() const {
	 return false; // TODO
    }

    inline bool RemoveFolderLabelAsSticky() {
	 return false; // TODO
    }

    inline bool AddFolderLabelAsSticky() {
	 return false;
    }

    static inline void ToggleFolderLabelStickynessCallback(Fl_Widget *btn, void *udata) {
         Folder *folder = (Folder *) btn->user_data();
	 if(folder == NULL) {
              return;
	 }
	 else if(folder->FolderLabelIsSticky()) {
              folder->RemoveFolderLabelAsSticky();
	 }
	 else {
              folder->AddFolderLabelAsSticky();
	 }
    }

    static inline void ExportDataFolderToXMLCallback(Fl_Widget *btn, void *udata) {
         // TODO
    }

    static inline void DisplayFolderInformationCallback(Fl_Widget *btn, void *udata) {
         // TODO
    }

};

#endif
