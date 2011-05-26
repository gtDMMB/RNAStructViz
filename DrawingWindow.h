/*
    The window into which structures are drawn.
*/

#ifndef DRAWINGWINDOW_H
#define DRAWINGWINDOW_H

#include <FL/Fl_Double_Window.h>
#include <vector>

class DrawingWindow : public Fl_Double_Window
{
public:
    // Constructors
    DrawingWindow(int w, int h, const char *label = 0);
    DrawingWindow(int x, int y, int w, int h, const char *label = 0);

    // Virtual destructor
    virtual ~DrawingWindow();

    // Manage active structures
    void AddActive(const int index);
    void RemoveActive(const int index);

protected:
    /*
	Draws the contents of the window.
    */
    void draw();

private:
    std::vector<int> m_activeIndices;

    bool m_rebuildRequired;
};

#endif
