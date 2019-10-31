#ifndef INPUT_WINDOW_H
#define INPUT_WINDOW_H

#include <string>
#include <vector>
using std::string;
using std::vector;

#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>

/* 
InputWindow is a pop-up window for taking in user input to specify the name of 
something. There are two types:

1. FOLDER_INPUT - takes input for the name of a folder of structures in the
   structure manager
2. FILE_INPUT - takes input for the filename of the exported table of 
   statistics in the StatsWindow

The window's callbacks close the window if the user either presses enter in the
input text field, pushes the 'OK' button, or closes the window. If no text has
been entered by the user, then the 'name' field will contain the empty string.
*/

class InputWindow : public Fl_Window
{
    public:

        static const int CANCELED = 0;
        static const int OK       = 1;

	/* 	Denotes whether the InputWindow is being used for entering the name of
	    	a folder (for the StructureManager) or for a file (for exporting in
	    	a StatsWindow) 
        */
    	enum InputWindowType {
	        FOLDER_INPUT,
        	FILE_INPUT, 
		CTVIEWER_FILE_INPUT, 
		RADIAL_LAYOUT_FILE_INPUT
    	};
	
    	/* 
	    w - width
	    h - height
	    label - name for the window to appear in the top bar of the window
	    defaultName - default value for the input, displayed in the window (if 
	    	no text is inputted, the InputWindow returns ""; the parent widget must
		    handle the conditioning to replace this with the default text)
	    type - types listed above
	    */
        InputWindow(int w, int h, const char *label, const char *defaultName,
    	            InputWindowType type, int folderIndex = -1);
	~InputWindow();

	    /* Returns the user input from the window*/
        inline char* getName() const {
            return name;
        }
        
	int getFileSelectionIndex() const;
	bool isCanceled() const;
	
	static int distinctStructureCount;
    
    protected:

	/* 	Callbacks for whether the user presses enter, pushes the button,
	    	or closes the InputWindow
        */
        static void InputCallback(Fl_Widget *widget, void *userdata);
        static void CloseCallback(Fl_Widget* widget, void* userData);
        static void DisplayCTFileCallback(Fl_Widget *w, void *udata);
	static void CancelCallback(Fl_Widget *w, void *udata);

	static std::string ExtractStructureNameFromFile(const char *ctPath);

    private:
        Fl_Input *input; // Text field where the user types in the input
        Fl_Check_Button *cbUseDefaultNames;
	Fl_Check_Button *cbKeepStickyFolders;
	Fl_Choice *ctFileChooser;
	InputWindowType windowType;
	char *name; // The input; if the user types nothing, is the empty string
	char *string; // For displaying the default name
	char *inputText;
	int userWindowStatus;
	int fileSelectionIndex;
};

#endif
