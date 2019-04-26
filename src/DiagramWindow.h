/*
    The window into which structures are drawn.
*/

#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <vector>
#include <algorithm>

using std::vector;
using std::min_element;
using std::max_element;

#include <cairo.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Cairo.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/x.H>

#include "ConfigOptions.h"
#include "RNAStructure.h"
#include "CairoDrawingUtils.h"
#include "BranchTypeIdentification.h"
#include "RadialLayoutImage.h"

#define IMAGE_DIM                    (485)
#define DIAGRAM_TO_IMAGE_RATIO       (0.85f)
#define IMAGE_WIDTH                  (IMAGE_DIM)
#define DIAGRAM_WIDTH                (DIAGRAM_TO_IMAGE_RATIO * IMAGE_WIDTH)
#define IMAGE_HEIGHT                 (IMAGE_DIM)
#define DIAGRAM_HEIGHT               (DIAGRAM_TO_IMAGE_RATIO * IMAGE_HEIGHT)
#define IMAGE_DEPTH                  (3)
#define STRAND_MARKER_IMAGE_HEIGHT   (25)
#define PNG_FOOTER_HEIGHT            (100)

#define GLWIN_TRANSLATEX             (50)
#define GLWIN_TRANSLATEY             (110)

#define WIDGET_SPACING               (35)
#define EXPORT_BUTTON_WIDTH          (115)
#define WINW_EXTENSION               (EXPORT_BUTTON_WIDTH + 3 * WIDGET_SPACING)

#define ZOOM_WIDTH                   (200)
#define ZOOM_HEIGHT                  (200)

#define DIAGRAMWIN_DEFAULT_CURSOR    (FL_CURSOR_CROSS)
#define DWIN_REDRAW_REFRESH          (1.75)
#define DWIN_DRAG_DX                 (3)

#define DWINARC_MAX_TICKS            (12)
#define DWINARC_LABEL_PCT            (0.0833)

class DiagramWindow : public Fl_Cairo_Window, public RadialLayoutWindowCallbackInterface {

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
    
    inline int GetFolderIndex() {
        return folderIndex;
    }
    
    void SetFolderIndex(int index);
    int GetFolderIndex() const;

    inline void RadialWindowCloseCallback(Fl_Widget *rlWin, void *udata) {
         if(radialDisplayWindow == NULL) {
	      return;
	 }
	 radialDisplayWindow->hide();
	 Delete(radialDisplayWindow);
    }   

protected:

    static void checkBoxChangedStateCallback(Fl_Widget*, void *v);
    static void exportToPNGButtonPressHandler(Fl_Widget*, void *v);
    
    /*
	Draws the contents of the window.
    */
    bool computeDrawKeyParams(RNAStructure **sequences, int *numToDraw, int *keyA, int *keyB);
    void drawWidgets(bool fillWin);
    static void Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr);

    void resize(int x, int y, int w, int h);

public:
    static const int ms_menu_minx[3];
    static const int ms_menu_width;

private:
    void RebuildMenus();

    void RedrawBuffer(cairo_t *cr, RNAStructure** structures, 
		      const int *structDrawParams, 
    	              const int resolution);

	/* Draws the color legend for the arcs. Input a and b correspond to the
	   index of the relevant structures*/
    void DrawKey3(); // if 3 structures are selected
    void DrawKey2(const int a, const int b); // if 2 selected structures
    void DrawKey1(const int a); // if 1 selected structure
    
    void SetCairoBranchColor(cairo_t *cr, const BranchID_t &branchType, 
		             int enabled, 
                             CairoColorSpec_t fallbackColorFlag);
    void SetCairoColor(cairo_t *cr, int colorFlag); 
    void SetCairoToFLColor(cairo_t *cr, Fl_Color flc);

    inline void CairoRectangle(cairo_t *cr, int x, int y, int w, int h) {
         double rectX = (double) x / this->w();
	 double rectY = (double) y / this->h();
	 double rectW = (double) w / this->w();
	 double rectH = (double) h / this->h();
	 cairo_rectangle(cr, rectX, rectY, rectW, rectH);
    }

    /* Draws the arcs for all the base pairs, colored according to their 
       corresponding structures */
    void Draw3(cairo_t *cr, RNAStructure** structures, const int resolution); // 3 structures
    void Draw2(cairo_t *cr, RNAStructure** structures, const int resolution); // 2 structures
    void Draw1(cairo_t *cr, RNAStructure** structures, const int resolution); // 1 structure
    
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
		cairo_t *cr, 
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

    static void MenuCallback(Fl_Widget *widget, void *userData);
    static void ChangeBaseColorPaletteCallback(Fl_Widget *btn, void *udata);

    // Holds the title of the window
    char* title; 

    std::vector<int> m_structures;

    Fl_Choice* m_menus[3];
    Fl_Check_Button *m_drawBranchesIndicator;
    Fl_Check_Button *m_cbShowTicks, *m_cbDrawBases;
    Fl_Button *exportButton;
    Fl_RGB_Image *baseColorPaletteImg;
    Fl_Button *baseColorPaletteImgBtn, *baseColorPaletteChangeBtn;
    Fl_Menu_Item* m_menuItems;
    int m_menuItemsSize;

    int imageStride;
    cairo_surface_t *crSurface, *crBasePairsSurface;
    cairo_t *crDraw, *crBasePairsOverlay;
    uchar *imageData;
    bool cairoTranslate;
    bool m_redrawStructures;
    bool showPlotTickMarks;
    
    int numPairs[7];
    int folderIndex;
    int structureFolderIndex, sequenceLength;
    int pixelWidth;
    bool userConflictAlerted;
    
    cairo_surface_t *crZoomSurface;
    cairo_t *crZoom;
    bool zoomButtonDown, haveZoomBuffer;
    bool zoomBufferContainsArc;
    int zx0, zy0, zx1, zy1, zw, zh;
    int zoomBufferMinArcIndex, zoomBufferMaxArcIndex;
    int initZoomX, initZoomY;
    int lastZoomX, lastZoomY;
    int handle(int flEvent);
    bool ParseZoomSelectionArcIndices();
    void RedrawCairoZoomBuffer(cairo_t *curWinContext);
    void HandleUserZoomAction();

    RadialLayoutDisplayWindow *radialDisplayWindow;

    void RedrawStrandEdgeMarker(cairo_t *curWinContext);
    void RedrawStructureTickMarks(cairo_t *curWinContext);
    
    static void ShowTickMarksCallback(Fl_Widget *cbw, void *udata);
    static void DrawBasesCallback(Fl_Widget *cbw, void *udata);

    void WarnUserDrawingConflict();
    std::string GetExportPNGFilePath();

    volatile static DiagramWindow *currentDiagramWindowInstance;
    static bool redrawRefreshTimerSet;
    static vector<string> errorMsgQueue;

public:
    void setAsCurrentDiagramWindow() const; 
    static void RedrawWidgetsTimerCallback(void *);
    void AddNewErrorMessageToDisplay(string errorMsg, float callbackTime = 0.5);
    static void DisplayErrorDialogTimerCallback(void *);

};

#endif //DIAGRAMWINDOW_H
