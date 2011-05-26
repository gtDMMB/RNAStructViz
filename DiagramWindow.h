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
    DiagramWindow(int w, int h, const char *label, const std::vector<int>& structures);
    DiagramWindow(int x, int y, int w, int h, const char *label, const std::vector<int>& structures);

    // Virtual destructor
    virtual ~DiagramWindow();

    // Manage known structures
    void AddStructure(const int index);
    void RemoveStructure(const int index);
    void SetStructures(const std::vector<int>& structures);

protected:
    /*
	Draws the contents of the window.
    */
    void draw();

private:
    static const int ms_menu_minx[3];
    static const int ms_menu_width;

    void RebuildMenus();

    void RedrawBuffer(RNAStructure** structures, const int numStructures, const int resolution);

    void DrawKey3();
    void DrawKey2(const int a, const int b);
    void DrawKey1(const int a);

    void Draw3(RNAStructure** structures, const int resolution);
    void Draw2(RNAStructure** structures, const int resolution);
    void Draw1(RNAStructure** structures, const int resolution);

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

    std::vector<int> m_structures;

    Fl_Choice* m_menus[3];
    Fl_Menu_Item* m_menuItems;
    int m_menuItemsSize;

    GLWindow* m_glWindow;
    Fl_Offscreen m_offscreenImage[2];
    uchar* m_imageData[2];

    bool m_redrawStructures;
};

#endif
