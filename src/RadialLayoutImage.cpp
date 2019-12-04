/* RadialLayoutImage.cpp : Implementation of the .h display functions and window classes;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <FL/fl_draw.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "RadialLayoutImage.h"
#include "RNAStructure.h"
#include "ThemesConfig.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

RadialLayoutDisplayWindow::RadialLayoutDisplayWindow(size_t width, size_t height) : 
    Fl_Cairo_Window(width, height), RadialLayoutWindowCallbackInterface(), 
    winTitle(NULL), vrnaPlotType(PLOT_TYPE_SIMPLE), 
    radialLayoutCanvas(NULL), radialLayoutCanvasOrig(NULL), 
    haveInitVRNAScroller(false), defaultScrollToX(-1), defaultScrollToY(-1),
    scrollerFillBox(NULL), windowScroller(NULL), 
    scalePlusBtn(NULL), scaleMinusBtn(NULL), resetBtn(NULL), 
    cairoWinTranslateX(0), cairoWinTranslateY(0), buttonToolbarHeight(0), 
    winScaleX(1.0), winScaleY(1.0), 
    imageHeight(0), imageWidth(0), 
    displayBaseLower(0), displayBaseHigher(0) {

     int offsetY = 10;
     int offsetX = (w() - 5 * RADIAL_BUTTON_WIDTH) / 2 + RADIAL_WIDGET_WIDTH / 2;
    
     scaleMinusBtn = new Fl_Button(offsetX, offsetY, RADIAL_BUTTON_WIDTH, RADIAL_WIDGET_HEIGHT, 
                           "@<<   Zoom Out");
     scaleMinusBtn->color(Darker(GUI_BGCOLOR, 0.5f));
     scaleMinusBtn->labelcolor(GUI_BTEXT_COLOR);
     scaleMinusBtn->labelfont(FL_HELVETICA);
     scaleMinusBtn->callback(ScaleRadialLayoutMinusCallback);
     offsetX += RADIAL_BUTTON_WIDTH + RADIAL_WIDGET_WIDTH / 2;
 
     scalePlusBtn = new Fl_Button(offsetX, offsetY, RADIAL_BUTTON_WIDTH, RADIAL_WIDGET_HEIGHT, 
                          "Zoom In   @>>");
     scalePlusBtn->color(Darker(GUI_BGCOLOR, 0.5f));
     scalePlusBtn->labelcolor(GUI_BTEXT_COLOR);
     scalePlusBtn->labelfont(FL_HELVETICA);
     scalePlusBtn->callback(ScaleRadialLayoutPlusCallback);
     offsetX += RADIAL_BUTTON_WIDTH + RADIAL_WIDGET_WIDTH / 2;

     resetBtn = new Fl_Button(offsetX, offsetY, RADIAL_BUTTON_WIDTH, RADIAL_WIDGET_HEIGHT, 
                          "@redo   Reset");
     resetBtn->color(Darker(GUI_BGCOLOR, 0.5f));
     resetBtn->labelcolor(GUI_BTEXT_COLOR);
     resetBtn->labelfont(FL_HELVETICA);
     resetBtn->callback(RadialLayoutResetCallback);
     offsetX += RADIAL_BUTTON_WIDTH + RADIAL_WIDGET_WIDTH / 2;

     exportToPNGBtn = new Fl_Button(offsetX, offsetY, RADIAL_BUTTON_WIDTH, RADIAL_WIDGET_HEIGHT, 
                            "@filesaveas  Export to PNG");
     exportToPNGBtn->color(Darker(GUI_BGCOLOR, 0.5f));
     exportToPNGBtn->labelcolor(GUI_BTEXT_COLOR);
     exportToPNGBtn->labelfont(FL_HELVETICA);
     exportToPNGBtn->callback(SaveRadialLayoutToPNGCallback);

     offsetY += 10 + RADIAL_WIDGET_HEIGHT;
     buttonToolbarHeight = offsetY; 

     windowScroller = new Fl_Scroll(0, offsetY, width, height - offsetY);
     windowScroller->color(GUI_WINDOW_BGCOLOR);
     windowScroller->scrollbar_size(SCROLL_SIZE);
     windowScroller->callback(HandleWindowScrollCallback);
     windowScroller->type(Fl_Scroll::BOTH_ALWAYS);
     windowScroller->box(FL_BORDER_BOX);

     color(GUI_WINDOW_BGCOLOR);
     set_draw_cb(Draw);
     //Fl::cairo_make_current(this);

}

RadialLayoutDisplayWindow::~RadialLayoutDisplayWindow() {
     Free(winTitle);
     Delete(radialLayoutCanvas, CairoContext_t);
     Delete(scalePlusBtn, Fl_Button);
     Delete(scaleMinusBtn, Fl_Button);
     Delete(exportToPNGBtn, Fl_Button);
     Delete(windowScroller, Fl_Scroll);
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

void RadialLayoutDisplayWindow::SetStructureCTFileName(std::string fileName) {
     ctFileName = fileName;
}

void RadialLayoutDisplayWindow::SetStructureFolderName(std::string folderName) {
     structFolderName = folderName;
}

void RadialLayoutDisplayWindow::SetStructureBases(int baseLower, int baseHigher, std::string baseStr) {
     displayBaseLower = baseLower;
     displayBaseHigher = baseHigher;
     displayBaseStr = baseStr;
}

bool RadialLayoutDisplayWindow::SetRadialPlotType(VRNAPlotType_t plotType) {
     vrnaPlotType = plotType;
     return true;
}

bool RadialLayoutDisplayWindow::DisplayRadialDiagram(const char *rnaSeq, size_t startSeqPos, 
                                             size_t endSeqPos, size_t seqLength) { 
     if(rnaSeq == NULL) {
          return false;
     }
     Delete(radialLayoutCanvas, CairoContext_t);
     Delete(radialLayoutCanvasOrig, CairoContext_t);
     
     radialLayoutCanvas = GetVRNARadialLayoutData(rnaSeq, startSeqPos, endSeqPos, seqLength, 
                                               (VRNAPlotType_t) vrnaPlotType);
     radialLayoutCanvasOrig = new CairoContext_t(*radialLayoutCanvas);
     ResizeScrollerFillBox();
     return true;
}

void RadialLayoutDisplayWindow::ResizeScrollerFillBox() {
     if(radialLayoutCanvas == NULL || windowScroller == NULL) {
          return;
     }
     if(scrollerFillBox != NULL) {
          windowScroller->remove(scrollerFillBox);
          Delete(scrollerFillBox, Fl_Box);
     }
     int nextFillerWidth = MAX(DEFAULT_RLWIN_WIDTH, radialLayoutCanvas->GetWidth());
     int nextFillerHeight = MAX(DEFAULT_RLWIN_HEIGHT - buttonToolbarHeight, radialLayoutCanvas->GetHeight());
     windowScroller->begin();
     scrollerFillBox = new Fl_Box(0, 0, nextFillerWidth, nextFillerHeight); 
     scrollerFillBox->type(FL_NO_BOX);
     windowScroller->end();
     windowScroller->redraw();
     redraw();
}

void RadialLayoutDisplayWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr) {
     if(thisCairoWindow == NULL || cr == NULL || 
        ((RadialLayoutDisplayWindow *) thisCairoWindow)->radialLayoutCanvas == NULL) {
          return;
     }
     RadialLayoutDisplayWindow *thisWindow = (RadialLayoutDisplayWindow *) thisCairoWindow; 
     fl_color(GUI_WINDOW_BGCOLOR);
     //fl_rectf(0, thisWindow->buttonToolbarHeight, 
     //     thisCairoWindow->w() - SCROLL_SIZE - 2, 
     //     thisCairoWindow->h() - thisWindow->buttonToolbarHeight - SCROLL_SIZE - 2);
     fl_color(FL_BLACK);
     fl_line_style(FL_SOLID | FL_CAP_ROUND | FL_JOIN_BEVEL);
     fl_xyline(0, thisWindow->buttonToolbarHeight - 2, thisCairoWindow->w());
     fl_line_style(0);
     fl_color(GUI_WINDOW_BGCOLOR);
     if(!thisWindow->haveInitVRNAScroller) {
          thisWindow->windowScroller->scroll_to(thisWindow->defaultScrollToX, thisWindow->defaultScrollToY);
          thisWindow->haveInitVRNAScroller = true;
     }
     thisWindow->cairoWinTranslateX = thisWindow->windowScroller->xposition();
     thisWindow->cairoWinTranslateY = thisWindow->windowScroller->yposition();
     
     cairo_surface_t *crSurface = cairo_get_target(thisWindow->radialLayoutCanvas->GetCairoContext());
     cairo_set_source_surface(cr, crSurface, -thisWindow->cairoWinTranslateX, -thisWindow->cairoWinTranslateY);
     cairo_rectangle(cr, 0, thisWindow->buttonToolbarHeight, 
             thisWindow->w() - SCROLL_SIZE - 2, 
             thisWindow->h() - thisWindow->buttonToolbarHeight - SCROLL_SIZE - 2);
     cairo_clip(cr);
     cairo_paint(cr);
     cairo_reset_clip(cr);
     if(thisWindow->windowScroller != NULL) {
          thisWindow->scalePlusBtn->redraw();
      thisWindow->scaleMinusBtn->redraw();
      thisWindow->windowScroller->redraw();
     }
}

std::string RadialLayoutDisplayWindow::GetExportToPNGOutputPath() {
    const char *chooserMsg = "Choose a file name for your PNG output image ...";
    const char *fileExtMask = "*.png";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[MAX_BUFFER_SIZE];
    defaultFilePath[0] = '\0';
    strcat(defaultFilePath, (char *) PNG_OUTPUT_DIRECTORY);
    strcat(defaultFilePath, defaultFilePath[strlen(defaultFilePath) - 1] == '/' ? "" : "/");
    strftime(defaultFilePath + strlen(defaultFilePath), MAX_BUFFER_SIZE - 1, (char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH, 
             tmCurrentTime);
    Fl_Native_File_Chooser fileChooser;
    fileChooser.title(chooserMsg);
    fileChooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fileChooser.options(Fl_Native_File_Chooser::NEW_FOLDER | 
                Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    fileChooser.preset_file(defaultFilePath);
    switch(fileChooser.show()) {
        case -1: // ERROR
             fl_alert("Error selecting file path to save PNG image: \"%s\".\n"
		      "If you are receiving a permissions error trying to save the "
		      "image into the directory you have chosen, try again by saving "
		      "the PNG image into a path in your user home directory.", 
		      fileChooser.errmsg());
         return string("");
    case 1: // CANCEL
         return string("");
    default:
         const char *outFileFullPath = fileChooser.filename();
         const char *dirMarkerPos = strrchr(outFileFullPath, '/');
         if(dirMarkerPos != NULL) {
	      unsigned int charsToCopy = (unsigned int) (dirMarkerPos - outFileFullPath + 1);
              strncpy((char *) PNG_OUTPUT_DIRECTORY, outFileFullPath, charsToCopy);
	      PNG_OUTPUT_DIRECTORY[charsToCopy] = '\0';
	      ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
	 }
	 return std::string(outFileFullPath);
    }
}

void RadialLayoutDisplayWindow::SaveRadialLayoutToPNGCallback(Fl_Widget *exportBtn, void *udata) {
     
     RadialLayoutDisplayWindow *rlDisplayWin = (RadialLayoutDisplayWindow *) exportBtn->parent();
     if(rlDisplayWin == NULL || rlDisplayWin->radialLayoutCanvas == NULL) {
          return;
     }
     std::string pngOutputPath = rlDisplayWin->GetExportToPNGOutputPath();
     if(pngOutputPath.length() == 0) {
          return;
     }
     
     // initialize the image data:
     cairo_surface_t *pngInitSrc = cairo_get_target(rlDisplayWin->radialLayoutCanvas->GetCairoContext());
     cairo_surface_t *pngSrc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                       rlDisplayWin->imageWidth, 
                   rlDisplayWin->imageHeight + PNG_FOOTER_HEIGHT);
     cairo_t *crImageOutput = cairo_create(pngSrc);
     CairoColor_t::FromNamedConstant(CairoColorSpec_t::CR_SOLID_WHITE).ApplyRGBAColor(crImageOutput);
     cairo_rectangle(crImageOutput, 0, 0, 
             rlDisplayWin->imageWidth, 
                 rlDisplayWin->imageHeight + PNG_FOOTER_HEIGHT);
     cairo_fill(crImageOutput);
     
     // draw the footer data (metadata):
     CairoColor_t::FromNamedConstant(CairoColorSpec_t::CR_SOLID_WHITE).ToOpaque().ApplyRGBAColor(crImageOutput);
     cairo_rectangle(crImageOutput, 0, 
             rlDisplayWin->imageHeight, 
                 rlDisplayWin->imageWidth, 
             PNG_FOOTER_HEIGHT);
     cairo_fill(crImageOutput);
     CairoColor_t::FromNamedConstant(CairoColorSpec_t::CR_SOLID_BLACK).ToOpaque().ApplyRGBAColor(crImageOutput);
     cairo_set_line_width(crImageOutput, 2);
     cairo_move_to(crImageOutput, 0, rlDisplayWin->imageHeight);
     cairo_line_to(crImageOutput, rlDisplayWin->imageWidth, 
           rlDisplayWin->imageHeight);
     cairo_stroke(crImageOutput);
     cairo_select_font_face(crImageOutput, "Courier New",
                            CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
     cairo_set_font_size(crImageOutput, 12);
     int offsetY = rlDisplayWin->imageHeight + 25;
     const char *footerLabels[] = {
      " * Structure Folder Name: ",
          " * Structure File Name:   ", 
      " * Structure Bases Shown: ", 
      " * Image Generated At:    ",
     };
     const char *footerData[] = {
      rlDisplayWin->structFolderName.c_str(), 
      rlDisplayWin->ctFileName.c_str(), 
      "[%d, %d]", 
      "%c %Z",
     };
     for(int fd = 0; fd < GetArrayLength(footerLabels); fd++) {
          char curStructLabel[MAX_BUFFER_SIZE];
      snprintf(curStructLabel, MAX_BUFFER_SIZE - 1, 
               "%s%s", footerLabels[fd], footerData[fd]);
      if(fd == 2) {
               char curStructLabel2[MAX_BUFFER_SIZE];
           snprintf(curStructLabel2, MAX_BUFFER_SIZE - 1, curStructLabel, 
            rlDisplayWin->displayBaseLower, rlDisplayWin->displayBaseHigher);
           strcpy(curStructLabel, curStructLabel2);
      }
      else if(fd == 3) {
               char curStructLabel2[MAX_BUFFER_SIZE];
           time_t curTime = time(NULL);
           struct tm *tmCurTime = localtime(&curTime);
           strftime(curStructLabel2, MAX_BUFFER_SIZE - 1, curStructLabel, tmCurTime);
           strcpy(curStructLabel, curStructLabel2);
      }
           cairo_move_to(crImageOutput, 12, offsetY);
      cairo_show_text(crImageOutput, curStructLabel);
      offsetY += 22;
     }
     
     // draw the source diagram onto the PNG output image:
     cairo_set_source_surface(crImageOutput, pngInitSrc, 0, 0);
     cairo_rectangle(crImageOutput, 0, 0, 
             rlDisplayWin->imageWidth, 
                 rlDisplayWin->imageHeight); 
     cairo_fill(crImageOutput);
     if(cairo_surface_write_to_png(pngSrc, pngOutputPath.c_str()) != CAIRO_STATUS_SUCCESS) {
          fl_alert("ERROR WRITING PNG TO FILE (\"%s\"): %s\n", pngOutputPath.c_str(), strerror(errno));
     }
     cairo_destroy(crImageOutput);
     cairo_surface_destroy(pngSrc);
     rlDisplayWin->redraw();
}

void RadialLayoutDisplayWindow::ScaleRadialLayoutPlusCallback(Fl_Widget *scaleBtn, void *udata) {
     RadialLayoutDisplayWindow *rwin = (RadialLayoutDisplayWindow *) scaleBtn->parent();
     float scalingFactor = 1.0 + DEFAULT_SCALING_PERCENT;
     rwin->radialLayoutCanvas->Scale(scalingFactor);
     rwin->ResizeScrollerFillBox();
     rwin->redraw();
}

void RadialLayoutDisplayWindow::ScaleRadialLayoutMinusCallback(Fl_Widget *scaleBtn, void *udata) {
     RadialLayoutDisplayWindow *rwin = (RadialLayoutDisplayWindow *) scaleBtn->parent();
     float scalingFactor = 1.0 - DEFAULT_SCALING_PERCENT;
     rwin->radialLayoutCanvas->Scale(scalingFactor);
     rwin->ResizeScrollerFillBox();
     rwin->redraw();
}

void RadialLayoutDisplayWindow::RadialLayoutResetCallback(Fl_Widget *resetBtn, void *udata) {
     RadialLayoutDisplayWindow *rwin = (RadialLayoutDisplayWindow *) resetBtn->parent();
     CairoContext_t *oldRadialLayoutCanvas = rwin->radialLayoutCanvas;
     rwin->radialLayoutCanvas = new CairoContext_t(*(rwin->radialLayoutCanvasOrig));
     Delete(oldRadialLayoutCanvas, CairoContext_t);
     rwin->haveInitVRNAScroller = false;
     rwin->ResizeScrollerFillBox();
     rwin->redraw();
}

void RadialLayoutDisplayWindow::CloseWindowCallback(Fl_Widget *cbtn, void *udata) {
     RadialLayoutDisplayWindow *rlWin = (RadialLayoutDisplayWindow *) cbtn->parent();
     rlWin->DoRadialWindowClose();
}

void RadialLayoutDisplayWindow::HandleWindowScrollCallback(Fl_Widget *scrw, void *udata) {
     if(scrw == NULL) {
          return;
     }
     RadialLayoutDisplayWindow *mainWindow = (RadialLayoutDisplayWindow *) scrw->parent();
     Fl_Scroll *windowScroller = (Fl_Scroll *) scrw;
     int scrollXPos = windowScroller->xposition();
     int scrollYPos = windowScroller->yposition();
     mainWindow->cairoWinTranslateX = scrollXPos; 
     mainWindow->cairoWinTranslateY = scrollYPos; 
     mainWindow->redraw();
}

CairoContext_t * RadialLayoutDisplayWindow::GetVRNARadialLayoutData(const char *rnaSubseq, 
                                    size_t startPos, size_t endPos, size_t seqLength, 
                        VRNAPlotType_t plotType) {
     
     char *effectiveRNASubseq = NULL;
     int seqIndexOffset = 0;
     if(seqLength <= MAX_SEQUENCE_DISPLAY_LENGTH) {
          effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, 0, MAX_SIZET);
     }
     else {
          int seqStartIdx = MAX(0, startPos - 25);
      int seqEndIdx = MIN(endPos + 25, seqLength);
      seqIndexOffset = seqStartIdx;
      effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, seqStartIdx, seqEndIdx);
     }
     if(effectiveRNASubseq == NULL) {
          return NULL;
     }
     StringToUppercase(effectiveRNASubseq);
     SetStructureBases(startPos, endPos, effectiveRNASubseq);
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
          int radius = 2 * rnaSubseqLen;
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
     double xmin, xmax, ymin, ymax, dmin;
     xmin = xmax = xPosArr[0];
     ymin = ymax = yPosArr[0];
     dmin = (double) INT_MAX;
     const int nodeSize = 38;
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
           xPosArr[xyPos] += ABS(xmin) + nodeSize;
      }
      else {
               xPosArr[xyPos] -= ABS(xmin);
      }
      if(ymin < 0) {
           yPosArr[xyPos] += ABS(ymin) + nodeSize;
      }
      else {
           yPosArr[xyPos] -= ABS(ymin);
      }
     }
     if(xmin < 0) {
      xmax += ABS(xmin) + nodeSize;
          xmin = nodeSize;
     }
     if(ymin < 0) {
          ymax += ABS(ymin) + nodeSize;
      ymin = nodeSize;
     }
     float winScale = 1.25 * nodeSize / dmin / M_SQRT2;
     float xScale = (float) (winScale * MAX(xmax / DEFAULT_RLWIN_WIDTH, 1.0));
     float yScale = (float) (winScale * MAX(ymax / DEFAULT_RLWIN_HEIGHT, 1.0));
     this->winScaleX = xScale;
     this->winScaleY = yScale;

     CairoContext_t *plotCanvas = new CairoContext_t(MAX(DEFAULT_RLWIN_WIDTH, (xmax - xmin) * xScale + 
                                             1.5 * nodeSize), 
                                             MAX(DEFAULT_RLWIN_HEIGHT, (ymax - ymin) * yScale + 
                             1.5 * nodeSize));
     imageHeight = ABS(ymax - ymin) * xScale + 1.5 * nodeSize;
     imageWidth = MAX(PNGOUT_MIN_IMAGE_WIDTH, ABS(xmax - xmin) * yScale + 1.5 * nodeSize);
     plotCanvas->BlankFillCanvas(CairoColor_t::FromFLColorType(GUI_WINDOW_BGCOLOR));
     plotCanvas->SetColor(CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_TRANSPARENT));
     plotCanvas->Translate(2 * nodeSize, 2 * nodeSize);
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
      if(idx + seqIndexOffset < startPos || idx + seqIndexOffset > endPos) {
           baseNodeColor = baseNodeColor.ToGrayscale();
      }
      else {
               if(defaultScrollToX == -1 || nodeX < defaultScrollToX) {
                defaultScrollToX = nodeX;
           }
           if(defaultScrollToY == -1 || nodeY < defaultScrollToY) {
            defaultScrollToY = nodeY;
           }
      }
      char nodeLabel[32];
      if(idx % NUMBERING_MODULO == NUMBERING_MODULO - 1) {
           snprintf(nodeLabel, 32, "%d\0", idx + seqIndexOffset + 1);
           plotCanvas->SetFontSize(1);
      }
      else {
           snprintf(nodeLabel, 32, "%c\0", effectiveRNASubseq[idx]);
      }
      plotCanvas->DrawBaseNode(nodeX, nodeY, nodeLabel, nodeSize, 
                     baseNodeColor);
      idx++;
     }
     if(defaultScrollToX + DEFAULT_RLWIN_WIDTH > imageWidth) {
          defaultScrollToX = MAX(0, imageWidth - DEFAULT_RLWIN_WIDTH);
     }
     if(defaultScrollToY + DEFAULT_RLWIN_HEIGHT > imageHeight) {
      defaultScrollToY = MAX(0, imageHeight - DEFAULT_RLWIN_HEIGHT);
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
