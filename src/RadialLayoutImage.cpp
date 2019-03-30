/* RadialLayoutImage.cpp : Implementation of the .h display functions and window classes;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#include <limits.h>

#include "RadialLayoutImage.h"
#include "RNAStructure.h"
#include "ThemesConfig.h"

RadialLayoutDisplayWindow::RadialLayoutDisplayWindow(size_t width, size_t height) : 
	Fl_Cairo_Window(width, height), RadialLayoutWindowCallbackInterface(), 
	winTitle(NULL), vrnaPlotType(PLOT_TYPE_SIMPLE), radialLayoutCanvas(NULL), 
	closeWindowFrameBox(NULL), scrollerFillBox(NULL), 
	closeWindowBtn(NULL), exportImageToPNGBtn(NULL), 
	cbPlotType(NULL), windowScroller(NULL), 
	cairoWinTranslateX(0), cairoWinTranslateY(0), 
        winScaleX(1.0), winScaleY(1.0) {

     windowScroller = new Fl_Scroll(0, 0, width, height);
     windowScroller->color(GUI_WINDOW_BGCOLOR);
     windowScroller->scrollbar_size(SCROLL_SIZE);
     windowScroller->callback(HandleWindowScrollCallback);
     windowScroller->type(Fl_Scroll::BOTH_ALWAYS);
     windowScroller->box(FL_BORDER_BOX);
     //windowScroller->begin();

     int offsetY = 10, offsetX = w() - 3 * WIDGET_WIDTH;

     /*closeWindowFrameBox = new Fl_Box(offsetX, offsetY, 3 * WIDGET_WIDTH, 
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
     offsetY += WIDGET_HEIGHT + 2 * WIDGET_SPACING;*/

     //scrollerFillBox = new Fl_Box(0, offsetY, 0, 0); //width - SCROLL_SIZE, height - SCROLL_SIZE);
     //scrollerFillBox->type(FL_NO_BOX);
     //windowScroller->end();
     //this->resizable(windowScroller);

     //set_modal();
     color(GUI_WINDOW_BGCOLOR);
     //fl_rectf(0, 0, w() - SCROLL_SIZE, h() - SCROLL_SIZE);
     //box(FL_NO_BOX);
     set_draw_cb(Draw);

}

RadialLayoutDisplayWindow::~RadialLayoutDisplayWindow() {
     Free(winTitle);
     Delete(radialLayoutCanvas);
     Delete(closeWindowFrameBox);
     Delete(closeWindowBtn);
     Delete(exportImageToPNGBtn);
     Delete(cbPlotType);
     Delete(windowScroller);
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
     int nextFillerWidth = radialLayoutCanvas->GetWidth();
     int nextFillerHeight = radialLayoutCanvas->GetHeight();
     windowScroller->begin();
     scrollerFillBox = new Fl_Box(0, 0, nextFillerWidth, nextFillerHeight); 
     scrollerFillBox->type(FL_NO_BOX);
     windowScroller->end();
     windowScroller->redraw();
     redraw();
     return true;
}

void RadialLayoutDisplayWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr) {
     if(thisCairoWindow == NULL || cr == NULL || 
        ((RadialLayoutDisplayWindow *) thisCairoWindow)->radialLayoutCanvas == NULL) {
          return;
     }
     fl_color(GUI_WINDOW_BGCOLOR);
     fl_rectf(0, 0, thisCairoWindow->w() - SCROLL_SIZE - 2, thisCairoWindow->h() - SCROLL_SIZE - 2);
     RadialLayoutDisplayWindow *thisWindow = (RadialLayoutDisplayWindow *) thisCairoWindow; 
     thisWindow->radialLayoutCanvas->SaveSettings();
     thisWindow->cairoWinTranslateX = thisWindow->windowScroller->xposition();
     thisWindow->cairoWinTranslateY = thisWindow->windowScroller->yposition();
     cairo_surface_t *crSurface = cairo_get_target(thisWindow->radialLayoutCanvas->GetCairoContext());
     cairo_set_source_surface(cr, crSurface, -thisWindow->cairoWinTranslateX, -thisWindow->cairoWinTranslateY);
     cairo_rectangle(cr, 0, 0, 
		     thisWindow->w() - SCROLL_SIZE - 2, thisWindow->h() - SCROLL_SIZE - 2);
     cairo_clip(cr);
     cairo_paint(cr);
     cairo_reset_clip(cr);
     if(thisWindow->closeWindowFrameBox != NULL) {
          thisWindow->closeWindowFrameBox->redraw();
          thisWindow->closeWindowBtn->redraw();
	  thisWindow->exportImageToPNGBtn->redraw();
	  thisWindow->cbPlotType->redraw();
	  thisWindow->windowScroller->redraw();
     }
     thisWindow->radialLayoutCanvas->RestoreSettings(); 
}

void RadialLayoutDisplayWindow::CloseWindowCallback(Fl_Widget *cbtn, void *udata) {
     RadialLayoutDisplayWindow *rlWin = (RadialLayoutDisplayWindow *) cbtn->parent();
     rlWin->DoRadialWindowClose();
}

void RadialLayoutDisplayWindow::ExportRadialImageToPNGCallback(Fl_Widget *ebtn, void *udata) {
     fl_alert("TODO");
}

void RadialLayoutDisplayWindow::PlotTypeCheckboxCallback(Fl_Widget *cbw, void *udata) {
     fl_alert("TODO");
}

void RadialLayoutDisplayWindow::HandleWindowScrollCallback(Fl_Widget *scrw, void *udata) {
     if(scrw == NULL) {
          return;
     }
     RadialLayoutDisplayWindow *mainWindow = (RadialLayoutDisplayWindow *) scrw->parent();
     Fl_Scroll *windowScroller = (Fl_Scroll *) scrw;
     int scrollXPos = windowScroller->xposition();
     int scrollYPos = windowScroller->yposition();
     mainWindow->cairoWinTranslateX = scrollXPos; //* mainWindow->winScaleX;
     mainWindow->cairoWinTranslateY = scrollYPos; //* mainWindow->winScaleY;
     mainWindow->redraw();
}

CairoContext_t * RadialLayoutDisplayWindow::GetVRNARadialLayoutData(const char *rnaSubseq, 
		                            size_t startPos, size_t endPos, 
					    VRNAPlotType_t plotType) {
     
     char *effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, 0, MAX_SIZET);
     if(effectiveRNASubseq == NULL) {
          return NULL;
     }
     StringToUppercase(effectiveRNASubseq);
     size_t startPos2 = 0; 
     size_t endPos2 = strlen(effectiveRNASubseq);
     endPos2 = endPos2 ? endPos2 - 1 : 0;
     unsigned int rnaSubseqLen = endPos2 - startPos2 + 1;
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

     float xmin, xmax, ymin, ymax, dmin;
     xmin = xmax = xPosArr[0];
     ymin = ymax = yPosArr[0];
     dmin = (float) INT_MAX;
     for(int idx = 1; idx < rnaSubseqLen; idx++) {
          xmin = xPosArr[idx] < xmin ? xPosArr[idx] : xmin;
          xmax = xPosArr[idx] > xmax ? xPosArr[idx] : xmax;
          ymin = yPosArr[idx] < ymin ? yPosArr[idx] : ymin;
          ymax = yPosArr[idx] > ymax ? yPosArr[idx] : ymax;
	  for(int j = idx + 1; j < rnaSubseqLen; j++) {
	       double edist = Square(xPosArr[idx] - xPosArr[j]) + Square(yPosArr[idx] - yPosArr[j]);
	       edist = sqrt(edist);
	       if(edist < dmin) {
	            dmin = edist;
	       }
	  }
     }
     for(int xyPos = 0; xyPos < rnaSubseqLen; xyPos++) {
          if(xmin < 0) {
	       xPosArr[xyPos] += ABS(xmin);
	  }
	  if(ymin < 0) {
	       yPosArr[xyPos] += ABS(ymin);
	  }
     }
     if(xmin < 0) {
	  xmax += ABS(xmin);
          xmin = 0.0;
     }
     if(ymin < 0) {
          ymax += ABS(ymin);
	  ymin = 0.0;
     }
     const int nodeSize = 29;
     float winScale = nodeSize / dmin / M_SQRT2;
     float xScale = (float) (0.85 * winScale * MAX(MAX(xmax / DEFAULT_RLWIN_WIDTH, DEFAULT_RLWIN_WIDTH / xmax), 1.0));
     float yScale = (float) (0.85 * winScale * MAX(MAX(ymax / DEFAULT_RLWIN_HEIGHT, DEFAULT_RLWIN_HEIGHT / ymax), 1.0));
     this->winScaleX = xScale;
     this->winScaleY = yScale;

     CairoContext_t *plotCanvas = new CairoContext_t(MAX(DEFAULT_RLWIN_WIDTH - SCROLL_SIZE, xmax - xmin) * xScale + 
		                                     nodeSize, 
		                                     MAX(DEFAULT_RLWIN_HEIGHT - SCROLL_SIZE, ymax - ymin) * yScale + 
						     nodeSize);
     plotCanvas->BlankFillCanvas(CairoColor_t::FromFLColorType(GUI_WINDOW_BGCOLOR));
     plotCanvas->SetColor(CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_TRANSPARENT));
     plotCanvas->Translate(nodeSize, nodeSize);
     plotCanvas->SetStrokeSize(2);
     plotCanvas->SetFontFace(CairoContext_t::CairoFontFace_t::MONOSPACE | 
		             CairoContext_t::CairoFontStyle_t::BOLD | 
			     CairoContext_t::CairoFontStyle_t::ITALIC);
     plotCanvas->SetFontSize(3);

     int idx = 0, nodeX = 0, nodeY = 0, lastNodeX, lastNodeY;
     while(idx < rnaSubseqLen) {
	  lastNodeX = nodeX; 
	  lastNodeY = nodeY;
          nodeX = (int) ((xPosArr[idx] - xmin) * xScale);
	  nodeY = (int) ((yPosArr[idx] - ymin) * yScale);
	  if(idx > 0) {
	       plotCanvas->SetColor(CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK).Lighten(0.5).ToOpaque());
	       plotCanvas->DrawLine(nodeX, nodeY, lastNodeX, lastNodeY);
	  }
          idx++;
     }
     idx = 0;
     while(idx < rnaSubseqLen) {
	  lastNodeX = nodeX; 
	  lastNodeY = nodeY;
          nodeX = (int) ((xPosArr[idx] - xmin) * xScale);
	  nodeY = (int) ((yPosArr[idx] - ymin) * yScale);
	  CairoColor_t baseNodeColor = GetBaseNodeColor(effectiveRNASubseq[idx]);
	  if(idx < startPos || idx > endPos) {
	       baseNodeColor = baseNodeColor.ToGrayscale();
	  } 
	  char nodeLabel[32];
	  if(idx % NUMBERING_MODULO == NUMBERING_MODULO - 1) {
	       snprintf(nodeLabel, 32, "%d\0", idx + 1);
	       plotCanvas->SetFontSize(2);
	  }
	  else {
	       snprintf(nodeLabel, 32, "%c\0", effectiveRNASubseq[idx]);
	       plotCanvas->SetFontSize(6);
	  }
	  plotCanvas->DrawBaseNode(nodeX, nodeY, nodeLabel, nodeSize, 
	  			   baseNodeColor);
	  idx++;
     }

     free(effectiveRNASubseq);
     free(vfc);
     free(mfeStructure);
     free(pairTable);
     free(pairTableG);
     free(xPosArr);
     free(yPosArr);
     
     return plotCanvas;

}

CairoColor_t RadialLayoutDisplayWindow::GetBaseNodeColor(char baseCh) {
     if(toupper(baseCh) == (int) 'A') {
          return CairoColor_t::FromFLColorType(FL_LOCAL_MEDIUM_GREEN);
     }
     else if(toupper(baseCh) == (int) 'C') {
	  return CairoColor_t::FromFLColorType(FL_LOCAL_DARK_RED);
     }
     else if(toupper(baseCh) == (int) 'G') {
	  return CairoColor_t::FromFLColorType(FL_LOCAL_LIGHT_PURPLE);
     }
     else if(toupper(baseCh) == (int) 'U') {
	  return CairoColor_t::FromFLColorType(FL_LOCAL_BRIGHT_YELLOW);
     }
     else {
          return CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK);
     }
}
