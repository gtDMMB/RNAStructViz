#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

#include "InputWindow.h"
#include "MainWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "RNAStructViz.h"
#include "StructureManager.h"
#include "FolderStructure.h"

int InputWindow::distinctStructureCount = 0;

InputWindow::InputWindow(int w, int h, const char *label, 
	const char *defaultName, InputWindowType type, int folderIndex) : 
	Fl_Window(MAX(w, 445), h, label), cbUseDefaultNames(NULL), ctFileChooser(NULL), 
	userWindowStatus(OK), fileSelectionIndex(-1)
{	
    string = (char*)malloc(sizeof(char)*90);
    color(GUI_WINDOW_BGCOLOR);
    set_modal();
    windowType = type;
    inputText = (char *) malloc(MAX_BUFFER_SIZE * sizeof(char));
    
    if(type == InputWindow::FILE_INPUT) {
    	    strncpy(inputText, defaultName, MAX_BUFFER_SIZE - 1);
	    inputText[MAX_BUFFER_SIZE - 1] = '\0';
	    char *extPtr = strrchr(inputText, '.');
	    if(extPtr != NULL) {
	         *extPtr = '\0';
	    }
	    char *filenameStartPtr = strrchr(inputText, '/');
	    int fnameStartPos;
	    if(filenameStartPtr != NULL) {
	         fnameStartPos = filenameStartPtr - inputText;
	    }
	    else {
		 fnameStartPos = 0;
	    }
	    char saveDirInfo[MAX_BUFFER_SIZE];
	    snprintf(saveDirInfo, fnameStartPos + 1, "%s", inputText);
	    sprintf(string, "Export to Directory: %s", saveDirInfo);
	    input = new Fl_Input(25, 50, 235, 30);
	    input->when(FL_WHEN_ENTER_KEY);
	    input->value(filenameStartPtr + 1);
	    Fl_Box *box = new Fl_Box(110, 20, 100, 30, (const char*)string);
	    box->box(FL_NO_BOX);
	    box->align(FL_ALIGN_CENTER);
	    Fl_Box *fileExtBox = new Fl_Box(260,50,30,30,".csv");
	    Fl_Button *button = new Fl_Button(305, 50, 110, 30, "Export to CSV @->");
	    button->callback(InputCallback, (void*)0);
	    button->set_active();
	    input->callback(InputCallback, (void*)0);
	    callback(CloseCallback);
    }
    else if(type == InputWindow::FOLDER_INPUT) {    
	    std::string actualStructName = 
		        ExtractStructureNameFromCTName(defaultName);
            const char *actualStructNameCStr = actualStructName.c_str();
            strncpy(inputText, actualStructNameCStr, actualStructName.size() + 1);
            ConfigParser::nullTerminateString(inputText, actualStructName.size());

	    sprintf(string, "Creating new folder for the CT structure %s", defaultName);
	    input = new Fl_Input(160, 50, 250, 30, "@fileopen  New Folder Name:");
	    input->when(FL_WHEN_ENTER_KEY);
            input->maximum_size(60);
	    input->value(inputText);
	    input->color(GUI_BGCOLOR);
	    input->textcolor(GUI_BTEXT_COLOR);
	    Fl_Box *box = new Fl_Box(75, 1, 300, 40, (const char*) string);
	    box->box(FL_OSHADOW_BOX);
	    box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	    box->color(GUI_BGCOLOR);
	    box->labelcolor(GUI_BTEXT_COLOR);
	    Fl_Button *button = new Fl_Button(340, 50, 100, 30, "Add Folder @|>");
	    button->callback(InputCallback, (void*)0);
	    button->labelcolor(GUI_BTEXT_COLOR);
	    button->set_active();
	    input->callback(InputCallback, (void*)0);
	    input->labelcolor(GUI_TEXT_COLOR);
	    const char *cbText = " Use only default names for structure folders";
	    cbUseDefaultNames = new Fl_Check_Button(55, 100, 325, 30, cbText);
	    cbUseDefaultNames->box(FL_ROUND_UP_BOX);
	    cbUseDefaultNames->color(GUI_BGCOLOR);
	    cbUseDefaultNames->labelcolor(GUI_BTEXT_COLOR);
	    cbUseDefaultNames->down_color(GUI_WINDOW_BGCOLOR);
	    cbUseDefaultNames->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	    callback(CloseCallback);
	}
        else { 
	    
	    const char *windowDisplayMsg = "Which CT file structure for the organism\ndo you want to display?";
	    Fl_Box *box = new Fl_Box(75, 5, 300, 40, windowDisplayMsg);
	    box->box(FL_RSHADOW_BOX);
	    box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	    box->color(GUI_BGCOLOR);
	    box->labelcolor(GUI_BTEXT_COLOR);
	    Fl_Button *displayButton = new Fl_Button(100, 100, 155, 30, "Display Zoomed Region @|>");
	    displayButton->callback(DisplayCTFileCallback, (void*)0);
	    displayButton->color(GUI_BGCOLOR);
	    displayButton->labelcolor(GUI_BTEXT_COLOR);
	    displayButton->set_active();
	    Fl_Button *cancelButton = new Fl_Button(275, 100, 75, 30, "Cancel @1+");
            cancelButton->callback(CancelCallback); 
	    cancelButton->color(GUI_BGCOLOR);
	    cancelButton->labelcolor(GUI_BTEXT_COLOR);
            
	    ctFileChooser = new Fl_Choice(190, 55, 200, 30, "Choose CT Structure: ");
            ctFileChooser->color(GUI_BGCOLOR);
	    ctFileChooser->labelcolor(GUI_BTEXT_COLOR);

	    StructureManager *structManager = RNAStructViz::GetInstance()->GetStructureManager();
	    Folder *curFolder = structManager->GetFolderAt(folderIndex);
	    for(int s = 0; s < curFolder->structCount; s++) { 
                 RNAStructure *rnaStruct = structManager->GetStructure(curFolder->folderStructs[s]);
		 const char *ctFileName = rnaStruct->GetFilename();
		 ctFileChooser->add(ctFileName);
	    }
	    ctFileChooser->value(0);
	
	}
        show();
        if(type != InputWindow::FOLDER_INPUT || !GUI_USE_DEFAULT_FOLDER_NAMES) { 
            show();
	}
	else {
	    show();
	    InputCallback((Fl_Widget *) cbUseDefaultNames, (void *) NULL);
	}
}

InputWindow::~InputWindow() {
    delete input;
    if(cbUseDefaultNames != NULL) {
        delete cbUseDefaultNames;
    }
    free(inputText);
}

int InputWindow::getFileSelectionIndex() const {
     return fileSelectionIndex;
}

bool InputWindow::isCanceled() const {
     return userWindowStatus == CANCELED;
}

void InputWindow::InputCallback(Fl_Widget *widget, void *userdata) {
    InputWindow *window = (InputWindow*) widget->parent();
    if(window->windowType == InputWindow::FILE_INPUT) {
         char exportSaveDir[MAX_BUFFER_SIZE];
	 char *fileSepPtr = strrchr(window->inputText, '/');
	 int dirTextLen = fileSepPtr - window->inputText + 1;
	 snprintf(exportSaveDir, dirTextLen, "%s", window->inputText);
	 snprintf(window->inputText, MAX_BUFFER_SIZE - 1, "%s/%s.csv", 
	          exportSaveDir, window->input->value());
         window->name = window->inputText;
    }
    else {
        if(window->inputText != (char*)window->input->value()) {
            strcpy(window->inputText, (char*)window->input->value());
        }
        if(!MainWindow::CheckDistinctFolderName(window->inputText)) {
	    fl_alert("The folder name for the structure \"%s\" already exists!", window->inputText);
	    return;
	}
	window->name = window->inputText;
        if(window->cbUseDefaultNames->value()) {
            GUI_USE_DEFAULT_FOLDER_NAMES = true;
        }
    }    
    free(window->string);
    window->hide();
}

void InputWindow::CloseCallback(Fl_Widget* widget, void* userData) {
    InputWindow *window = (InputWindow*)widget;
    window->name = "";
    free(window->string);
    window->hide();
}

void InputWindow::DisplayCTFileCallback(Fl_Widget *w, void *udata) {
     InputWindow *iwin = (InputWindow *) w->parent();
     if(iwin->ctFileChooser == NULL) {
          iwin->userWindowStatus = CANCELED;
	  iwin->fileSelectionIndex = -1;
     }
     else {
	  iwin->userWindowStatus = OK;
	  iwin->fileSelectionIndex = iwin->ctFileChooser->value();
     }
     iwin->hide();
}

void InputWindow::CancelCallback(Fl_Widget *w, void *udata) {
     InputWindow *iwin = (InputWindow *) w->parent();
     iwin->userWindowStatus = CANCELED;
     iwin->fileSelectionIndex = -1;
     iwin->hide();
}

std::string InputWindow::ExtractStructureNameFromCTName(const char *ctPath) {
    RNAStructure *rnaStruct = RNAStructViz::GetInstance()->GetStructureManager()->
	                      LookupStructureByCTPath(ctPath);
    const char *suggestedFolderName = rnaStruct ? rnaStruct->GetSuggestedStructureFolderName() : NULL;
    const char *folderNumberDivider = suggestedFolderName ? " -- " : "";
    char suggestedShortName[MAX_BUFFER_SIZE];
    snprintf(suggestedShortName, MAX_BUFFER_SIZE, "No. #% 2d%s%s\0", ++InputWindow::distinctStructureCount, 
	     folderNumberDivider, suggestedFolderName ? suggestedFolderName : "");
    return std::string(suggestedShortName);
}

