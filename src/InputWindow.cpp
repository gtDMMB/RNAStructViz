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
#include "BaseSequenceIDs.h"

int InputWindow::distinctStructureCount = 0;

InputWindow::InputWindow(int w, int h, const char *label, 
    const char *defaultName, InputWindowType type, int folderIndex) : 
    Fl_Window(w, h, label), cbUseDefaultNames(NULL), ctFileChooser(NULL), 
    userWindowStatus(OK), fileSelectionIndex(-1), 
    stickyFolderNameFound(false), suggestedFolderNameFound(false), 
    stickyFolderNamesNeedsReset(false)
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
        input = new Fl_Input(15, 50, 245, 30);
        //input->when(FL_WHEN_ENTER_KEY);
        input->value(filenameStartPtr + 1);
        Fl_Box *box = new Fl_Box(100, 20, 110, 30, (const char*)string);
        box->box(FL_NO_BOX);
        box->align(FL_ALIGN_CENTER);
        Fl_Box *fileExtBox = new Fl_Box(255,50,40,30,".csv");
        Fl_Button *button = new Fl_Button(305, 50, 125, 30, "Export to CSV @->");
        button->callback(InputCallback, (void*)0);
        button->set_active();
        input->callback(InputCallback, (void*)0);
        callback(CloseCallback);
    }
    else if(type == InputWindow::FOLDER_INPUT) {    
        std::string actualStructName = 
                ExtractStructureNameFromFile(defaultName);
            const char *actualStructNameCStr = actualStructName.c_str();
            strcpy(inputText, actualStructNameCStr);
            ConfigParser::nullTerminateString(inputText, actualStructName.size());

        sprintf(string, "Creating new folder for the sample structure %s", defaultName);
        input = new Fl_Input(160, 50, 360, 30, "@fileopen  New Folder Name:");
        //input->when(FL_WHEN_ENTER_KEY);
        input->maximum_size(60);
        input->value(inputText);
        input->color(GUI_BGCOLOR);
        input->textcolor(GUI_BTEXT_COLOR);
        Fl_Box *box = new Fl_Box(100, 1, 400, 40, (const char*) string);
        box->box(FL_OSHADOW_BOX);
        box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
        box->color(GUI_BGCOLOR);
        box->labelcolor(GUI_BTEXT_COLOR);
        Fl_Button *button = new Fl_Button(410, 165, 100, 30, "Add Folder @|>");
        button->callback(InputCallback, (void*)0);
        button->labelcolor(GUI_BTEXT_COLOR);
        button->set_active();
        input->callback(InputCallback, (void*)0);
        input->labelcolor(GUI_TEXT_COLOR);
        const char *cbText = " Use only default names for structure folders";
        cbUseDefaultNames = new Fl_Check_Button(16, 100, 375, 30, cbText);
        cbUseDefaultNames->box(FL_ROUND_UP_BOX);
        cbUseDefaultNames->color(GUI_BGCOLOR);
        cbUseDefaultNames->labelcolor(GUI_BTEXT_COLOR);
        cbUseDefaultNames->down_color(GUI_WINDOW_BGCOLOR);
        cbUseDefaultNames->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
        cbUseDefaultNames->value(GUI_USE_DEFAULT_FOLDER_NAMES);
        cbUseDefaultNames->selection_color(GUI_TEXT_COLOR);
        const char *stickyFoldersCBText = "Save folder names for known organisms";
        cbKeepStickyFolders = new Fl_Check_Button(16, 145, 375, 30, stickyFoldersCBText);
            cbKeepStickyFolders->box(FL_ROUND_UP_BOX);
        cbKeepStickyFolders->color(GUI_BGCOLOR);
        cbKeepStickyFolders->labelcolor(GUI_BTEXT_COLOR);
        cbKeepStickyFolders->down_color(GUI_WINDOW_BGCOLOR);
        cbKeepStickyFolders->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
        cbKeepStickyFolders->value(GUI_KEEP_STICKY_FOLDER_NAMES && (stickyFolderNameFound || suggestedFolderNameFound));
        cbKeepStickyFolders->selection_color(GUI_TEXT_COLOR);
        InitCleanupParams();
        callback(CloseCallback);
    }
        else { 
        
        const char *windowDisplayMsg = "Which structure file\ndo you want to display?";
        Fl_Box *box = new Fl_Box(70, 5, 260, 40, windowDisplayMsg);
        box->box(FL_RSHADOW_BOX);
        box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
        box->color(GUI_BGCOLOR);
        box->labelcolor(GUI_BTEXT_COLOR);
        Fl_Button *displayButton = new Fl_Button(50, 100, 200, 30, "Display Zoomed Region @|>");
        displayButton->callback(DisplayCTFileCallback, (void*)0);
        displayButton->color(GUI_BGCOLOR);
        displayButton->labelcolor(GUI_BTEXT_COLOR);
        displayButton->set_active();
        displayButton->shortcut(FL_Enter);
        Fl_Button *cancelButton = new Fl_Button(260, 100, 75, 30, "Cancel @1+");
            cancelButton->callback(CancelCallback); 
        cancelButton->color(GUI_BGCOLOR);
        cancelButton->labelcolor(GUI_BTEXT_COLOR);
            
        ctFileChooser = new Fl_Choice(150, 55, 240, 30, "Choose Structure: ");
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
        if(type != InputWindow::FOLDER_INPUT || !GUI_USE_DEFAULT_FOLDER_NAMES && 
       (!GUI_KEEP_STICKY_FOLDER_NAMES || !stickyFolderNameFound)) {
            show();
    }
    else {
        show();
        InputCallback((Fl_Widget *) cbUseDefaultNames, (void *) NULL);
    }
}

InputWindow::~InputWindow() {
    Cleanup(false);
    Delete(input);
    Delete(cbUseDefaultNames);
    Delete(cbKeepStickyFolders);
    Free(inputText);
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
     snprintf(window->inputText, MAX_BUFFER_SIZE - 1, "%s%s%s.csv", 
              (char *) PNG_OUTPUT_DIRECTORY, 
	      PNG_OUTPUT_DIRECTORY[strlen((char *) PNG_OUTPUT_DIRECTORY) - 1] == '/' || 
	      window->inputText[0] == '/' ? "" : "/", 
	      window->input->value());
     window->name = "";
    }
    else {
    window->name = "";
        GUI_USE_DEFAULT_FOLDER_NAMES = window->cbUseDefaultNames->value();
        GUI_KEEP_STICKY_FOLDER_NAMES = window->cbKeepStickyFolders->value();
    }    
    Free(window->string);
    window->hide();
}

void InputWindow::CloseCallback(Fl_Widget* widget, void* userData) {
    InputWindow *window = (InputWindow*)widget;
    if(window != NULL) {
         window->name = "";
         Free(window->string);
         window->hide();
    }
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
     if(iwin != NULL) {
          iwin->userWindowStatus = CANCELED;
          iwin->fileSelectionIndex = -1;
          iwin->hide();
     }
}

std::string InputWindow::ExtractStructureNameFromFile(const char *seqFilePath) {
    
    RNAStructure *rnaStruct = RNAStructViz::GetInstance()->GetStructureManager()->
                          LookupStructureByCTPath(seqFilePath);
    
    // see if there is a sticky folder name already saved to display:
    off_t stickyFolderExists = FolderNameForSequenceExists(
                            DEFAULT_STICKY_FOLDERNAME_CFGFILE,
                    rnaStruct
                   );
    if(stickyFolderExists != LSEEK_NOT_FOUND) {
         char *stickyFolderName = LookupStickyFolderNameForSequence(
                           DEFAULT_STICKY_FOLDERNAME_CFGFILE,
                       stickyFolderExists
                  );
     if(stickyFolderName != NULL) {
          //// executive decision: save space on the small button labels by not 
          //// prepending a folder number if there is a saved sticky folder label 
          //// already there to go from the local user config files:
          std::string suggestedStickyName = std::string(stickyFolderName);
          Free(stickyFolderName);
          stickyFolderNameFound = true;
          return suggestedStickyName;
     }
    }
    
    // otherwise, use file name and file header comments to guess at a good name for the sequence:
    InputFileTypeSpec inputFileType = ClassifyInputFileType(seqFilePath);
    std::string fullSeqFilePath = std::string((char *) CTFILE_SEARCH_DIRECTORY) + std::string("/") + 
                              std::string(seqFilePath);
    std::string fileHeaderLines = GetSequenceFileHeaderLines(fullSeqFilePath.c_str(), inputFileType);
    if(fileHeaderLines.size() > 0) {
         rnaStruct->SetFileCommentLines(fileHeaderLines, inputFileType);
    }
    const char *suggestedFolderName = rnaStruct ? 
                                  rnaStruct->GetSuggestedStructureFolderName() : NULL;
    if(suggestedFolderName != NULL) {
         suggestedFolderNameFound = true;
         return std::string(suggestedFolderName);
    }
    char fileTypeIdStr[MAX_BUFFER_SIZE] = "";
    const char *extPos = strrchr(seqFilePath, '.');
    if(extPos != NULL) {
         fileTypeIdStr[0] = '(';
     strcpy(fileTypeIdStr + 1, extPos + 1);
     strcat(fileTypeIdStr, ")");
     StringTransform(fileTypeIdStr, toupper);
    }
    else {
     fileTypeIdStr[0] = '\0';
    }
    char suggestedShortName[MAX_BUFFER_SIZE];
    snprintf(suggestedShortName, MAX_BUFFER_SIZE, "No. #% 2d %s", 
             ++InputWindow::distinctStructureCount, fileTypeIdStr); 
    if(fileTypeIdStr[0] == '\0') {
         suggestedShortName[MAX_BUFFER_SIZE - 2] = '\0';
    }
    else {
         suggestedShortName[MAX_BUFFER_SIZE - 1] = '\0';
    }
    return std::string(suggestedShortName);

}

