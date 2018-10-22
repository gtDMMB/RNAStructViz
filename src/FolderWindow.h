/*
    The window that shows the files in a folder.
 */

#ifndef FOLDERWINDOW_H
#define FOLDERWINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_RGB_Image.H>
#include <vector>

class FolderWindow : public Fl_Group
{

    public:
        void Construct(int w, int h, int folderIndex);
        FolderWindow(int x, int y, int wid, int hgt, const char *label, int folderIndex);
	virtual ~FolderWindow();

        /*
         Add a new sequence to the list of displayed sequences.
         */
        void AddStructure(const char* filename, const int index);

        /*
         Sets up all the structures in the folder.
         */
        void SetStructures(int folderIndex);

    private:

        /*
         Callback to show/hide a file.
         */
        static void ShowFileCallback(Fl_Widget* widget, void* userData);
        static void CloseFolderCallback(Fl_Widget* widget, void* userData);

        /*
         Callback to remove a structure.
         */
        static void RemoveCallback(Fl_Widget* widget, void* userData);

        static void DiagramCallback(Fl_Widget* widget, void* userData);
        static void StatsCallback(Fl_Widget* widget, void* userData);

	/*
         The scrolling group containing the folder info and files, and the packed group inside it.
         */
        Fl_Scroll *folderScroll;
        Fl_Pack *folderPack;
        
	/* The watermark structure icon at the top of the folder window. */
	Fl_RGB_Image *structureIcon;

        /*
         folderIndex of the folder being shown.
         */
        int m_folderIndex;

        // Holds the title of the window
        char* title;

};

#endif
