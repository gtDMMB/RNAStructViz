/* RadialLayoutImage.h : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#ifndef __RADIAL_LAYOUT_IMAGE_H__
#define __RADIAL_LAYOUT_IMAGE_H__

#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>

#include <cairo.h>

extern "C" {
     #include <ViennaRNA/utils/basic.h>
     #include <ViennaRNA/params/basic.h>
     #include <ViennaRNA/datastructures/basic.h>
     #include <ViennaRNA/utils/strings.h>
     #include <ViennaRNA/plotting/structures.h>
     #include <ViennaRNA/fold.h>
     #include <ViennaRNA/gquad.h>
}

#include "ConfigOptions.h"
#include "CairoDrawingUtils.h"

#define DEFAULT_RLWIN_WIDTH           (550)
#define DEFAULT_RLWIN_HEIGHT          (550)

#define RADIAL_WIDGET_HEIGHT          (35)
#define RADIAL_WIDGET_WIDTH           (50)
#define RADIAL_BUTTON_WIDTH           (115)
#define RADIAL_WIDGET_SPACING         (10)
#define SCROLL_SIZE                   (20)

#define NUMBERING_MODULO              (10)
#define DEFAULT_SCALING_PERCENT       (0.25)


class RadialLayoutWindowCallbackInterface {
     
     public:
	  inline RadialLayoutWindowCallbackInterface() : parentCallingWindow(NULL) {}
          inline RadialLayoutWindowCallbackInterface(RadialLayoutWindowCallbackInterface *parentWin) : 
		 parentCallingWindow(parentWin) {}

	  virtual void RadialWindowCloseCallback(Fl_Widget *rlWin, void *udata) = 0;
          
	  inline void SetParentWindow(RadialLayoutWindowCallbackInterface *callingWin) {
               parentCallingWindow = callingWin;
	  }

	  inline bool DoRadialWindowClose() {
               if(parentCallingWindow == NULL) {
	            return NULL;
	       }
	       parentCallingWindow->RadialWindowCloseCallback(NULL, NULL);
	       return true;
	  }
     
     protected:
	  RadialLayoutWindowCallbackInterface *parentCallingWindow;

};

class RadialLayoutDisplayWindow : public Fl_Cairo_Window, public RadialLayoutWindowCallbackInterface {

     public:
          static const int MAX_SEQUENCE_DISPLAY_LENGTH = 500;
	  
	  typedef enum {
                PLOT_TYPE_SIMPLE   = VRNA_PLOT_TYPE_SIMPLE, 
		PLOT_TYPE_CIRCULAR = VRNA_PLOT_TYPE_CIRCULAR, 
		PLOT_TYPE_NAVIEW
	  } VRNAPlotType_t;

          RadialLayoutDisplayWindow(size_t width = DEFAULT_RLWIN_WIDTH, 
			            size_t height = DEFAULT_RLWIN_HEIGHT);
	  ~RadialLayoutDisplayWindow();

	  bool SetTitle(const char *windowTitleStr);
	  bool SetTitleFormat(const char *windowTitleFmt, ...);
	  
	  bool SetRadialPlotType(VRNAPlotType_t plotType = PLOT_TYPE_CIRCULAR);
	  bool DisplayRadialDiagram(const char *rnaSeq, size_t startSeqPos, 
			            size_t endSeqPos, size_t seqLength);

	  inline void RadialWindowCloseCallback(Fl_Widget *rlWin, void *udata) {}

     protected:
	  void ResizeScrollerFillBox();
	  static void Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr);

     private:
	  char *winTitle;
	  int vrnaPlotType;
          CairoContext_t *radialLayoutCanvas, *radialLayoutCanvasOrig;
          bool haveInitVRNAScroller;

	  Fl_Box *scrollerFillBox;
	  Fl_Button *scalePlusBtn, *scaleMinusBtn, *resetBtn;
	  Fl_Scroll *windowScroller;
	  int buttonToolbarHeight;
	  int cairoWinTranslateX, cairoWinTranslateY;
	  int defaultScrollToX, defaultScrollToY;
	  double winScaleX, winScaleY;

	  static void ScaleRadialLayoutPlusCallback(Fl_Widget *scaleBtn, void *udata);
          static void ScaleRadialLayoutMinusCallback(Fl_Widget *scaleBtn, void *udata);
	  static void RadialLayoutResetCallback(Fl_Widget *resetBtn, void *udata);
	  static void CloseWindowCallback(Fl_Widget *cbtn, void *udata);
          static void HandleWindowScrollCallback(Fl_Widget *scrw, void *udata);

     public:
          CairoContext_t * GetVRNARadialLayoutData(const char *rnaSubseq, 
 		                                   size_t startPos, 
		                                   size_t endPos, 
					           size_t seqLength, 
						   VRNAPlotType_t plotType = PLOT_TYPE_SIMPLE);
          static CairoColor_t GetBaseNodeColor(char baseCh);

};

#endif
