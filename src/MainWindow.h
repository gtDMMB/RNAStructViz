/**
    Singleton class for the main UI window of RNAStructViz.

    An instance of this class is created by the RNAStructViz main method. It 
    manages the UI elements in the main window.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>

#include <vector>

#include "ConfigOptions.h"
#include "RNAStructViz.h"
#include "FolderWindow.h"

#define NAVBUTTONS_OFFSETX   (5)
#define NAVBUTTONS_OFFSETY   (10) 
#define NAVBUTTONS_BWIDTH    (135)
#define NAVBUTTONS_BHEIGHT   (30)
#define NAVBUTTONS_SPACING   (8)

class MainWindow
{
    public:
        /*
	    Initialize and display the main window.
        */
        static bool Initialize(int argc, char **argv);

        /*
	    Shut down the windows, which should also cause the application to exit.
        */
        static void Shutdown();
    
        /*
         Add a new folder to the list of displayed folders
         */
        static void AddFolder(const char* foldername, const int index, 
    	                      const bool isSelected);
    
        static void RemoveFolderByIndex(const int index);
    
        /*
         Removes the folder contents group from the folder tabs pane.
         */
        static void HideFolderByIndex(const int index);
        static void HideFolderByName(const char* foldername);
    
        /*
         The tabs for displaying folder contents.
         */
    
        Fl_Group* mainMenuPane;
        Fl_Group* folderWindowPane;
    
        Fl_Button* menu_collapse;
        Fl_Button* folder_collapse;
    
        static void ShowFolderSelected();

    protected:
        /*
	    Protected constructor to enforce singleton semantics.
        */
        MainWindow(int argc, char **argv);

        /*
	    Protected destructor to enforce singleton semantics.
        */
        ~MainWindow();

        /*
	    Callback for file loading.
        */
        static void OpenFileCallback(Fl_Widget* widget, void* userData);

        /* Callback for options / config setup. */
        static void ConfigOptionsCallback(Fl_Widget *widget, void *userData);

        /*
         Callback to show/hide a folder
         */
        static void ShowFolderCallback(Fl_Widget* widget, void* userData);

        /*
	    Callback to remove a structure.
        */
        static void RemoveFolderCallback(Fl_Widget* widget, void* userData);
    
        /*
	    Callback invoked when the main window is closed.
        */
        static void CloseCallback(Fl_Widget* widget, void* userData);
    
        /*
         Callback for testings
         */
        static void TestCallback(Fl_Widget *widget, void* userData);
    
	/* Callback for displaying the help dialog */
        static void HelpButtonCallback(Fl_Widget *btn, void *userData);
        static Fl_RGB_Image *helpIconImage;

	/*
        Callbacks for collapsing the two sides of the window.
        */
        static void CollapseMainCallback(Fl_Widget *widget, void* userData);
        static void CollapseFolderCallback(Fl_Widget *widget, void* userData);
    
        static void CollapseMainMenu();
        static void CollapseFolderPane();

        static void ExpandAlwaysFolderPane();
        static void CollapseAlwaysFolderPane();
    
        /*
        Callbacks for moving folders up and down in the folder list.
        */
        static void MoveFolderUp(Fl_Widget *widget, void* userData);
        static void MoveFolderDown(Fl_Widget *widget, void* userData);
    
        /*
	    Create the file chooser dialog and set it to the current working directory.
        */
        bool CreateFileChooser();

        /*
	    The instance of the window.
	    Only non-null after a call to Initialize and before the next Shutdown.
        */
        static MainWindow* ms_instance;

        /*
	    The main FLTK window.
        */
        Fl_Double_Window* m_mainWindow;

        /*
	    The scrolling group containing the sequence manager info, and the packed 
	    group inside it.
        */
	Fl_Scroll* m_structureInfo;
        Fl_Pack* m_packedInfo;
        Fl_Button *selectedFolderBtn;
        int selectedFolderIndex;

        #define ExtractWidgetIndex(w)            ((long int) (w->user_data()));

        /*
	    The file chooser dialog.
        */
        Fl_Native_File_Chooser* m_fileChooser;

	/* Other widgets that need to be updated when we change the theme settings */
	Fl_Box *columnLabel, *actionsLabel;
	Fl_Box *topTextDivider, *midTextDivider;
	Fl_Button *openButton, *configOptionsButton;
	std::vector<Fl_Button *> folderDataBtns;

    public:
	/* Resets the color theme for the window */
	static inline void RethemeMainWindow() {

	     Fl::foreground(GetRed(LOCAL_FGCOLOR), 
			    GetGreen(LOCAL_FGCOLOR), 
			    GetBlue(LOCAL_FGCOLOR));
	     Fl::background(GetRed(GUI_BGCOLOR), 
			    GetGreen(GUI_BGCOLOR), 
			    GetBlue(GUI_BGCOLOR)); 
             Fl::background2(GetRed(LOCAL_BG2COLOR), 
			     GetGreen(LOCAL_BG2COLOR), 
			     GetBlue(LOCAL_BG2COLOR));
	     
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
                  ms_instance->selectedFolderBtn->color(FL_LIGHT2);
		  ms_instance->selectedFolderBtn->labelcolor(FL_DARK1);
	     }
	     if(ms_instance->selectedFolderIndex >= 0) {
                  FolderWindow *curFolderWin = (FolderWindow *) 
			       RNAStructViz::GetInstance()->
			       GetStructureManager()->
			       GetFolders()[ms_instance->selectedFolderIndex];
	          curFolderWin->RethemeFolderWindow();
	          if(ms_instance->selectedFolderBtn != NULL) { 
		       ShowFolderCallback(ms_instance->selectedFolderBtn, NULL);
		  }
	     }
	     Fl::scheme(FLTK_THEME);
	     Fl::redraw();
	     Fl::flush();
	}

};

#endif // MAINWINDOW_H
