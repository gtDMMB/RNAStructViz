/* RadialLayoutImage.cpp : Implementation of the .h display functions and window classes;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#include "RadialLayoutImage.h"

RadialLayoutDisplayWindow::RadialLayoutDisplayWindow(size_t width, size_t height) : 
	Fl_Cairo_Window(width, height), RadialLayoutWindowCallbackInterface(), 
	winTitle(NULL), vrnaPlotType(PLOT_TYPE_SIMPLE), radialLayoutCanvas(NULL), 
	closeWindowFrameBox(NULL), closeWindowBtn(NULL), exportImageToPNGBtn(NULL), 
	cbPlotType(NULL) {

     int offsetY = 10, offsetX = w() - 3 * WIDGET_WIDTH;
     
     closeWindowFrameBox = new Fl_Box(offsetX, offsetY, 3 * WIDGET_WIDTH, 
		                      3 * WIDGET_HEIGHT + 2 * WIDGET_SPACING);
     closeWindowFrameBox->box(FL_RSHADOW_BOX);
     closeWindowFrameBox->color(GUI_BGCOLOR);
     offsetY += WIDGET_HEIGHT / 2;

     closeWindowBtn = new Fl_Button(offsetX + WIDGET_WIDTH / 2, offsetY, 
		                    WIDGET_WIDTH, WIDGET_HEIGHT, 
				    "@1+  Close Window");
     closeWindowBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
     closeWindowBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     closeWindowBtn->callback(CloseWindowCallback);

     exportImageToPNGBtn = new Fl_Button(offsetX + 1.5 * WIDGET_WIDTH, offsetY, 
		                         WIDGET_WIDTH, WIDGET_HEIGHT, 
					 "@saveas  Export to PNG");
     exportImageToPNGBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
     exportImageToPNGBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     exportImageToPNGBtn->callback(ExportRadialImageToPNGCallback);
     offsetY += WIDGET_HEIGHT + WIDGET_SPACING;

     cbPlotType = new Fl_Check_Button(offsetX + WIDGET_WIDTH / 2, offsetY, 
		                      WIDGET_WIDTH, WIDGET_HEIGHT, 
				      "Simple Radial Plot");
     cbPlotType->callback(PlotTypeCheckboxCallback);
     cbPlotType->type(FL_TOGGLE_BUTTON);
     cbPlotType->labelcolor(GUI_TEXT_COLOR);
     cbPlotType->selection_color(GUI_BTEXT_COLOR);
     cbPlotType->value(1);

     set_modal();
     color(GUI_WINDOW_BGCOLOR);
     fl_rectf(0, 0, w(), h());
     box(FL_NO_BOX);
     set_draw_cb(Draw);

}

RadialLayoutDisplayWindow::~RadialLayoutDisplayWindow() {
     Free(winTitle);
     Delete(radialLayoutCanvas);
     Delete(closeWindowFrameBox);
     Delete(closeWindowBtn);
     Delete(exportImageToPNGBtn);
     Delete(cbPlotType);
}

bool RadialLayoutDisplayWindow::SetTitle(const char *windowTitleStr) {
     if(windowTitleStr == NULL) {
           return false;
     }
     Free(winTitle);
     size_t titleLen = strlen(windowTitleStr);
     winTitle = (char *) malloc((titleLen + 1) * sizeof(char));
     strncpy(winTitle, windowTitleStr, titleLen);
     winTitle[titleLen] = '\0';
     label(winTitle);
     return true;
}

bool RadialLayoutDisplayWindow::SetTitleFormat(const char *windowTitleFmt, ...) {
     if(windowTitleFmt == NULL) {
          return false;
     }
     char nextTitleStr[MAX_BUFFER_SIZE + 1];
     va_list argPtr;
     va_start(argPtr, windowTitleFmt);
     vsnprintf(nextTitleStr, MAX_BUFFER_SIZE, windowTitleFmt, argPtr);
     va_end(argPtr);
     nextTitleStr[MAX_BUFFER_SIZE] = '\0';
     return SetTitle(nextTitleStr);
}

bool RadialLayoutDisplayWindow::SetRadialPlotType(VRNAPlotType_t plotType) {
     vrnaPlotType = plotType;
     return true;
}

bool RadialLayoutDisplayWindow::DisplayRadialDiagram(const char *rnaSeq, size_t startSeqPos, 
		                                     size_t endSeqPos) { 
     if(rnaSeq == NULL) {
          return false;
     }
     radialLayoutCanvas = GetVRNARadialLayoutData(rnaSeq, startSeqPos, endSeqPos, 
		                                  (VRNAPlotType_t) vrnaPlotType);
     if(radialLayoutCanvas == NULL) {
          return false;
     }
     redraw();
     return true;
}

void RadialLayoutDisplayWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr) {
     if(thisCairoWindow == NULL || cr == NULL) {
          return;
     }
     fl_color(GUI_WINDOW_BGCOLOR);
     fl_rectf(0, 0, thisCairoWindow->w(), thisCairoWindow->h());
     RadialLayoutDisplayWindow *thisWindow = (RadialLayoutDisplayWindow *) thisCairoWindow;
     thisWindow->radialLayoutCanvas->SaveSettings();
     cairo_surface_t *crSurface = cairo_get_target(thisWindow->radialLayoutCanvas->GetCairoContext());
     cairo_set_source_surface(cr, crSurface, 0, 0);
     cairo_rectangle(cr, 0, 0, thisWindow->w(), thisWindow->h());
     cairo_fill(cr);
     thisWindow->radialLayoutCanvas->RestoreSettings();
     if(thisWindow->closeWindowFrameBox != NULL) {
          thisWindow->closeWindowFrameBox->redraw();
          thisWindow->closeWindowBtn->redraw();
	  thisWindow->exportImageToPNGBtn->redraw();
	  thisWindow->cbPlotType->redraw();
     }
}

void RadialLayoutDisplayWindow::CloseWindowCallback(Fl_Widget *cbtn, void *udata) {
     RadialLayoutDisplayWindow *rlWin = (RadialLayoutDisplayWindow *) cbtn->parent();
     rlWin->DoRadialWindowClose();
}

void RadialLayoutDisplayWindow::ExportRadialImageToPNGCallback(Fl_Widget *ebtn, void *udata) {




}

void RadialLayoutDisplayWindow::PlotTypeCheckboxCallback(Fl_Widget *cbw, void *udata) {




}

CairoContext_t * RadialLayoutDisplayWindow::GetVRNARadialLayoutData(const char *rnaSubseq, 
		                            size_t startPos, size_t endPos, 
					    VRNAPlotType_t plotType) {
     
     char *effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, 0, MAX_SIZET);
     if(effectiveRNASubseq == NULL) {
          return NULL;
     }
     StringToUppercase(effectiveRNASubseq);
     startPos = 0; endPos = strlen(effectiveRNASubseq);
     endPos = endPos ? endPos - 1 : 0;
     unsigned int rnaSubseqLen = endPos - startPos + 1;
     vrna_seq_toRNA(effectiveRNASubseq);

     vrna_fold_compound_t *vfc = vrna_fold_compound(effectiveRNASubseq, NULL, VRNA_OPTION_DEFAULT);
     vrna_md_t vmdParam = vfc->params->model_details;
     int length = vfc->length;
     
     char *mfeStructure = (char *) vrna_alloc((length + 1) * sizeof(char));
     double minEnergy = (double) vrna_mfe(vfc, mfeStructure);
     short *pairTable = vrna_ptable(mfeStructure); 
     short *pairTableG = vrna_ptable(mfeStructure);

     int ge = 0, ee, gb, Lg, l[3], workingIdx;
     while( (ee = parse_gquad(mfeStructure + ge, &Lg, l)) > 0) {
          ge += ee;
	  gb = ge - 4 * Lg - l[0] - l[1] - l[2] + 1;
	  for(int pbpIdx = 0; pbpIdx < Lg; pbpIdx++) {
               pairTableG[ge - pbpIdx] = gb + pbpIdx;
	       pairTableG[gb + pbpIdx] = ge - pbpIdx;
	  }
     }

     float *xPosArr = (float *) vrna_alloc((rnaSubseqLen + 1) * sizeof(float));
     float *yPosArr = (float *) vrna_alloc((rnaSubseqLen + 1) * sizeof(float));
     if(plotType == PLOT_TYPE_SIMPLE) { 
          workingIdx = simple_xy_coordinates(pairTableG, xPosArr, yPosArr);
     }
     else if(plotType == PLOT_TYPE_CIRCULAR) {
          int radius = 3 * rnaSubseqLen;
	  workingIdx = simple_circplot_coordinates(pairTableG, xPosArr, yPosArr);
	  for(int idx = 0; idx < rnaSubseqLen; idx++) {
               xPosArr[idx] *= radius;
	       xPosArr[idx] += radius;
	       yPosArr[idx] *= radius;
	       yPosArr[idx] += radius;
	  }
     }
     else {
          workingIdx = naview_xy_coordinates(pairTableG, xPosArr, yPosArr);
     }
     if(workingIdx != rnaSubseqLen) {
          fl_alert("Warning: Strange things are happening with the ViennaRNA PS plot algorithm ...\n"
	           " > workingIdx = %d, RNASubseqLen = %d;", 
		   workingIdx, rnaSubseqLen);
     }

     float xmin, xmax, ymin, ymax;
     xmin = xmax = xPosArr[0];
     ymin = ymax = yPosArr[0];
     for(int idx = 1; idx < rnaSubseqLen; idx++) {
          xmin = xPosArr[idx] < xmin ? xPosArr[idx] : xmin;
          xmax = xPosArr[idx] > xmax ? xPosArr[idx] : xmax;
          ymin = yPosArr[idx] < ymin ? yPosArr[idx] : ymin;
          ymax = yPosArr[idx] > ymax ? yPosArr[idx] : ymax;
     }
     float xScale = (float) (0.8 *(DEFAULT_RLWIN_WIDTH / xmax));
     float yScale = (float) (0.8 *(DEFAULT_RLWIN_HEIGHT / ymax));
     
     CairoContext_t *plotCanvas = new CairoContext_t(DEFAULT_RLWIN_WIDTH, DEFAULT_RLWIN_HEIGHT);
     plotCanvas->SetColor(CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK));

     // TODO: Draw connecting lines;
     int idx = 0;
     while(idx < rnaSubseqLen) {
          plotCanvas->DrawBaseNode(xPosArr[idx] * xScale, yPosArr[idx] * yScale, 
			           effectiveRNASubseq[idx], idx + 1, 8, 
				   CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK));
	  idx++;
     }

     free(effectiveRNASubseq);
     free(vfc);
     free(mfeStructure);
     free(pairTable);
     free(pairTableG);
     free(xPosArr);
     free(yPosArr);
     
     //plotCanvas->SaveToImage("/home/maxie/Desktop/testRadial.png");
     return plotCanvas;

}


