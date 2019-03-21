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

#define WIDGET_HEIGHT                 (15)
#define WIDGET_WIDTH                  (50)
#define WIDGET_SPACING                (5)

#define NUMBERING_MODULO              (10)

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
	  bool DisplayRadialDiagram(const char *rnaSeq, size_t startSeqPos = 0, 
			            size_t endSeqPos = MAX_SIZET);

	  inline void RadialWindowCloseCallback(Fl_Widget *rlWin, void *udata) {}

     protected:
	  static void Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr);

     private:
	  char *winTitle;
	  int vrnaPlotType;
          CairoContext_t *radialLayoutCanvas;
	  
	  Fl_Box *closeWindowFrameBox;
	  Fl_Button *closeWindowBtn, *exportImageToPNGBtn;
          Fl_Check_Button *cbPlotType;

	  static void CloseWindowCallback(Fl_Widget *cbtn, void *udata);
	  static void ExportRadialImageToPNGCallback(Fl_Widget *ebtn, void *udata);
          static void PlotTypeCheckboxCallback(Fl_Widget *cbPlotType, void *udata);

     public:
          static CairoContext_t * GetVRNARadialLayoutData(const char *rnaSubseq, 
			                                  size_t startPos = 0, 
		                                          size_t endPos = MAX_SIZET, 
							  VRNAPlotType_t plotType = PLOT_TYPE_SIMPLE);
          static CairoColor_t GetBaseNodeColor(char baseCh);

};

#endif
