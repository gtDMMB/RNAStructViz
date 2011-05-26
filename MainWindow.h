/**
    Singleton class for the main UI window of RNAStructViz.

    An instance of this class is created by the RNAStructViz main method. It manages the UI elements in the main window.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Fl/Fl.h>
#include <Fl/Fl_Window.h>
#include <Fl/Fl_File_Chooser.h>
#include <Fl/Fl_Pack.h>
#include <Fl/Fl_Scroll.h>

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
	Add a new sequence to the list of displayed sequences.
    */
    static void AddStructure(const char* filename, const int index, const bool isReference);

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

    /*
	Callback to quit.
    */
    //static void QuitCallback(Fl_Widget* widget, void* userData);

    /*
	Callback to make a structure the reference.
    */
    static void ReferenceCallback(Fl_Widget* widget, void* userData);

    /*
	Callback to open a diagram window.
    */
    static void DiagramCallback(Fl_Widget* widget, void* userData);

    /*
	Callback to show/hide a file.
    */
    static void ShowFileCallback(Fl_Widget* widget, void* userData);

    /*
	Callback to remove a structure.
    */
    static void RemoveCallback(Fl_Widget* widget, void* userData);

    /*
	Callback invoked when the main window is closed.
    */
    static void CloseCallback(Fl_Widget* widget, void* userData);

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
	The scrolling group containig the sequence manager info, and the packed group inside it.
    */
    Fl_Scroll* m_structureInfo;
    Fl_Pack* m_packedInfo;

    /*
	The file chooser dialog.
    */
    Fl_File_Chooser* m_fileChooser;
};

#endif // MAINWINDOW_H
