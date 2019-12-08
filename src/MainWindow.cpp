#include "MainWindow.h"
#include "RNAStructViz.h"
#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "CommonDialogs.h"
#include "TerminalPrinting.h"
#include "ThemesConfig.h"
#include "RNAStructVizTypes.h"

#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
using std::string;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_PNG_Image.H>

#include "pixmaps/RNAStructVizLogo.c"
#include "pixmaps/RNAWindowIcon.xbm"
#include "pixmaps/HelpIcon.c"
#include "pixmaps/InfoButton.c"

MainWindow* MainWindow::ms_instance = NULL;

Fl_RGB_Image * MainWindow::helpIconImage = new Fl_RGB_Image( 
           HelpIcon.pixel_data, 
           HelpIcon.width, HelpIcon.height, HelpIcon.bytes_per_pixel);

Fl_RGB_Image * MainWindow::infoButtonImage = new Fl_RGB_Image( 
           InfoButton.pixel_data, 
           InfoButton.width, InfoButton.height, InfoButton.bytes_per_pixel);

MainWindow::MainWindow(int argc, char **argv)
          : m_fileChooser(NULL), m_fileChooserSelectAllBtn(NULL), 
        selectedFolderBtn(NULL), 
        selectedFolderIndex(-1)
{
    
    m_mainWindow = new Fl_Double_Window(650, 450, RNASTRUCTVIZ_VSTRING);
    m_mainWindow->size_range(650, 450, 650, 450);
    m_mainWindow->callback(CloseCallback);
    m_mainWindow->color(GUI_WINDOW_BGCOLOR);
    m_mainWindow->begin();    

    mainMenuPane = new Fl_Group(0, 0, 300, 450, "");
    {
        
        // setup the program logo (for now dynamically loaded):
        appLogo = new Fl_RGB_Image( 
              RNAStructVizLogo.pixel_data, 
              RNAStructVizLogo.width, 
              RNAStructVizLogo.height, 
              RNAStructVizLogo.bytes_per_pixel
	);
        appLogo->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
	Fl_Box *appLogoCont = new Fl_Box(NAVBUTTONS_OFFSETX, 
                                         NAVBUTTONS_OFFSETY, appLogo->w(), appLogo->h());
        appLogoCont->image(appLogo);

        int helpButtonDims = 26;
        int helpButtonOffsetX = 300 - helpButtonDims - NAVBUTTONS_SPACING / 2;
        Fl_Button *helpButton = new Fl_Button(helpButtonOffsetX, NAVBUTTONS_SPACING / 2, 
                                      helpButtonDims, helpButtonDims, "");
        helpButton->color(GUI_WINDOW_BGCOLOR);
        helpButton->labelcolor(GUI_BTEXT_COLOR);
        helpButton->labelsize(2 * LOCAL_TEXT_SIZE);
	helpIconImage->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
        helpButton->image(helpIconImage);
        helpButton->deimage(helpIconImage);
        helpButton->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
        helpButton->labeltype(_FL_ICON_LABEL);
        helpButton->shortcut(FL_CTRL + 'h');
        helpButton->box(FL_NO_BOX);
        helpButton->copy_tooltip("Click for help");
        helpButton->callback(HelpButtonCallback);
	helpButton->redraw();

        int infoButtonDims = 26;
        int infoButtonOffsetX = 300 - helpButtonDims - 3 * NAVBUTTONS_SPACING / 2 - helpButton->w();
        Fl_Button *infoButton = new Fl_Button(infoButtonOffsetX, NAVBUTTONS_SPACING / 2, 
                                              infoButtonDims, infoButtonDims, "");
        infoButton->color(GUI_WINDOW_BGCOLOR);
        infoButton->labelcolor(GUI_BTEXT_COLOR);
        infoButton->labelsize(2 * LOCAL_TEXT_SIZE);
        infoButtonImage->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
	infoButton->image(infoButtonImage);
        infoButton->deimage(infoButtonImage);
        infoButton->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
        infoButton->labeltype(_FL_ICON_LABEL);
        infoButton->shortcut(FL_CTRL + 'i');
        infoButton->box(FL_NO_BOX);
        infoButton->copy_tooltip("Click for information about RNAStructViz");
        infoButton->callback(InfoButtonCallback);
	infoButton->redraw();

        // consistent alignment with the folder window display:
        int upperYOffset = NAVBUTTONS_OFFSETY + appLogo->h() + 5;
        int dividerTextHeight = NAVBUTTONS_SPACING; 

        // make it more user friendly by including some help text: 
        const char *navInstText = "@refresh   Actions.\n  Each action button click\n  expands into a new window.";
        int navButtonsLabelHeight = 2 * NAVBUTTONS_BHEIGHT;
        actionsLabel = new Fl_Box(NAVBUTTONS_OFFSETX, upperYOffset, 
                                  2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING, 
                                  navButtonsLabelHeight, navInstText);     
        actionsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        actionsLabel->color(GUI_BGCOLOR);
        actionsLabel->labelcolor(GUI_BTEXT_COLOR);
        actionsLabel->labelfont(FL_HELVETICA_BOLD);
        actionsLabel->labelsize(LOCAL_TEXT_SIZE);
        actionsLabel->box(FL_RSHADOW_BOX);
	actionsLabel->labeltype(FL_SHADOW_LABEL);

        //Open button
        int btnsOffsetX = NAVBUTTONS_OFFSETX + 7;
	openButton = new Fl_Button(btnsOffsetX, 
                           NAVBUTTONS_OFFSETY + upperYOffset + navButtonsLabelHeight, 
                           NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
                           "@search   Load Files @>|");
        openButton->callback(OpenFileCallback);
        openButton->labelcolor(GUI_BTEXT_COLOR);
        openButton->box(FL_RSHADOW_BOX);
	//openButton->box(FL_PLASTIC_UP_BOX);
	openButton->labelfont(FL_HELVETICA);
	openButton->labeltype(FL_SHADOW_LABEL);

        // The button to open configuration settings:
        configOptionsButton = new Fl_Button( 
              btnsOffsetX + NAVBUTTONS_BWIDTH + NAVBUTTONS_SPACING, 
              NAVBUTTONS_OFFSETY + upperYOffset + navButtonsLabelHeight, 
              NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
              "@menu   User Config @>|"
	);
        configOptionsButton->callback(ConfigOptionsCallback);
        configOptionsButton->labelcolor(GUI_BTEXT_COLOR);
        configOptionsButton->labelfont(FL_HELVETICA);
	configOptionsButton->box(FL_RSHADOW_BOX);
	//configOptionsButton->box(FL_PLASTIC_UP_BOX);
	configOptionsButton->labeltype(FL_SHADOW_LABEL);

        const char *foldersInstText = "@fileopen   Folders.\n  A list of sequences for which\n  "
		                      "structure files are loaded.";
        columnLabel = new Fl_Box(NAVBUTTONS_OFFSETX, 
                  NAVBUTTONS_OFFSETY + upperYOffset + navButtonsLabelHeight + 
                  NAVBUTTONS_BHEIGHT + dividerTextHeight, 
                  2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING, 
                  navButtonsLabelHeight, foldersInstText
        );
        columnLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        columnLabel->color(GUI_BGCOLOR);
        columnLabel->labelcolor(GUI_BTEXT_COLOR);
        columnLabel->labelfont(FL_HELVETICA_BOLD);
        columnLabel->labelsize(LOCAL_TEXT_SIZE);
        columnLabel->box(FL_RSHADOW_BOX);
	columnLabel->labeltype(FL_SHADOW_LABEL);

        m_structureInfo = new Fl_Scroll(0, 105 + upperYOffset + navButtonsLabelHeight + 
                                        dividerTextHeight + 15, 290, 360 - 
                                        upperYOffset - navButtonsLabelHeight - 
                                        dividerTextHeight - 50);
        m_structureInfo->type(Fl_Scroll::VERTICAL_ALWAYS);
	m_structureInfo->color(Darker(GUI_BGCOLOR, 0.23));
        m_packedInfo = new Fl_Pack(0, 105 + upperYOffset + navButtonsLabelHeight + 
                       dividerTextHeight + 15, 270,
                       360 - upperYOffset - navButtonsLabelHeight - dividerTextHeight - 50);
        m_packedInfo->type(Fl_Pack::VERTICAL);

        m_packedInfo->end();
        m_packedInfo->color(GUI_WINDOW_BGCOLOR);
        m_structureInfo->end();
        m_structureInfo->color(GUI_WINDOW_BGCOLOR);
        
        mainMenuPane->resizable(m_structureInfo);
    
    }
    mainMenuPane->end();
    mainMenuPane->color(GUI_WINDOW_BGCOLOR);
    
    menu_collapse = new Fl_Button(300, 0, 15, 450, "@>");
    menu_collapse->callback(CollapseMainCallback);
    menu_collapse->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5));
    menu_collapse->tooltip("Click to collapse or expand main menu pane");
    menu_collapse->deactivate();

    folder_collapse = new Fl_Button(315, 0, 15, 450, "@<");
    folder_collapse->callback(CollapseFolderCallback);
    folder_collapse->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5));
    folder_collapse->tooltip("Click to collapse or expand folder menu pane");
    folder_collapse->deactivate();

    folderWindowPane = new Fl_Group(330, 0, 320, 450, "");
    folderWindowPane->end();
    folderWindowPane->color(GUI_WINDOW_BGCOLOR);
    
    m_mainWindow->resizable(mainMenuPane);
    m_mainWindow->size_range(650, 450, 650, 0);
    m_mainWindow->end();

    m_mainWindow->show(argc, argv);

}

MainWindow::~MainWindow() {
    Delete(m_fileChooserSelectAllBtn, SelectAllButton);    
    Delete(m_fileChooser, Fl_File_Chooser);
    Delete(folderWindowPane, Fl_Group);
    Delete(m_packedInfo, Fl_Pack);
    Delete(m_structureInfo, Fl_Scroll);
    Delete(columnLabel, Fl_Box);
    Delete(actionsLabel, Fl_Box);
    Delete(openButton, Fl_Button);
    Delete(configOptionsButton, Fl_Button);
    Delete(appLogo, Fl_RGB_Image);
    for(int w = 0; w < folderDataBtns.size(); w++) {
        delete folderDataBtns[w];
        folderDataBtns[w] = NULL;
    }
    Delete(m_mainWindow, Fl_Double_Window);
}

bool MainWindow::Initialize(int argc, char **argv) {
    if(ms_instance)
        return true;
    ms_instance = new MainWindow(argc, argv);
    return true;
}

void MainWindow::Shutdown() {
    if(ms_instance) {
        ms_instance->m_mainWindow->hide();
	while(ms_instance->m_mainWindow->visible()) { Fl::wait(1.0); }
	delete ms_instance;
	ms_instance = NULL;
	Delete(MainWindow::helpIconImage, Fl_RGB_Image);
	Delete(MainWindow::infoButtonImage, Fl_RGB_Image);
    }
}

bool MainWindow::IsRunning() {
     return (ms_instance != NULL) && (ms_instance->m_mainWindow != NULL);
}

MainWindow* MainWindow::GetInstance() {
     return ms_instance;
}

void MainWindow::DisplayFirstTimeUserInstructions() {
     CommonDialogs::DisplayFirstRunInstructions();
}

void MainWindow::AddFolder(const char* foldername, const int index, 
                           const bool isSelected) {
    Folder* folder = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(index);
    Fl_Pack* pack = ms_instance->m_packedInfo;
    folder->SetFolderLabel();
    folder->SetTooltipTextData();
    folder->SetSelected(isSelected);
    ms_instance->m_packedInfo->hide();
    ms_instance->m_packedInfo->show();
    ms_instance->m_packedInfo->redraw();
    ms_instance->m_structureInfo->scrollbar.align();
    ms_instance->m_structureInfo->redraw();
}

void MainWindow::OpenFileCallback(Fl_Widget* widget, void* userData)
{
    if(!ms_instance->CreateFileChooser()) {
        fl_alert("Unable to re-create the file chooser! Returning without loading files ...");
	return;
    }
    ms_instance->m_fileChooser->show();
    ms_instance->m_fileChooserSelectAllBtn->redraw();
    while(ms_instance->m_fileChooser->visible()) {
        Fl::wait();
    }

    const char *nextWorkingDir = ms_instance->m_fileChooser->directory();
    if(nextWorkingDir != NULL && strcmp(nextWorkingDir, (char *) CTFILE_SEARCH_DIRECTORY)) { 
         // update the working directory:
         strncpy((char *) CTFILE_SEARCH_DIRECTORY, nextWorkingDir, MAX_BUFFER_SIZE - 1);
         CTFILE_SEARCH_DIRECTORY[strlen(nextWorkingDir)] = '\0';
	 ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
    }

    if(ms_instance->m_fileChooserSelectAllBtn->SelectAllFilesActivated()) {
        SelectAllButton::LoadAllFilesFromDirectory(nextWorkingDir, *(ms_instance->m_fileChooserSelectAllBtn));
    }
    else if(ms_instance->m_fileChooserSelectAllBtn->SelectAllFilesInHomeActivated()) {
	SelectAllButton::LoadAllFilesFromDirectory(GetUserHome(), *(ms_instance->m_fileChooserSelectAllBtn));
    }
    else {
        for (int i = 0; i < ms_instance->m_fileChooser->count(); ++i) {
            const char *nextFilename = strrchr(ms_instance->m_fileChooser->value(i), '/');
            nextFilename = nextFilename ? nextFilename : ms_instance->m_fileChooser->value(i);
            if(!strcmp(nextFilename, "") || !strcmp(nextFilename, ".") || 
               !strcmp(nextFilename, "..") || 
               ConfigParser::directoryExists(nextFilename)) { // invalid file to parser, so ignore it:
                continue;
            }
            if(strlen(nextFilename) + strlen(nextWorkingDir) + 1 >= MAX_BUFFER_SIZE) {
                 fl_alert("Unable to open file: %s\nTotal file path name exceeds %d bytes.", 
                          nextFilename, MAX_BUFFER_SIZE);
                 continue;
            }
            char nextFilePath[MAX_BUFFER_SIZE];
            snprintf(nextFilePath, MAX_BUFFER_SIZE, "%s%s%s\0", nextWorkingDir, 
                     nextWorkingDir[strlen(nextWorkingDir) - 1] == '/' ? "" : "/", 
                     nextFilename);
            RNAStructViz::GetInstance()->GetStructureManager()->AddFile(nextFilePath);
        }
    }
    
    ms_instance->m_packedInfo->redraw();
    ms_instance->folderWindowPane->redraw();

}

void MainWindow::OpenFileCallback() {
     if(MainWindow::ms_instance != NULL) {
          MainWindow::OpenFileCallback(NULL, NULL);
     }
}

void MainWindow::ConfigOptionsCallback(Fl_Widget* widget, void* userData) {
     DisplayConfigWindow *cfgWindow = new DisplayConfigWindow(); 
     cfgWindow->show();
     while(!cfgWindow->isDone() && cfgWindow->visible()) {
          Fl::wait();
     }
     Delete(cfgWindow, DisplayConfigWindow);
}

void MainWindow::ConfigOptionsCallback() {
     if(MainWindow::ms_instance != NULL) {
          MainWindow::ConfigOptionsCallback(NULL, NULL);
     }
}

void MainWindow::TestCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->TestFolders();
}

void MainWindow::HelpButtonCallback(Fl_Widget *btn, void *udata) {
     CommonDialogs::DisplayHelpDialog();
}

void MainWindow::InfoButtonCallback(Fl_Widget *btn, void *udata) {
     CommonDialogs::DisplayInfoAboutDialog();
}

void MainWindow::CollapseMainMenu()
{
    Fl_Group* mainMenu = ms_instance->mainMenuPane;
    Fl_Button* button = ms_instance->menu_collapse;
    if (ms_instance->folderWindowPane->visible())
    {
        if (mainMenu->visible())
        {
            mainMenu->hide();
            button->label("@<");
            button->labelcolor(GUI_TEXT_COLOR);
            
            int x = ms_instance->m_mainWindow->x() + 300;
            int y = ms_instance->m_mainWindow->y();
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(350, 450);
            ms_instance->m_mainWindow->position(x, y);
            
            ms_instance->menu_collapse->position(0, 0);
            ms_instance->folder_collapse->position(15, 0);
            ms_instance->folderWindowPane->position(30, 0);
            
            ms_instance->folder_collapse->deactivate();
        }
        else 
        {
            mainMenu->show();
            button->label("@>");
            button->labelcolor(GUI_TEXT_COLOR);
            
            int x = ms_instance->m_mainWindow->x() - 300;
            if (x < 0)
                x = 0;
            int y = ms_instance->m_mainWindow->y();            
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->position(x,y);
            ms_instance->m_mainWindow->size(650, 450);
            
            ms_instance->menu_collapse->position(300, 0);
            ms_instance->folder_collapse->position(315, 0);
            ms_instance->folderWindowPane->position(330, 0);
            
            ms_instance->folder_collapse->activate();
        }
    }
}

void MainWindow::CollapseMainCallback(Fl_Widget* widget, void* userData)
{
    ms_instance->CollapseMainMenu();
}

void MainWindow::CollapseFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    if (ms_instance->mainMenuPane->visible())
    {
        if (folderMenu->visible())
        {
            folderMenu->hide();
            button->label("@>");
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(330, 450);
            ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
            ms_instance->menu_collapse->deactivate();
        }
        else 
        {
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(650, 450);
            button->label("@<");
            folderMenu->show();
            ms_instance->menu_collapse->activate();
        }
    }
}

void MainWindow::CollapseAlwaysFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    if (!ms_instance->mainMenuPane->visible())
    {
        MainWindow::CollapseMainMenu();
    }
    if (folderMenu->visible())
    {
        folderMenu->hide();
        button->label("@>");
        ms_instance->m_mainWindow->resizable(NULL);
        ms_instance->m_mainWindow->size(330, 450);
        ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
        ms_instance->menu_collapse->deactivate();
    }
}

void MainWindow::ExpandAlwaysFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    ms_instance->m_mainWindow->resizable(NULL);
    ms_instance->m_mainWindow->size(650, 450);
    button->label("@<");
    folderMenu->show();
    ms_instance->menu_collapse->activate();
}

void MainWindow::CollapseFolderCallback(Fl_Widget* widget, void* userData)
{
    ms_instance->CollapseFolderPane();
}

void MainWindow::MoveFolderUp(Fl_Widget *widget, void* userData)
{
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Group* folderGroup = (Fl_Group*)widget->parent();
    
    int index = pack->find(folderGroup);
    if (index > 0)
        pack->insert(*folderGroup,index-1);
    
    pack->redraw();
}

void MainWindow::MoveFolderDown(Fl_Widget *widget, void* userData)
{
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Group* folderGroup = (Fl_Group*)widget->parent();
    
    int index = pack->find(folderGroup);
    pack->insert(*folderGroup,index + 2);
    
    pack->redraw();
}

bool MainWindow::CreateFileChooser() {

    // Get the current working directory.
    char currentWD[MAX_BUFFER_SIZE];
    currentWD[0] = '\0';
    if(ConfigParser::directoryExists((char *) CTFILE_SEARCH_DIRECTORY)) { 
        strncpy(currentWD, (char *) CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1); 
    }
    else if (!getcwd(currentWD, MAX_BUFFER_SIZE)) {
	TerminalText::PrintError("Error: getcwd failed. Cannot create file chooser.\n");
        return false;
    }
    Delete(m_fileChooser, Fl_File_Chooser);
    if(m_fileChooser == NULL) {
         m_fileChooser = new Fl_File_Chooser(NULL, NULL, Fl_File_Chooser::MULTI, NULL);
    }
    m_fileChooser->label("Select RNA Structures From File(s) ...");
    m_fileChooser->filter(
                 #if WITH_FASTA_FORMAT_SUPPORT > 0
		 "All Formats (*.{boltz,ct,nopct,dot,bracket,dbn,fasta,helix,hlx,bpseq})\t"
                 #else
		 "All Formats (*.{boltz,ct,nopct,dot,bracket,dbn,helix,hlx,bpseq})\t"
                 #endif
		 "CT Files (*.{nopct,ct})\t"
                 "DOT Bracket (*.{dot,bracket,dbn})\t"
                 "Boltzmann Format (*.boltz)\t"
                 "Helix Triple Format (*.{helix,hlx})\t"
                 "SEQ Files (*.bpseq)\t"
                 #if WITH_FASTA_FORMAT_SUPPORT > 0
                 "FASTA Files (*.fasta)\t"
                 #endif
		 "All Files (*)"
            );
    m_fileChooser->directory(currentWD);
    m_fileChooser->preview(true);
    m_fileChooser->textcolor(GUI_TEXT_COLOR);
    m_fileChooser->color(GUI_WINDOW_BGCOLOR);
    m_fileChooser->showHiddenButton->value(true); // show hidden files by default
    m_fileChooser->favorites_label = "  @search  Goto Favorites ...";
     
    // add select all button:
    Delete(m_fileChooserSelectAllBtn, SelectAllButton);
    m_fileChooserSelectAllBtn = new SelectAllButton(m_fileChooser);
    m_fileChooser->add_extra(m_fileChooserSelectAllBtn);

    return true;
}

void MainWindow::ShowFolderCallback(Fl_Widget* widget, void* userData)
{
    //Find the folderName label in the contentsButton widget's group
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
    ms_instance->selectedFolderBtn = (Fl_Button *) widget;

    Fl_Pack* pack = ms_instance->m_packedInfo;
    folderLabel->color(Lighter(GUI_BGCOLOR, 0.5f));
    folderLabel->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
    
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;

    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName, (char*)(folderLabel->user_data())))
            break;
    }
    ShowFolderByIndex(index);

}

void MainWindow::ShowFolderByIndex(int index) { 

    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->
                                          GetStructureManager()->GetFolders();
    FolderWindow* fwindow;
    if (folders[index]->folderWindow == NULL)
    {
        fwindow = new FolderWindow(340, 40, 300, 390, 
                                   folders[index]->folderName, index);
	folders[index]->folderWindow = fwindow;
    }
    else 
    {
        fwindow = folders[index]->folderWindow;
    }

    if (ms_instance->folderWindowPane->children() > 0) {
         ms_instance->folderWindowPane->remove(0);
    }
    ms_instance->selectedFolderIndex = index;
    ms_instance->folderWindowPane->add((Fl_Group*) fwindow);
    ms_instance->ShowFolderSelected();

    ExpandAlwaysFolderPane();
    ms_instance->folderWindowPane->hide();
    ms_instance->folderWindowPane->show();
    ms_instance->folderWindowPane->redraw();

}

void MainWindow::ShowFolderSelected()
{
    //Find the folderName label in the contentsButton widget's group
    Fl_Button* folderLabel = NULL;
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Folder *selectedFolder = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt( 
		             ms_instance->selectedFolderIndex);
    Fl_Button *selectedBtn = selectedFolder != NULL ? selectedFolder->mainWindowFolderBtn : NULL;
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button *childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if(childLabel == selectedBtn) {
	     childLabel->color(Lighter(GUI_BGCOLOR, 0.5f));
             childLabel->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
	     folderLabel = childLabel;
        }
	else {
	     childLabel->color(GUI_BGCOLOR);
	     childLabel->labelcolor(GUI_BTEXT_COLOR);
	}
    }
    
    if (folderLabel != NULL) {
        
        const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
        int index;
        for (index = 0; index < folders.size(); ++index)
        {
            if (!strcmp(folders[index]->folderName,(char*)(folderLabel->user_data())))
                break;
        }
        FolderWindow* fwindow;
        if (folders[index]->folderWindow == NULL)
        {
            fwindow = new FolderWindow(340, 40, 300, 390, folders[index]->folderName, index);
	    folders[index]->folderWindow = fwindow;
        }
        else
        {
            fwindow = folders[index]->folderWindow;
        }
        
        if (ms_instance->folderWindowPane->children() > 0) {
            ms_instance->folderWindowPane->remove(0);
        }

        ms_instance->folderWindowPane->add((Fl_Group*)fwindow);

        ExpandAlwaysFolderPane();
        ms_instance->folderWindowPane->hide();
        ms_instance->folderWindowPane->show();

    }
}

bool MainWindow::CheckDistinctFolderName(const char *nextFolderName) {
    for(int btn = 0; btn < ms_instance->folderDataBtns.size(); btn++) {
        if(!strcmp(nextFolderName, ms_instance->folderDataBtns[btn]->label())) {
        return false;
    }
    }
    return true;
}

void MainWindow::HideFolderByIndex(const int index)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Folder* folder = appInstance->GetStructureManager()->GetFolderAt(index);
    Fl_Group* pane = ms_instance->folderWindowPane;
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    TerminalText::PrintInfo("Hiding folder with label \"%s\"\n", folder->folderName);

    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()), folder->folderName)) {
             childLabel->color(FL_BACKGROUND_COLOR);
	     childLabel->labelcolor(FL_BACKGROUND_COLOR);
	}
	else {
	     childLabel->color(GUI_BGCOLOR);
	     childLabel->labelcolor(GUI_BTEXT_COLOR);
	}
    }
    
    if (pack->children() > 0)
    {
         Fl_Widget *fwinToRemove = pack->child(0);
	 pack->remove((Fl_Widget *) fwinToRemove);
         pack->redraw();
	 pane->redraw();
    }
    else {
	 pane->redraw();
    }
    pane->hide();
    pane->show();

}

void MainWindow::HideFolderByName(const char* foldername)
{   
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()), foldername + 5))
            childLabel->color(FL_BACKGROUND_COLOR);
        childLabel->labelcolor(GUI_BTEXT_COLOR);
    }
    
    Fl_Group* pane = ms_instance->folderWindowPane;
    if (pane->children() > 0)
    {
        Fl_Group* childGroup = (Fl_Group*)(pane->child(0));
        if (!strcmp(childGroup->label(), foldername)) {
             FolderWindow *fwinToRemove = (FolderWindow *) pane->child(pane->children() - 1);
             pane->remove((Fl_Widget *) fwinToRemove);
	     Delete(fwinToRemove, FolderWindow);
        }
    }
    pane->hide();
    pane->show();
}

void MainWindow::RemoveFolderByIndex(const int index, bool selectNext)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    const std::vector<Folder*>& folders = appInstance->GetStructureManager()->GetFolders();
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childButton->user_data()),folders[index]->folderName)) {
            
            Fl_Group* toRemove = (Fl_Group*) pack->child(i);
            pack->remove(toRemove);
	    pack->insert(*toRemove, 0);
            
	    const std::vector<DiagramWindow*>& diagrams = appInstance->GetDiagramWindows();
            const std::vector<StatsWindow*>& stats = appInstance->GetStatsWindows();
            int structNum = folders[index]->structCount;
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int structIndex = folders[index]->folderStructs[j];
                        diagrams[ui]->RemoveStructure(structIndex);
                    }
                    diagrams[ui]->hide();
                }
            }
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int structIndex = folders[index]->folderStructs[j];
                        stats[ui]->RemoveStructure(structIndex);
                    }
                    stats[ui]->hide();
                }
            }
	    break;
        }
        
    }
    for(int si = 0; si < folders[index]->structCount; si++) {
        int sindex = folders[index]->folderStructs[si];
	appInstance->GetStructureManager()->RemoveStructure(sindex);
	//Delete(FolderWindow::m_storedStructDisplayData[sindex], StructureData);
    }
    //appInstance->GetStructureManager()->GetFolderAt(index)->SetPerformWidgetDeletion(false);
    appInstance->GetStructureManager()->RemoveFolder(index);
    ms_instance->m_packedInfo->hide();
    ms_instance->m_packedInfo->show();
    ms_instance->m_packedInfo->redraw();
    ms_instance->m_structureInfo->scrollbar.align();
    ms_instance->m_structureInfo->redraw();

    if(index == ms_instance->selectedFolderIndex) {
         ms_instance->selectedFolderIndex = -1;
    }

    if(selectNext && folders.size() > 0) { 
        // select the next folder in line:
        Fl_Group* pane = ms_instance->folderWindowPane;
        pane->hide();
        pane->show();
        int nextIndex = index % folders.size();    
        if(folders[nextIndex] != NULL && folders[nextIndex]->structCount > 0) { 
             ShowFolderByIndex(nextIndex);
	     Fl_Button *folderLabel = folders[nextIndex]->mainWindowFolderBtn;
             folderLabel->color(Lighter(GUI_BGCOLOR, 0.5f));
             folderLabel->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
             ms_instance->selectedFolderBtn = folderLabel;
        }
    }
    else {
        Fl_Group* pane = ms_instance->folderWindowPane;
        pane->hide();
        pane->show();
    }
}

void MainWindow::RemoveFolderCallback(Fl_Widget* widget, void* userData)
{
    // Find the group with this child
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
                
    for(int btn = 0; btn < ms_instance->folderDataBtns.size(); btn++) {
        if(ms_instance->folderDataBtns[btn] == folderLabel) {
             ms_instance->folderDataBtns.erase(ms_instance->folderDataBtns.begin() + btn);
             break;
        }
    }

    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    unsigned int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName, (char*)(folderLabel->user_data())))
            break;
    }
    ms_instance->RemoveFolderByIndex(index, true);
    
    /*for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(3));
        if (childButton == widget)
        {
            Fl_Group* toRemove = (Fl_Group*) pack->child(i);
	    pack->remove(toRemove);
	    pack->insert(*toRemove, pack->children());
            Folder* folder = appInstance->GetStructureManager()->GetFolderAt(index);
            const std::vector<DiagramWindow*>& diagrams = appInstance->GetDiagramWindows();
            const std::vector<StatsWindow*>& stats = appInstance->GetStatsWindows();
            int structNum = folder->structCount;
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int index = folder->folderStructs[j];
                        diagrams[ui]->RemoveStructure(index);
                    }
                    diagrams[ui]->hide();
                }
            }
            
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int index = folder->folderStructs[j];
                        stats[ui]->RemoveStructure(index);
                    }
                    stats[ui]->hide();
                }
            }
            HideFolderByIndex(index);
            
            int shift = 0;
            for (int j = 0; j < folder->structCount; j++)
            {
                if(folder->folderStructs[(j+shift)] == -1)
                {
                    shift++;
                }
                if(folder->folderStructs[(j+shift)] != -1)
                {   
                    int index = folder->folderStructs[(j+shift)];
                    appInstance->GetStructureManager()->RemoveStructure(index);
                }
            }
            //appInstance->GetStructureManager()->RemoveFolder(i);
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if (diagrams[ui]->GetFolderIndex() > index)
                {
                    int prevIndex = diagrams[ui]->GetFolderIndex();
                    diagrams[ui]->SetFolderIndex(prevIndex - 1);
                }
            }
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() > index)
                {
                    int prevIndex = stats[ui]->GetFolderIndex();
                    stats[ui]->SetFolderIndex(prevIndex - 1);
                }
            }
            
            ms_instance->m_packedInfo->hide();
	    ms_instance->m_packedInfo->show();
            ms_instance->m_packedInfo->redraw();
	    ms_instance->m_structureInfo->scrollbar.align();
            ms_instance->m_structureInfo->redraw();
            break;
        }
    }
    appInstance->GetStructureManager()->DecreaseStructCount(index);
    ms_instance->m_packedInfo->hide();
    ms_instance->m_packedInfo->show();
    ms_instance->m_packedInfo->redraw();
    ms_instance->m_structureInfo->scrollbar.align();
    ms_instance->m_structureInfo->redraw();*/

}

void MainWindow::CloseCallback(Fl_Widget* widget, void* userData) {
    widget->hide();
    exit(0);
}

void MainWindow::ResetThemeColormaps() {
     Fl::set_color(FL_BACKGROUND2_COLOR, GUI_BGCOLOR);
     Fl::set_color(FL_BACKGROUND_COLOR, GUI_BGCOLOR);
     Fl::set_color(FL_FOREGROUND_COLOR, GUI_BTEXT_COLOR);
     Fl::set_color(FL_INACTIVE_COLOR, Darker(GUI_BTEXT_COLOR, 0.35f));
     Fl::set_color(FL_SELECTION_COLOR, Lighter(GUI_BTEXT_COLOR, 0.4f));
     
     // the following two options respectively get rid of the red label 
     // ugliness present in the shadow box types and the Fl_Scroll 
     // scrollbar bases colors when using dark backgrounds:
     Fl::set_color(FL_DARK3, Lighter(GUI_BGCOLOR, 0.85f));
     Fl::set_color(FL_DARK2, Lighter(GUI_BGCOLOR, 0.7f));
}

void MainWindow::RethemeMainWindow() {
         
         ResetThemeColormaps();

         if(ms_instance == NULL) {
              return;
         }
         ms_instance->m_mainWindow->color(GUI_WINDOW_BGCOLOR);
             ms_instance->m_mainWindow->redraw();
             if(ms_instance->m_packedInfo) {
              ms_instance->m_packedInfo->color(GUI_WINDOW_BGCOLOR);
              ms_instance->m_packedInfo->redraw();
         }
         if(ms_instance->m_structureInfo) { 
              ms_instance->m_structureInfo->color(GUI_WINDOW_BGCOLOR);
              ms_instance->m_structureInfo->labelcolor(GUI_BTEXT_COLOR);
              ms_instance->m_structureInfo->redraw();
         }
         ms_instance->menu_collapse->color(GUI_BGCOLOR);
         ms_instance->menu_collapse->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->menu_collapse->redraw();
         ms_instance->folder_collapse->color(GUI_BGCOLOR);
         ms_instance->folder_collapse->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->folder_collapse->redraw();
         ms_instance->columnLabel->labelcolor(GUI_TEXT_COLOR);
         ms_instance->columnLabel->redraw();
         ms_instance->actionsLabel->labelcolor(GUI_TEXT_COLOR);
         ms_instance->actionsLabel->redraw();
         ms_instance->openButton->color(GUI_BGCOLOR);
         ms_instance->openButton->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->openButton->redraw();
         ms_instance->configOptionsButton->color(GUI_BGCOLOR);
         ms_instance->configOptionsButton->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->configOptionsButton->redraw();
         
         for(int b = 0; b < ms_instance->folderDataBtns.size(); b++) {
              ms_instance->folderDataBtns[b]->color(GUI_BGCOLOR);
              ms_instance->folderDataBtns[b]->labelcolor(GUI_BTEXT_COLOR);
         }
         if(ms_instance->selectedFolderBtn != NULL) {
              ms_instance->selectedFolderBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
              ms_instance->selectedFolderBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
         }
         ms_instance->actionsLabel->color(GUI_BGCOLOR);
         ms_instance->actionsLabel->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->actionsLabel->redraw();
         ms_instance->columnLabel->color(GUI_BGCOLOR);
         ms_instance->columnLabel->labelcolor(GUI_BTEXT_COLOR);
         ms_instance->columnLabel->redraw();
         if(ms_instance->selectedFolderIndex >= 0) {
                  FolderWindow *curFolderWin = (FolderWindow *) 
                   RNAStructViz::GetInstance()->
                   GetStructureManager()->
                   GetFolders()[ms_instance->selectedFolderIndex];
          curFolderWin->RethemeFolderWindow();
         }
         Fl::scheme((char *) FLTK_THEME);
         Fl::redraw();
         Fl::flush();
}
