/*
    The window into which structures are drawn.
*/

#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#ifndef FLTK_HAVE_CAIRO
     #define FLTK_HAVE_CAIRO
#endif

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/x.H>
#include <cairo.h>
#include <vector>

#include "GLWindow.h"
#include "RNAStructure.h"
#include "BranchTypeIdentification.h"

#define IMAGE_DIM          (640)
#define IMAGE_WIDTH        (IMAGE_DIM)
#define IMAGE_HEIGHT       (IMAGE_DIM)
#define IMAGE_DEPTH        (3)

#define GLWIN_TRANSLATEX   (15)
#define GLWIN_TRANSLATEY   (110)

#define GLWIN_REFRESH      (0.35)

typedef enum {
     CR_BLACK   = 0, 
     CR_RED     = 1, 
     CR_GREEN   = 2, 
     CR_BLUE    = 3, 
     CR_YELLOW  = 4, 
     CR_MAGENTA = 5, 
     CR_CYAN    = 6, 
     CR_BRANCH1 = 7, 
     CR_BRANCH2 = 8, 
     CR_BRANCH3 = 9, 
     CR_BRANCH4 = 10
} CairoColorSpec_t;

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
    
    void ResetWindow(bool resetMenus);
    
    inline int GetFolderIndex()
    {
        return folderIndex;
    }
    
    void SetFolderIndex(int index);
    
protected:

    static void checkBoxChangedStateCallback(Fl_Widget*, void *v);
    static void exportToPNGButtonPressHandler(Fl_Widget*, void *v);
    
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
    
    void CairoPrepareDisplay();
    void CairoBufferFinishingTouches();
    void CairoDrawBufferToScreen(); 
    void SetCairoBranchColor(const BranchID_t &branchType, int enabled, CairoColorSpec_t fallbackColorFlag);

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
    Fl_Check_Button *m_drawBranchesIndicator;
    Fl_Button *exportButton;
    Fl_Menu_Item* m_menuItems;
    int m_menuItemsSize;

    GLWindow* m_glWindow;
    Fl_Offscreen m_offscreenImage[2];
    uchar* m_imageData[2];
    int imageStride;
    cairo_surface_t *crSurface, *crMaskSurface;
    cairo_t *crDraw, *crDrawTemp;
    cairo_pattern_t *circleMask;
    bool holdSurfaceRef;

    bool m_redrawStructures;
    int numPairs[7];
    int folderIndex;
    int pixelWidth;
    bool userConflictAlerted;
    
    void WarnUserDrawingConflict();
    void CairoSetRGB(unsigned short R, unsigned short G, unsigned short B);
    char * GetExportPNGFilePath();

    inline static void cbUpdateGLWindow(void *windowPtr) {
         DiagramWindow *thisWindow = (DiagramWindow *) windowPtr;
         thisWindow->m_redrawStructures = true;
         thisWindow->redraw();
         //Fl::repeat_timeout(GLWIN_REFRESH, cbUpdateGLWindow, windowPtr);
         return;
         cairo_status(thisWindow->crDraw); // try to wake up the buffer
         cairo_pattern_t *localContextPattern = cairo_get_source(thisWindow->crDraw);
         cairo_pattern_get_surface(localContextPattern, &(thisWindow->crSurface));
         if(!thisWindow->holdSurfaceRef) { 
              cairo_surface_reference(thisWindow->crSurface);
              thisWindow->holdSurfaceRef = true;
         }
         cairo_surface_status(thisWindow->crSurface);
         thisWindow->m_redrawStructures = true;
         thisWindow->redraw();
         thisWindow->m_glWindow->position(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
         thisWindow->m_glWindow->flush();
    }
    
    inline void startRefreshTimer() {
         Fl::add_timeout(GLWIN_REFRESH, cbUpdateGLWindow, (void *) this);
    }

};

#endif //DIAGRAMWINDOW_H
