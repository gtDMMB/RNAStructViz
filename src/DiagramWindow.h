/*
    The window into which structures are drawn.
*/

#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <FL/Fl_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/x.H>
#include <vector>

#include "GLWindow.h"
#include "RNAStructure.h"

class DiagramWindow : public Fl_Window
{
public:
    // Constructors
    void Construct(int w, int h, const std::vector<int>& structures);
    DiagramWindow(int w, int h, const char *label, 
    	const std::vector<int>& structures);
    DiagramWindow(int x, int y, int w, int h, const char *label, 
    	const std::vector<int>& structures);

    // Virtual destructor
    virtual ~DiagramWindow();

    // Manage known structures
    void AddStructure(const int index);
    void RemoveStructure(const int index);
    void SetStructures(const std::vector<int>& structures);
    
    void ResetWindow();
    
    inline int GetFolderIndex()
    {
        return folderIndex;
    }
    
    void SetFolderIndex(int index);
    
protected:
    /*
	Draws the contents of the window.
    */
    void draw();
    
    void resize(int x, int y, int w, int h);

private:
    static const int ms_menu_minx[3];
    static const int ms_menu_width;

    void RebuildMenus();

    void RedrawBuffer(RNAStructure** structures, const int numStructures, 
    	const int resolution);

	/* Draws the color legend for the arcs. Input a and b correspond to the
	   index of the relevant structures*/
    void DrawKey3(); // if 3 structures are selected
    void DrawKey2(const int a, const int b); // if 2 selected structures
    void DrawKey1(const int a); // if 1 selected structure

	/* Draws the arcs for all the base pairs, colored according to their 
	   corresponding structures */
    void Draw3(RNAStructure** structures, const int resolution); // 3 structures
    void Draw2(RNAStructure** structures, const int resolution); // 2 structures
    void Draw1(RNAStructure** structures, const int resolution); // 1 structure
    
    /* Computes the numbers for the base pairs, updates the counters in the 
       legend */
    void ComputeNumPairs(RNAStructure** structures, int numStructures);

    void ComputeCircle(
		const float& x1,
		const float& y1,
		const float& x2,
		const float& y2,
		const float& x3,
		const float& y3,
		double& cX,
		double& cY,
		double& r);

    void DrawArc(
		const unsigned int b1,
		const unsigned int b2,
		const float centerX,
		const float centerY,
		const float angleBase,
		const float angleDelta,
		const float radius);

    void DrawBase(
		const unsigned int index,
		const RNAStructure::Base base,
		const float centerX,
		const float centerY,
		const float angleBase,
		const float angleDelta,
		const float radius);

    void ComputeDiagramParams(
		const int numBases,
		const int resolution,
		float& centerX,
		float& centerY,
		float& angleBase,
		float& angleDelta,
		float& radius);

    static void MenuCallback(Fl_Widget* widget, void* userData);

	// Holds the title of the window
    char* title; 

    std::vector<int> m_structures;

    Fl_Choice* m_menus[3];
    Fl_Menu_Item* m_menuItems;
    int m_menuItemsSize;

    GLWindow* m_glWindow;
    Fl_Offscreen m_offscreenImage[2];
    uchar* m_imageData[2];

    bool m_redrawStructures;
    int numPairs[7];
    int folderIndex;

};

#endif //DIAGRAMWINDOW_H
