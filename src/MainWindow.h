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
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>

#define NAVBUTTONS_OFFSETX   (10)
#define NAVBUTTONS_OFFSETY   (10) 
#define NAVBUTTONS_BWIDTH    (125)
#define NAVBUTTONS_BHEIGHT   (30)
#define NAVBUTTONS_SPACING   (15)

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
    //Fl_Tabs* folder_tabs;
    
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
    Fl_Window* m_mainWindow;

    /*
	The scrolling group containing the sequence manager info, and the packed 
	group inside it.
    */
    Fl_Scroll* m_structureInfo;
    Fl_Pack* m_packedInfo;

    /*
	The file chooser dialog.
    */
    Fl_File_Chooser* m_fileChooser;
};

#endif // MAINWINDOW_H
