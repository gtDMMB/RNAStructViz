#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Enumerations.H>
#include <FL/names.h>
#include <FL/Fl_Tooltip.H>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <numeric>

#include "DiagramWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "ThemesConfig.h"
#include "CairoDrawingUtils.h"
#include "DisplayConfigWindow.h"
#include "TerminalPrinting.h"

#include "pixmaps/FivePrimeThreePrimeStrandEdgesMarker.c"
#include "pixmaps/BaseColorPaletteButtonImage.c"

#include <FL/x.H>
#ifdef __APPLE__
     #include <cairo-quartz.h>
#else
     #include <cairo-xlib.h>
#endif

static const char *BASE_LINE_INCL_LABELS[3][7] = {
     { "> 1", "", "", "", "", "", "" }, 
     { "> 1 & 2", "> 1", "> 2", "", "", "", "" }, 
     { "> 1 & 2 & 3", "> 1", "> 2", "> 1 & 2", "> 1 & 3", "> 2 & 3", "> 3" },
};

const int DiagramWindow::ms_menu_minx[3] = { 
     2 * WIDGET_SPACING + 5, 
     2 * WIDGET_SPACING + 205, 
     2 * WIDGET_SPACING + 405 
};
const int DiagramWindow::ms_menu_width = 190;
vector<string> DiagramWindow::errorMsgQueue;
bool DiagramWindow::errorAlertDisplayShown = false;

DiagramWindow * DiagramWindow::currentDiagramWindowInstance = NULL;
bool DiagramWindow::redrawRefreshTimerSet = false;

void DiagramWindow::Construct(int w, int h, const std::vector<int> &structures) {

    xclass("RNAStructViz");

    pixelWidth = 1; 
    zoomButtonDown = false;
    haveZoomBuffer = false;
    zoomBufferContainsArc = false;
    zx0 = zy0 = zx1 = zy1 = zw = zh = 0;
    zoomBufferMinArcIndex = zoomBufferMaxArcIndex = 0;

    m_menus[0] = m_menus[1] = m_menus[2] = NULL;
    m_menuItems = 0;
    m_menuItemsSize = 0;
    folderIndex = -1;
    sequenceLength = 0;
    m_drawBranchesIndicator = NULL;
    m_cbShowTicks = NULL;
    m_cbDrawBases = NULL;
    userConflictAlerted = false;
    showPlotTickMarks = true;
    baseColorPaletteImg = NULL;
    baseColorPaletteImgBtn = NULL;
    baseColorPaletteChangeBtn = NULL;
    radialDisplayWindow = NULL;
    ctFileSelectWin = NULL;
    imgExportSelectWin = NULL;

    Fl::visual(FL_RGB);
    default_cursor(DIAGRAMWIN_DEFAULT_CURSOR);
    cursor(DIAGRAMWIN_DEFAULT_CURSOR);

    //colors the top of the Diagram window where structures are chosen
    color(GUI_WINDOW_BGCOLOR);
    size_range(w, h, w, h);
    box(FL_NO_BOX);
    iconize();
    make_current();

    title = (char *) malloc(128 * sizeof(char));
    SetStructures(structures);
 
    crSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                                           IMAGE_WIDTH, IMAGE_HEIGHT);
    crZoomSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                                               ZOOM_WIDTH, ZOOM_HEIGHT);
    crBasePairsSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_HEIGHT);
    crDraw = cairo_create(crSurface);
    crBasePairsOverlay = cairo_create(crBasePairsSurface);
    SetCairoColor(crDraw, CairoColorSpec_t::CR_TRANSPARENT);
    cairo_rectangle(crDraw, 0, 0, this->w(), this->h());
    cairo_fill(crDraw);
    SetCairoColor(crBasePairsOverlay, CairoColorSpec_t::CR_TRANSPARENT);
    cairo_rectangle(crBasePairsOverlay, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
    cairo_fill(crBasePairsOverlay);
    crZoom = cairo_create(crZoomSurface);
    cairo_set_source_rgb(crZoom, 
                         GetRed(GUI_WINDOW_BGCOLOR) / 255.0f, 
                         GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
                         GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
    cairo_rectangle(crZoom, 0, 0, ZOOM_WIDTH, ZOOM_HEIGHT);
    cairo_fill(crZoom);
    
    set_draw_cb(Draw);
    redraw_full(this);
    show();
    /* It can take some time from the time show() is called until the 
     * point when the user should see it on screen. This leads to buggy 
     * behavior on recent MacOS platforms (>= Big Sur) as seen in issue #105. 
     * The strategy is to wait until the user sees the window, call show() again, 
     * and then redraw everything within the window after that happens:
     */
    while(!shown() || !visible()) {
        Fl::wait(0.5);
    }
    show();
    redraw_full(this);

}

DiagramWindow::DiagramWindow(int w, int h, const char *label,
                             const std::vector<int> &structures) 
        : Fl_Cairo_Window(w + WINW_EXTENSION, h), 
          RadialLayoutWindowCallbackInterface(this), 
          m_redrawStructures(true), imageData(NULL), crSurface(NULL) {
    copy_label(label);
    Construct(w + WINW_EXTENSION, h, structures);
}

DiagramWindow::DiagramWindow(int x, int y, int w, int h, const char *label,
                             const std::vector<int> &structures)
        : Fl_Cairo_Window(w + WINW_EXTENSION, h),
          RadialLayoutWindowCallbackInterface(this),     
          m_redrawStructures(true), imageData(NULL), crSurface(NULL) {
    copy_label(label);
    resize(x, y, w + WINW_EXTENSION, h);
    Construct(w + WINW_EXTENSION, h, structures);
}

DiagramWindow::~DiagramWindow() {
    Free(title);
    Free(m_menuItems);
    if(imageData != NULL) {
        delete imageData;
    }
    cairo_surface_destroy(crSurface);
    cairo_destroy(crDraw);
    cairo_surface_destroy(crZoomSurface);
    cairo_destroy(crZoom);
    cairo_surface_destroy(crBasePairsSurface);
    cairo_destroy(crBasePairsOverlay);
    Delete(m_drawBranchesIndicator, Fl_Check_Button);
    Delete(m_cbShowTicks, Fl_Check_Button);
    Delete(m_cbDrawBases, Fl_Check_Button);
    Delete(baseColorPaletteImg, Fl_RGB_Image);
    Delete(baseColorPaletteImgBtn, Fl_Button);
    Delete(baseColorPaletteChangeBtn, Fl_Button);
}

void DiagramWindow::SetFolderIndex(int folderIndex) {
    this->structureFolderIndex = this->folderIndex = folderIndex;
    Folder *dwinFolder = RNAStructViz::GetInstance()->
                            GetStructureManager()->
                                GetFolderAt(folderIndex);
    const char *structureNameFull = dwinFolder->folderName;
    RNAStructure *rnaStruct = RNAStructViz::GetInstance()->GetStructureManager()->
                  GetStructure(folderIndex);
    int basePairCount = (rnaStruct != NULL) ? rnaStruct->GetLength() : 0;
    this->sequenceLength = basePairCount;
    snprintf(title, 128, "Comparison of Arc Diagrams: %-.48s  -- % 5d Bases", 
        structureNameFull, basePairCount);
    title[127] = '\0';
    label(title);
}

int DiagramWindow::GetFolderIndex() const {
     return structureFolderIndex;
}

void DiagramWindow::ResetWindow(bool resetMenus = true) {
    
    if (resetMenus) {
        m_menus[0]->value(0);
        m_menus[1]->value(0);
        m_menus[2]->value(0);
        if(m_drawBranchesIndicator != NULL) {
	     m_drawBranchesIndicator->clear();
	}
        userConflictAlerted = false;
    }
    if(resetMenus) {
         zoomButtonDown = haveZoomBuffer = false;
         zoomBufferContainsArc = false;
         zx0 = zx1 = zy0 = zy1 = zw = zh = 0;
         zoomBufferMinArcIndex = zoomBufferMaxArcIndex = 0;
    }
    if(haveZoomBuffer) {
        cairo_set_source_rgb(crZoom, 
                             GetRed(GUI_WINDOW_BGCOLOR) / 255.0f, 
                             GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
                             GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
        cairo_rectangle(crZoom, 0, 0, ZOOM_WIDTH, ZOOM_HEIGHT);
        cairo_fill(crZoom);
    }
    redraw();

}

void DiagramWindow::checkBoxChangedStateCallback(Fl_Widget *, void *v) {
    Fl_Check_Button *cbDrawIndicator = (Fl_Check_Button *) v;
    if (cbDrawIndicator->changed()) {
        DiagramWindow *thisWindow = (DiagramWindow *) cbDrawIndicator->parent();
        thisWindow->m_redrawStructures = true;
        thisWindow->cairoTranslate = true;
        thisWindow->redraw();
        cbDrawIndicator->clear_changed();
    }
}

void DiagramWindow::exportToPNGButtonPressHandler(Fl_Widget *, void *v) {
         Fl_Button *buttonPressed = (Fl_Button *) v;
         DiagramWindow *thisWindow = (DiagramWindow *) buttonPressed->parent();
         bool userImageSaved = false;
         std::string exportFilePath = thisWindow->GetExportPNGFilePath();
         cairo_surface_t *pngInitSource = cairo_get_target(thisWindow->crDraw);
         cairo_surface_t *pngSource = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                                                                 IMAGE_WIDTH + GLWIN_TRANSLATEX + 5 * PNG_IMAGE_PADDING, 
							         IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + 
							         PNG_IMAGE_PADDING + DWIN_PNG_FOOTER_HEIGHT);
         cairo_t *crImageOutput = cairo_create(pngSource);
         thisWindow->SetCairoToExactFLColor(crImageOutput, thisWindow->color());
         cairo_rectangle(crImageOutput, 0, 0, IMAGE_WIDTH + GLWIN_TRANSLATEX + 5 * PNG_IMAGE_PADDING, 
		         IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + 
		         PNG_IMAGE_PADDING + DWIN_PNG_FOOTER_HEIGHT);
         cairo_fill(crImageOutput);
         // now draw the footer data:
         thisWindow->SetCairoToExactFLColor(crImageOutput, thisWindow->color());
         cairo_rectangle(crImageOutput, 0, IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + PNG_IMAGE_PADDING / 2, 
                                             IMAGE_WIDTH + GLWIN_TRANSLATEX + 5 * PNG_IMAGE_PADDING, DWIN_PNG_FOOTER_HEIGHT);
         cairo_fill(crImageOutput);
         thisWindow->SetCairoColor(crImageOutput, CairoColorSpec_t::CR_SOLID_BLACK);
         cairo_set_line_width(crImageOutput, 2);
         cairo_move_to(crImageOutput, 0, IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + PNG_IMAGE_PADDING / 2); 
         cairo_line_to(crImageOutput, IMAGE_WIDTH + GLWIN_TRANSLATEX + 5 * PNG_IMAGE_PADDING, 
		       IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + PNG_IMAGE_PADDING / 2);
         cairo_stroke(crImageOutput);
         cairo_select_font_face(crImageOutput, "Courier New", 
                                CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
         cairo_set_font_size(crImageOutput, 12);
         int offsetY = IMAGE_HEIGHT + GLWIN_TRANSLATEY + STRAND_MARKER_IMAGE_HEIGHT + 
		  PNG_IMAGE_PADDING / 2 + 25;
         char folderStructLabel[MAX_BUFFER_SIZE];
         snprintf(folderStructLabel, MAX_BUFFER_SIZE - 1, " * Sequence Folder: %s", 
                  RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(thisWindow->m_structures[0])->folderName);
         cairo_move_to(crImageOutput, 12, offsetY);
         cairo_show_text(crImageOutput, folderStructLabel);
         offsetY += 22;
         for(int s = 0; s < 3; s++) {
                   char curStructLabel[MAX_BUFFER_SIZE];
                   int menuChoiceIdx = thisWindow->m_menus[s]->value();
                   const char *curStructName = thisWindow->m_menus[s]->menu()[menuChoiceIdx].label();
                   snprintf(curStructLabel, MAX_BUFFER_SIZE - 1, 
                             " * Structure %d:          %s", 
                             s + 1, curStructName);
                   cairo_move_to(crImageOutput, 12, offsetY);
                   cairo_show_text(crImageOutput, curStructLabel);
                   offsetY += 22;
         }
         time_t currentTime = time(NULL);
         struct tm *tmCurrentTime = localtime(&currentTime);
         char imageTimeStamp[MAX_BUFFER_SIZE];
         strftime(imageTimeStamp, MAX_BUFFER_SIZE - 1, " * Image Generated: %c %Z", tmCurrentTime);
         cairo_move_to(crImageOutput, 12, offsetY);
         cairo_show_text(crImageOutput, imageTimeStamp);
         // draw the source diagram image onto the PNG output image:
         thisWindow->m_redrawStructures = true;
         thisWindow->Draw(thisWindow, crImageOutput, false);
         Delete(thisWindow->imgExportSelectWin, InputWindowExportImage);
	 thisWindow->imgExportSelectWin = new InputWindowExportImage(exportFilePath);
	 while(thisWindow->imgExportSelectWin->visible() || !thisWindow->imgExportSelectWin->done()) {
	      Fl::wait(1.0);
	 }
	 if(!thisWindow->imgExportSelectWin->WriteImageToFile(pngSource)) {
	         TerminalText::PrintError("WRITING IMAGE \"%s\" TO FILE: %s\n", 
				          thisWindow->imgExportSelectWin->GetOutputFileSavePath().c_str(), strerror(errno));          
         }
         cairo_surface_destroy(pngSource);
         cairo_destroy(crImageOutput);
         thisWindow->drawWidgets(NULL);
         thisWindow->redraw();
}

void DiagramWindow::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
}

void DiagramWindow::drawWidgets(cairo_t *crDraw = NULL) {
    
    if(exportButton) { 
         exportButton->redraw();
    }
    if(m_cbShowTicks) { 
     m_cbShowTicks->redraw();
    }
    if(m_cbDrawBases) { 
     m_cbDrawBases->redraw();
    }
    if(baseColorPaletteImgBtn) {
     baseColorPaletteImgBtn->redraw();
    }
    if(baseColorPaletteChangeBtn) {
     baseColorPaletteChangeBtn->redraw();
    }
    for(int m = 0; m < 3; m++) {
        if(m_menus[m] != NULL && !m_menus[m]->active()) {
        m_menus[m]->redraw();
    }
    }
    if(crDraw != NULL) {
     cairo_save(crDraw);
     CairoColor_t::FromFLColorType(GUI_WINDOW_BGCOLOR).ApplyRGBAColor(crDraw);
     cairo_rectangle(crDraw, 0, 0, w(), h());
     cairo_fill(crDraw);
     cairo_restore(crDraw);
    }
    Fl_Double_Window::draw();

}

bool DiagramWindow::computeDrawKeyParams(RNAStructure **sequences, int *numToDraw, int *keyA, int *keyB) {

    if(numToDraw == NULL || keyA == NULL || keyB == NULL) {
         return false;
    }

    // Get the structures. Be sure the reference structure is first.
    //RNAStructure *sequences[3];
    StructureManager *structureManager =
            RNAStructViz::GetInstance()->GetStructureManager();
    for (int j = 0; j < 3; ++j) {
        if ((intptr_t) (m_menuItems[m_menus[j]->value()].user_data()) == -1) {
            sequences[j] = 0;
        } else {
            sequences[j] = structureManager->GetStructure(
                    (intptr_t) (m_menuItems[m_menus[j]->value()].user_data()));
            if (sequences[j]->GetLength() > 1000) {
                pixelWidth = 1;
            } 
	    else if (sequences[j]->GetLength() <= 1000 && 
                       sequences[j]->GetLength() >= 500) {
                pixelWidth = 2;
            } 
	    else {
                pixelWidth = 3;
            }
        }
    }

    *numToDraw = 0, *keyA = 0, *keyB = 0;
    if (sequences[0]) {
        if (sequences[1]) {
            if (sequences[2]) {
                *numToDraw = 3;
                ComputeNumPairs(sequences, 3);
            } else {
                *numToDraw = 2;
                ComputeNumPairs(sequences, 2);
                *keyA = 0; *keyB = 1;
            }
        } else {
            if (sequences[2]) {
                sequences[1] = sequences[2];
                sequences[2] = NULL;
                ComputeNumPairs(sequences, 2);
                *keyA = 0; *keyB = 2;
                *numToDraw = 2;
            } else {
                ComputeNumPairs(sequences, 1);
                *keyA = 0;
                *numToDraw = 1;
            }
        }
    } else {
        if (sequences[1]) {
            if (sequences[2]) {
                sequences[0] = sequences[1];
                sequences[1] = sequences[2];
                sequences[2] = NULL;
                ComputeNumPairs(sequences, 2);
                *keyA = 1; *keyB = 2;
                *numToDraw = 2;
            } else {
                sequences[0] = sequences[1];
                sequences[1] = NULL;
                ComputeNumPairs(sequences, 1);
                *keyA = 1;
                *numToDraw = 1;
            }
        } else {
            if (sequences[2]) {
                sequences[0] = sequences[2];
                sequences[2] = NULL;
                ComputeNumPairs(sequences, 1);
                *keyA = 2;
                *numToDraw = 1;
            } else {
                *numToDraw = 0;
            }
        }
    }
    return true;

}

void DiagramWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr, bool redrawWidgets) {

    DiagramWindow *thisWindow = (DiagramWindow *) thisCairoWindow;
    thisWindow->cursor(FL_CURSOR_WAIT);
    if(redrawWidgets) {
         thisWindow->drawWidgets(cr);
    }

    RNAStructure *sequences[3];
    int numToDraw, keyA, keyB;
    thisWindow->computeDrawKeyParams(sequences, &numToDraw, &keyA, &keyB);
      
    if(thisWindow->m_redrawStructures) {
        cairo_t *crDraw = redrawWidgets ? thisWindow->crDraw : cr;
	// __Draw the actual arc diagram pixels and frame 
        //   them in a circular frame:__ 
	cairo_identity_matrix(crDraw);
        if(!redrawWidgets) {
	     cairo_translate(crDraw, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
	}
        thisWindow->SetCairoToExactFLColor(crDraw, thisWindow->color());
        cairo_rectangle(crDraw, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
        cairo_fill(crDraw);
        thisWindow->SetCairoToExactFLColor(thisWindow->crBasePairsOverlay, thisWindow->color());
        cairo_rectangle(thisWindow->crBasePairsOverlay, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
        cairo_fill(thisWindow->crBasePairsOverlay);
        cairo_push_group(crDraw);
        int drawParams[] = { numToDraw, keyA, keyB };
        thisWindow->RedrawBuffer(crDraw, sequences, drawParams, DIAGRAM_WIDTH);
        cairo_pop_group_to_source(crDraw);
        if(thisWindow->m_cbDrawBases->value()) {
             cairo_save(crDraw);
             cairo_set_source_surface(crDraw, cairo_get_target(thisWindow->crBasePairsOverlay), 0, 0);
             cairo_rectangle(crDraw, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);    
             cairo_fill(crDraw);
             cairo_restore(crDraw);
        }
        cairo_arc(crDraw, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 
                  DIAGRAM_WIDTH / 2, 0.0, 2.0 * M_PI);
        cairo_clip(crDraw);
        cairo_paint(crDraw);
        cairo_reset_clip(crDraw);
        cairo_arc(crDraw, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 
                  DIAGRAM_WIDTH / 2, 0.0, 2.0 * M_PI);
        thisWindow->SetCairoColor(crDraw, CairoColorSpec_t::CR_BLACK);
        cairo_stroke(crDraw);
        thisWindow->RedrawStructureTickMarks(crDraw);
        if(!redrawWidgets) {
	     cairo_identity_matrix(crDraw);
	}
	thisWindow->m_redrawStructures = false;
    }
    cairo_set_source_surface(cr, cairo_get_target(thisWindow->crDraw), 
                             GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_rectangle(cr, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY, 
                    IMAGE_WIDTH, IMAGE_HEIGHT);    
    cairo_fill(cr);
     
    thisWindow->RedrawStrandEdgeMarker(cr);
    if(redrawWidgets) {
         thisWindow->RedrawCairoZoomBuffer(cr);
    }
   
    if(numToDraw == 1) {
        thisWindow->DrawKey1(cr, keyA);
    }
    else if(numToDraw == 2) {
        thisWindow->DrawKey2(cr, keyA, keyB);
    }
    else if(numToDraw == 3) {
        thisWindow->DrawKey3(cr);
    }
	
    thisWindow->cursor(DIAGRAMWIN_DEFAULT_CURSOR);
    thisWindow->show();

}

void DiagramWindow::RedrawBuffer(cairo_t *cr, RNAStructure **structures,
                                 const int *structParams, 
                                 const int resolution) {

    int priorFont = fl_font();
    int priorFontSize = fl_size();
    fl_font(priorFont, 10);
    fl_line_style(0);
    
    SetCairoColor(cr, CairoColorSpec_t::CR_SOLID_WHITE);
    cairo_rectangle(cr, 0, 0, this->w(), this->h());
    cairo_fill(cr);

    int numStructures = structParams[0];
    int keyA = structParams[1], keyB = structParams[2];
    if (numStructures == 1) {
        Draw1(cr, structures, resolution);
    } else if (cr, numStructures == 2) {
        Draw2(cr, structures, resolution);
    } else if (numStructures == 3) {
        Draw3(cr, structures, resolution);
    }

    fl_font(priorFont, priorFontSize);
}

void DiagramWindow::DrawWithCairo::fl_rectf(cairo_t *crDraw, int x, int y, int w, int h) {
     if(crDraw == NULL) {
          return;
     }
     cairo_rectangle(crDraw, x, y, w, h);
     cairo_fill(crDraw);
}

void DiagramWindow::DrawWithCairo::fl_draw(cairo_t *crDraw, const char *drawStr, int x, int y) {
     if(crDraw == NULL || drawStr == NULL) {
          return;
     }
     cairo_move_to(crDraw, x, y);
     cairo_show_text(crDraw, drawStr);
}

void DiagramWindow::DrawWithCairo::fl_line_style(cairo_t *crDraw, int lineStyle) {
     if(crDraw == NULL) {
          return;
     }
     switch(lineStyle) {
          case FL_DASH: {
               double dashes[] = { 4.0, 4.0 };
           int ndash = 2;
               double offset = 0.0;
           cairo_set_dash(crDraw, dashes, ndash, offset);
           break;
      }
      case FL_DOT: {
           double dots[] = { 2.5, 2.5 };
           int ndots = 2;
               double offset = 0.0;
           cairo_set_dash(crDraw, dots, ndots, offset);
           break;
      }
      case FL_SOLID: {
           double dashes[] = { 1.0 };
           int ndash = 1;
               double offset = 0.0;
           cairo_set_dash(crDraw, dashes, ndash, offset);
           break;
      }
      default:
           break;
     }
}

void DiagramWindow::DrawWithCairo::fl_xyline(cairo_t *crDraw, int x, int y, int lineWidth) {
     if(crDraw == NULL) {
          return;
     }
     cairo_move_to(crDraw, x, y);
     cairo_line_to(crDraw, lineWidth, y + 2);
     cairo_set_line_width(crDraw, 1.0);
     cairo_stroke(crDraw);
}

void DiagramWindow::DrawKey3(cairo_t *crDraw) {
    
    if(crDraw == NULL) {
        return;
    }
    cairo_save(crDraw);
    cairo_select_font_face(crDraw, "monospace", CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crDraw, BASE_LINE_FONT_SIZE); 
    cairo_set_line_cap(crDraw, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(crDraw, CAIRO_LINE_JOIN_MITER);

    int yPosn = 55;
    char mystr[10];
    SetStringToEmpty(mystr);
    int dashedLineStyle = FL_DASH; // FL_DOT | FL_SOLID

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][0]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    sprintf(mystr, "%d", numPairs[0]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][0], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][5]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    DrawWithCairo::fl_xyline(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[1]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][1], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][4]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    DrawWithCairo::fl_xyline(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[2]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][2], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][2]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    DrawWithCairo::fl_xyline(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    sprintf(mystr, "%d", numPairs[6]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][6], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][1]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[3]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][3], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][3]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    sprintf(mystr, "%d", numPairs[4]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][4], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[2][6]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    sprintf(mystr, "%d", numPairs[5]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[2][5], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
   
    cairo_restore(crDraw);

}

void DiagramWindow::DrawKey2(cairo_t *crDraw, const int a, const int b) {
        
    if(crDraw == NULL) {
        return;
    }
    cairo_save(crDraw);
    cairo_select_font_face(crDraw, "monospace", CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crDraw, BASE_LINE_FONT_SIZE); 

    int yPosn = 55;
    char mystr[10];
    SetStringToEmpty(mystr);
    int dashedLineStyle = FL_DASH; // FL_DOT | FL_SOLID
 
    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[1][0]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    DrawWithCairo::fl_rectf(crDraw, m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
    sprintf(mystr, "%d", numPairs[0]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[1][0], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[1][1]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[b]->x(), yPosn, m_menus[b]->x() + m_menus[b]->w());
    sprintf(mystr, "%d", numPairs[1]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[1][1], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    yPosn += 10;

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[1][2]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
    DrawWithCairo::fl_line_style(crDraw, dashedLineStyle);
    DrawWithCairo::fl_xyline(crDraw, m_menus[a]->x(), yPosn, m_menus[a]->x() + m_menus[a]->w());
    sprintf(mystr, "%d", numPairs[2]);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[1][2], STRUCTURE_INCLBL_XOFFSET, yPosn + 3);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);

    cairo_restore(crDraw);

}

void DiagramWindow::DrawKey1(cairo_t *crDraw, const int a) {
        
    if(crDraw == NULL) {
        return;
    }
    cairo_save(crDraw);
    cairo_select_font_face(crDraw, "monospace", CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crDraw, BASE_LINE_FONT_SIZE); 

    SetCairoToFLColor(crDraw, STRUCTURE_DIAGRAM_COLORS[0][0]);
    DrawWithCairo::fl_rectf(crDraw, m_menus[a]->x(), 55, m_menus[a]->w(), 3);
    char mystr[10];
    SetStringToEmpty(mystr);
    sprintf(mystr, "%d", numPairs[0]);
    DrawWithCairo::fl_draw(crDraw, mystr, m_menus[2]->x() + m_menus[2]->w() + 10, 55 + 3);
    DrawWithCairo::fl_draw(crDraw, BASE_LINE_INCL_LABELS[0][0], STRUCTURE_INCLBL_XOFFSET, 55 + 3);

    cairo_restore(crDraw);

}

void DiagramWindow::SetCairoBranchColor(cairo_t *cr, 
            const BranchID_t &branchType, int enabled,
                    CairoColorSpec_t fallbackColorFlag) {

    int nextColorFlag = fallbackColorFlag;
    if (enabled && branchType != BRANCH_UNDEFINED) {
        switch (branchType) {
            case BRANCH1:
                nextColorFlag = CairoColorSpec_t::CR_BRANCH1;
                break;
            case BRANCH2:
                nextColorFlag = CairoColorSpec_t::CR_BRANCH2;
                break;
            case BRANCH3:
                nextColorFlag = CairoColorSpec_t::CR_BRANCH3;
                break;
            case BRANCH4:
                nextColorFlag = CairoColorSpec_t::CR_BRANCH4;
                break;
            default:
                break;
        }
    }
    SetCairoColor(cr, nextColorFlag);

}
    
void DiagramWindow::SetCairoColor(cairo_t *cr, int nextColorFlag, bool toOpaque) {
    if(!toOpaque) {
         CairoColor_t::GetCairoColor((CairoColorSpec_t) nextColorFlag).ApplyRGBAColor(cr);
    }
    else {
         CairoColor_t::GetCairoColor((CairoColorSpec_t) nextColorFlag).ToOpaque().ApplyRGBAColor(cr);
    }
}

void DiagramWindow::SetCairoToFLColor(cairo_t *cr, Fl_Color flc) {
     if(cr == NULL) {
          return;
     }
     CairoColor_t::CairoColorSpec_t crEquivColor = CairoColor_t::ConvertFromFLColor(flc);
     CairoColor_t::FromNamedConstant(crEquivColor).ApplyRGBAColor(cr);
}

void DiagramWindow::SetCairoToExactFLColor(cairo_t *cr, Fl_Color flc) {
     if(cr == NULL) {
          return;
     }
     CairoColor_t::FromFLColorType(flc).ToOpaque().ApplyRGBAColor(cr);
}

void DiagramWindow::Draw3(cairo_t *cr, RNAStructure **structures, const int resolution) {
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase,
                         angleDelta, radius);

    WarnUserDrawingConflict();

    for (unsigned int ui = 0; ui < numBases; ++ui) {
        const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
        if(m_cbDrawBases->value()) {
             DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
                      radius + 7.5f);
        }

        const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
        const RNAStructure::BaseData *baseData3 = structures[2]->GetBaseAt(ui);

        if (baseData1->m_pair != RNAStructure::UNPAIRED &&
            baseData1->m_pair > ui) {
            if (baseData1->m_pair == baseData2->m_pair) {
                if (baseData1->m_pair == baseData3->m_pair) {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[2][0]);
                    SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][0]);
                    #if PERFORM_BRANCH_TYPE_ID
                         SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLACK);
                    #endif
                    DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                } else {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[2][1]);
                    SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][1]);
                    #if PERFORM_BRANCH_TYPE_ID
                         SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_YELLOW);
                    #endif
                    DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);

                    if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                        baseData3->m_pair > ui) {
                        fl_color(STRUCTURE_DIAGRAM_COLORS[2][2]);
                        SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][2]);
                        #if PERFORM_BRANCH_TYPE_ID
                             SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                                 (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLUE);
                        #endif
                        DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    }
                }
            } 
	    else if (baseData1->m_pair == baseData3->m_pair) {
                fl_color(STRUCTURE_DIAGRAM_COLORS[2][3]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][3]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_MAGENTA);
                #endif
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[2][4]);
                    SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][4]);
                    #if PERFORM_BRANCH_TYPE_ID
                         SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                             (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_GREEN);
                    #endif
                    DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            } 
	    else {
                fl_color(STRUCTURE_DIAGRAM_COLORS[2][5]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][5]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_RED);
                #endif
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    if (baseData2->m_pair == baseData3->m_pair) {
                        fl_color(STRUCTURE_DIAGRAM_COLORS[2][6]);
                        SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][6]);
                        #if PERFORM_BRANCH_TYPE_ID
                             SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                                 (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_CYAN);
                        #endif
                        DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    } else {
                        fl_color(STRUCTURE_DIAGRAM_COLORS[2][4]);
                        SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][4]);
                        #if PERFORM_BRANCH_TYPE_ID
                             SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                                 (int) m_drawBranchesIndicator->value(), CairoColoDrawBasesCR_GREEN);
                        #endif
                        DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);

                        if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                            baseData3->m_pair > ui) {
                            fl_color(STRUCTURE_DIAGRAM_COLORS[2][2]);
                            SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][2]);
                            #if PERFORM_BRANCH_TYPE_ID
                                 SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                                     (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLUE);
                            #endif
                            DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                                    angleDelta, radius);
                        }
                    }
                } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                           baseData3->m_pair > ui) {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[2][2]);
                    SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][2]);
                    #if PERFORM_BRANCH_TYPE_ID
                         SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                             (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLUE);
                    #endif
                    DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                   baseData2->m_pair > ui) {
            if (baseData2->m_pair == baseData3->m_pair) {
                fl_color(STRUCTURE_DIAGRAM_COLORS[2][6]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][6]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_CYAN);
                #endif
                DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(STRUCTURE_DIAGRAM_COLORS[2][4]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][4]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_GREEN);
                #endif
                DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                    baseData3->m_pair > ui) {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[2][2]);
            SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][2]);
                    #if PERFORM_BRANCH_TYPE_ID
            SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLUE);
                    #endif
            DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                   baseData3->m_pair > ui) {
            fl_color(STRUCTURE_DIAGRAM_COLORS[2][2]);
            SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[2][2]);
            #if PERFORM_BRANCH_TYPE_ID
                 SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                     (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLUE);
            #endif
            DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
        }
    }
}

void DiagramWindow::Draw2(cairo_t *cr, RNAStructure **structures, const int resolution) {
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase,
                         angleDelta, radius);
    WarnUserDrawingConflict();

    for (unsigned int ui = 0; ui < numBases; ++ui) {
        const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
        if(m_cbDrawBases->value()) {
             DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
                      radius + 7.5f);
    }

        const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
        if (baseData1->m_pair != RNAStructure::UNPAIRED
            && baseData1->m_pair > ui) {
            if (baseData1->m_pair == baseData2->m_pair) {
                fl_color(STRUCTURE_DIAGRAM_COLORS[1][0]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[1][0]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLACK);
                #endif
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(STRUCTURE_DIAGRAM_COLORS[1][1]);
                SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[1][1]);
                #if PERFORM_BRANCH_TYPE_ID
                     SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                         (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_RED);
                #endif
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair !=
                    RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
                    fl_color(STRUCTURE_DIAGRAM_COLORS[1][2]);
                    SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[1][2]);
                    #if PERFORM_BRANCH_TYPE_ID
                         SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                             (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_GREEN);
                    #endif
                     DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair !=
                   RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
            fl_color(STRUCTURE_DIAGRAM_COLORS[1][2]);
            SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[1][2]);
            #if PERFORM_BRANCH_TYPE_ID
                 SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                     (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_GREEN);
            #endif
            DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
        }
    }
}

void DiagramWindow::Draw1(cairo_t *cr, RNAStructure **structures, const int resolution) {
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;
    int counter = 0;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase,
                         angleDelta, radius);

    for (unsigned int ui = 0; ui < numBases; ++ui) {
        const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
        if(m_cbDrawBases->value()) { 
             DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
                      radius + 7.5f);
        }

	if (baseData1->m_pair != RNAStructure::UNPAIRED
            && baseData1->m_pair > ui) {
            fl_color(STRUCTURE_DIAGRAM_COLORS[0][0]);
            SetCairoToFLColor(cr, STRUCTURE_DIAGRAM_COLORS[0][0]);
            #if PERFORM_BRANCH_TYPE_ID
                 SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                     (int) m_drawBranchesIndicator->value(), CairoColorSpec_t::CR_BLACK);
            #endif
            DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
            counter++;
        }
    }
}

void DiagramWindow::ComputeNumPairs(RNAStructure **structures,
                                    int numStructures) {

    if(numStructures == 0) {
         return;
    }
    for(int np = 0; np < 7; np++) {
         numPairs[np] = 0;
    }

    unsigned int numBases = structures[0]->GetLength();
    if (numStructures == 1) {
        unsigned int counter1 = 0;
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            const RNAStructure::BaseData *baseData1 =
                    structures[0]->GetBaseAt(ui);
            if (baseData1->m_pair != RNAStructure::UNPAIRED &&
		ui < baseData1->m_pair) {
                counter1++;
	    }
        }
        numPairs[0] = counter1;
    } else if (numStructures == 2) {
        unsigned int counter1 = 0; // black (both structure 1 and 2)
        unsigned int counter2 = 0; // red (structure 1 only)
        unsigned int counter3 = 0; // green (structure 2 only)
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
	    if (baseData1->m_pair != RNAStructure::UNPAIRED && 
		baseData1->m_pair == baseData2->m_pair && 
		baseData1->m_pair > ui) {
                    counter1++;
            } 
	    else {
	         if (baseData1->m_pair != RNAStructure::UNPAIRED &&
                     baseData1->m_pair > ui) {
                    counter2++;
                 } 
	         if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                     baseData2->m_pair > ui) {
		    counter3++;
                 }
	    }
        }
        numPairs[0] = counter1;
        numPairs[1] = counter2;
        numPairs[2] = counter3;
    } else if (numStructures == 3) {
        unsigned int counter1 = 0; // black = in all 3 structures
        unsigned int counter2 = 0; // red = in only structure 1
        unsigned int counter3 = 0; // green = in only structure 2 
        unsigned int counter4 = 0; // yellow = in structures 1 & 2
        unsigned int counter5 = 0; // magenta = in structures 1 & 3
        unsigned int counter6 = 0; // cyan = in structures 2 & 3
        unsigned int counter7 = 0; // blue = in only structure 3
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            
            const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData3 = structures[2]->GetBaseAt(ui);
            
            // define pair counting functions using C++ lambda expressions:
            auto PCEligible = [](const RNAStructure::BaseData *bd, unsigned int cidx) {
                 return (bd->m_pair != RNAStructure::UNPAIRED) && (bd->m_pair > cidx);
            };
            auto PC1 = [](const RNAStructure::BaseData *bd1, const RNAStructure::BaseData *bd2, 
                          const RNAStructure::BaseData *bd3) {
                 return (bd1->m_pair != bd2->m_pair) && (bd1->m_pair != bd3->m_pair);
            };
            auto PC2 = [](const RNAStructure::BaseData *bd1, const RNAStructure::BaseData *bd2, 
                          const RNAStructure::BaseData *bd3) {
                 return (bd1->m_pair == bd2->m_pair) && (bd1->m_pair != bd3->m_pair);
            };
            auto PC3 = [](const RNAStructure::BaseData *bd1, const RNAStructure::BaseData *bd2, 
                          const RNAStructure::BaseData *bd3) {
                 return (bd1->m_pair == bd2->m_pair) && (bd1->m_pair == bd3->m_pair);
            };

            if(PCEligible(baseData1, ui) && PC3(baseData1, baseData2, baseData3)) counter1++;
            if(PCEligible(baseData1, ui) && PC2(baseData1, baseData2, baseData3)) counter4++;
            if(PCEligible(baseData1, ui) && PC2(baseData1, baseData3, baseData2)) counter5++;
            if(PCEligible(baseData2, ui) && PC2(baseData2, baseData3, baseData1)) counter6++;
            if(PCEligible(baseData1, ui) && PC1(baseData1, baseData2, baseData3)) counter2++;
            if(PCEligible(baseData2, ui) && PC1(baseData2, baseData1, baseData3)) counter3++;
            if(PCEligible(baseData3, ui) && PC1(baseData3, baseData1, baseData2)) counter7++;

            /*if (baseData1->m_pair != RNAStructure::UNPAIRED &&
                baseData1->m_pair > ui) {
                if (baseData1->m_pair == baseData2->m_pair && 
		    baseData1->m_pair > ui) {
                    if (baseData1->m_pair == baseData3->m_pair) {
                        counter1++;
                    } else {
                        counter4++;
                        if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                            baseData3->m_pair > ui) {
                            counter7++;
                        }
                    }
                } else if (baseData1->m_pair == baseData3->m_pair && 
		           baseData1->m_pair > ui) {
                    counter5++;

                    if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                        baseData2->m_pair > ui) {
                        counter3++;
                    }
                } else {
                    counter2++;
                    if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                        baseData2->m_pair > ui) {
                        if (baseData2->m_pair == baseData3->m_pair) {
                            counter6++;
                        } else {
                            counter3++;

                            if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                                baseData3->m_pair > ui) {
                                counter7++;
                            }
                        }
                    } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                               baseData3->m_pair > ui) {
                        counter7++;
                    }
                }
            } else if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                       baseData2->m_pair > ui) {
                if (baseData2->m_index == baseData3->m_pair) {
                    counter6++;
                } else {
                    counter3++;

                    if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                        baseData3->m_pair > ui) {
                        counter7++;
                    }
                }
            } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                       baseData3->m_pair > ui) {
                counter7++;
            }*/
        }
        numPairs[0] = counter1;
        numPairs[1] = counter2;
        numPairs[2] = counter3;
        numPairs[3] = counter4;
        numPairs[4] = counter5;
        numPairs[5] = counter6;
        numPairs[6] = counter7;
    
     }
}

void DiagramWindow::ComputeCircle(
        const float &x1,
        const float &y1,
        const float &x2,
        const float &y2,
        const float &x3,
        const float &y3,
        double &cX,
        double &cY,
        double &r) {
    
    double denom = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - y2 * x3;
    //if (denom < 0.25) {
    //    cX = cY = 0.0f;
    //    r = 0.0f;
    //    return;
    //}

    double sq1 = x1 * x1 + y1 * y1;
    double sq2 = x2 * x2 + y2 * y2;
    double sq3 = x3 * x3 + y3 * y3;

    cX = (sq1 * (y2 - y3) - y1 * (sq2 - sq3) + sq2 * y3 - y2 * sq3) / (2.0 * denom);
    cY = (x1 * (sq2 - sq3) - sq1 * (x2 - x3) + sq3 * x2 - x3 * sq2) / (2.0 * denom);
    r = sqrt((x1 - cX) * (x1 - cX) + (y1 - cY) * (y1 - cY));

}

void DiagramWindow::DrawArc(
        cairo_t *cr, 
        const unsigned int b1,
        const unsigned int b2,
        const float centerX,
        const float centerY,
        const float angleBase,
        const float angleDelta,
        const float radius) {
    float angle1 = angleBase - (float) b1 * angleDelta;
    float xPosn1 = centerX + cos(angle1) * radius;
    float yPosn1 = centerY - sin(angle1) * radius;

    float angle2 = angleBase - (float) b2 * angleDelta;
    float xPosn2 = centerX + cos(angle2) * radius;
    float yPosn2 = centerY - sin(angle2) * radius;

    // Calculate a third point on the arc, midway between the endpoints.
    float midAngle = (angle1 + angle2) / 2.0f;
    float diffAngleRatio = (angle1 - angle2) / M_PI;
    float xPosn3 = centerX + cos(midAngle) * radius * (1.0f - diffAngleRatio);
    float yPosn3 = centerY - sin(midAngle) * radius * (1.0f - diffAngleRatio);

    double arcX = 0.0f;
    double arcY = 0.0f;
    double arcR = 0.0f;
    ComputeCircle(xPosn1, yPosn1, xPosn2, yPosn2, xPosn3, yPosn3, arcX, arcY, arcR);

    float boundX = (arcX - arcR);
    float boundY = (arcY - arcR);
    float boundSize = 2.0 * arcR;
    double arc1 = 180.0 / M_PI * atan2(arcY - yPosn1, xPosn1 - arcX);
    double arc2 = 180.0 / M_PI * atan2(arcY - yPosn2, xPosn2 - arcX);

    float boundingBoxCenterX = boundX + boundSize / 2;
    float boundingBoxCenterY = boundY + boundSize / 2;
    float boundingBoxRadius = boundSize / 2.0;
    cairo_set_line_width(cr, pixelWidth);
    
    while(arc1 < 0.0) arc1 += 360.0;
    while(arc1 > 360.0) arc1 -= 360.0;
    while(arc2 < 0.0) arc2 += 360.0;
    while(arc2 > 360.0) arc2 -= 360.0;
    //if (arc1 > 180.0)
    //    arc1 += 360.0;
    //if (arc1 - arc2 > 180.0)
    //    arc2 += 360.0;

    // cairo arc drawing functions require the angles to be in radians:
    arc1 = arc1 * M_PI / 180.0;
    arc2 = arc2 * M_PI / 180.0;

    if (arc2 > arc1) {
        cairo_arc_negative(cr, boundingBoxCenterX, boundingBoxCenterY, 
                  boundingBoxRadius, arc2, arc1);
        cairo_stroke(cr);
        cairo_arc(cr, boundingBoxCenterX, boundingBoxCenterY, 
                  boundingBoxRadius, arc2, arc1);
        cairo_stroke(cr);
    } 
    else {
        cairo_arc_negative(cr, boundingBoxCenterX, boundingBoxCenterY, 
                  boundingBoxRadius, arc1, arc2);
        cairo_stroke(cr);
        cairo_arc(cr, boundingBoxCenterX, boundingBoxCenterY, 
                  boundingBoxRadius, arc1, arc2);
        cairo_stroke(cr);
    }
}

void DiagramWindow::DrawBase(
        const unsigned int index,
        const RNAStructure::Base base,
        const float centerX,
        const float centerY,
        const float angleBase,
        const float angleDelta,
        const float radius) {
    
    if(sequenceLength > BASE_PAIRS_AROUND_CIRCLE && (index % (sequenceLength / BASE_PAIRS_AROUND_CIRCLE)) != 0) {
         return;
    }	 
	
    float angle1 = angleBase - (float) index * angleDelta;
    float xPosn1 = centerX + cos(angle1) * (radius + 5);
    float yPosn1 = centerY - sin(angle1) * (radius + 5) - 
               fl_descent() + 0.5 * fl_height();
    const char *baseChar = "X";
    CairoColor_t baseColor = CairoColor_t::FromFLColorType(FL_BLACK);
    switch (base) {
        case RNAStructure::A:
        baseChar = "A";
            baseColor = CairoColor_t::FromFLColorType(FL_LOCAL_MEDIUM_GREEN);
            break;
        case RNAStructure::C:
        baseChar = "C";
            baseColor = CairoColor_t::FromFLColorType(FL_LOCAL_DARK_RED);
            break;
        case RNAStructure::G:
        baseChar = "G";
            baseColor = CairoColor_t::FromFLColorType(FL_LOCAL_LIGHT_PURPLE);
            break;
        case RNAStructure::U:
        baseChar = "U";
            baseColor = CairoColor_t::FromFLColorType(FL_LOCAL_BRIGHT_YELLOW);
            break;
    }
    cairo_save(crBasePairsOverlay);
    cairo_text_extents_t textDims;
    cairo_text_extents(crBasePairsOverlay, baseChar, &textDims);
    baseColor.ApplyRGBAColor(crBasePairsOverlay);
    cairo_select_font_face(crBasePairsOverlay, "monospace", 
                   CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crBasePairsOverlay, CairoContext_t::FONT_SIZE_SMALLER);
    cairo_move_to(crBasePairsOverlay, (int) (xPosn1 - textDims.width / 2), (int) yPosn1);
    cairo_show_text(crBasePairsOverlay, baseChar);
    cairo_restore(crBasePairsOverlay);
    fl_color(GUI_WINDOW_BGCOLOR);
}

void DiagramWindow::ComputeDiagramParams(
        const int numBases,
        const int resolution,
        float &centerX,
        float &centerY,
        float &angleBase,
        float &angleDelta,
        float &radius) {
    angleDelta = (M_PI * 2.0f) / (float) numBases;
    angleBase = 1.5f * M_PI;
    centerX = (float) resolution / DIAGRAM_TO_IMAGE_RATIO / 2.0f;
    centerY = (float) resolution / DIAGRAM_TO_IMAGE_RATIO / 2.0f;
    radius = resolution / 2.0f;
}

bool DiagramWindow::GetMouseoverArcDiagramBoundingCircle(int x, int y, int &selectedBaseIdx, int &biCoordX, int &biCoordY) {
     double originX = GLWIN_TRANSLATEX + IMAGE_DIM / 2;
     double originY = GLWIN_TRANSLATEY + IMAGE_DIM / 2;
     double radius = DIAGRAM_WIDTH / 2;
     double denom = sqrt((x - originX) * (x - originX) + (y - originY) * (y - originY));
     biCoordX = originX + radius * (x - originX) / denom;
     biCoordY = originY + radius * (y - originY) / denom;
     double biCoordTheta = atan2(-(biCoordY - originY), -(biCoordX - originX)) + M_PI_2;
     while(biCoordTheta < 0.0) biCoordTheta += 2.0f * M_PI;
     int biCoordThetaIntMod = biCoordTheta * 180.0 / M_PI;
     biCoordThetaIntMod = fmod(biCoordThetaIntMod, 360);
     //fprintf(stderr, "  >> (X, Y) = (%d, %d), InitTheta=%d,%d\n", biCoordX, biCoordY, 
     //        (int) (biCoordTheta * 180.0 / M_PI), (int) (biCoordTheta * 180.0 / M_PI) % 360);
     selectedBaseIdx = (int) ceilf(DiagramWindow::lastClickSequenceLength / 360.0 * biCoordThetaIntMod);
     double edist = sqrt(Square(x - biCoordX) + Square(y - biCoordY));
     return (int) edist <= GLWIN_ARCTOL;
}

void DiagramWindow::AddStructure(const int index) {
    if (std::find(m_structures.begin(), m_structures.end(), index) == m_structures.end()) {
        m_structures.push_back(index);
        RebuildMenus();
        redraw();
    }
}

void DiagramWindow::RemoveStructure(const int index) {
    std::vector<int>::iterator iter = std::find(m_structures.begin(),
                                                m_structures.end(), index);
    if (iter != m_structures.end()) {
        m_structures.erase(iter);

        intptr_t user_data0 = (intptr_t) (m_menuItems[m_menus[0]->value()].user_data());
        intptr_t user_data1 = (intptr_t) (m_menuItems[m_menus[1]->value()].user_data());
        intptr_t user_data2 = (intptr_t) (m_menuItems[m_menus[2]->value()].user_data());

        RebuildMenus();

        if ((intptr_t) (m_menuItems[m_menus[0]->value()].user_data()) != user_data0
            || (intptr_t) (m_menuItems[m_menus[1]->value()].user_data()) != user_data1
            || (intptr_t) (m_menuItems[m_menus[2]->value()].user_data()) != user_data2) {
            m_redrawStructures = true;
        }
        redraw();
    }
}

void DiagramWindow::SetStructures(const std::vector<int> &structures) {
    m_structures.clear();
    for (unsigned int ui = 0; ui < structures.size(); ++ui) {
        m_structures.push_back(structures[ui]);
    }
    RebuildMenus();
    redraw();
}

void DiagramWindow::RebuildMenus() {
    // Create the menus, if they don't already exist.
    int activeMenuIndex[3];
    bool activeSet[3];
    if (!m_menus[0]) {
        this->begin();

         Fl_Box *label = new Fl_Box(ms_menu_minx[0], 0, ms_menu_width, 25,
                                    "Structure 1");
         label->labelcolor(GUI_BTEXT_COLOR);
         label->labelfont(FL_HELVETICA_BOLD_ITALIC);
         label->labelsize(12);
         label = new Fl_Box(ms_menu_minx[1], 0, ms_menu_width, 25, "Structure 2");
         label->labelcolor(GUI_BTEXT_COLOR);
         label->labelfont(FL_HELVETICA_BOLD_ITALIC);
         label->labelsize(12);
         label = new Fl_Box(ms_menu_minx[2], 0, ms_menu_width, 25, "Structure 3");
         label->labelcolor(GUI_BTEXT_COLOR);
         label->labelfont(FL_HELVETICA_BOLD_ITALIC);
         label->labelsize(12);
         for(int m = 0; m < 3; m++) { 
             m_menus[m] = new Fl_Choice(ms_menu_minx[m], 25, ms_menu_width, 25);
             m_menus[m]->callback(MenuCallback);
             m_menus[m]->labelcolor(GUI_BTEXT_COLOR);
             m_menus[m]->textcolor(GUI_BTEXT_COLOR);
             m_menus[m]->labelfont(FL_HELVETICA);
             m_menus[m]->selection_color(Lighter(GUI_BTEXT_COLOR, 0.5f));
             m_menus[m]->activate();
             activeMenuIndex[m] = -1;
             activeSet[m] = false;
         }
         int horizCheckBoxPos = ms_menu_minx[2] + ms_menu_width + 2 * WIDGET_SPACING;
    
         #if PERFORM_BRANCH_TYPE_ID
             m_drawBranchesIndicator = new Fl_Check_Button(horizCheckBoxPos, 5, 
                                                           20, 20,"Draw (16S) Domains");
             m_drawBranchesIndicator->callback(checkBoxChangedStateCallback, 
                                               m_drawBranchesIndicator);
             m_drawBranchesIndicator->tooltip("Whether to color code the four domains in 16S structures");
             m_drawBranchesIndicator->labelcolor(GUI_TEXT_COLOR);
             if(!PERFORM_BRANCH_TYPE_ID) {
                  m_drawBranchesIndicator->hide(); // make this option invisible
             }
         #endif

         if(exportButton == NULL || m_cbShowTicks == NULL || baseColorPaletteImg == NULL) { 
              
              int offsetY = 25;
              exportButton = new Fl_Button(horizCheckBoxPos, offsetY, 
                                           EXPORT_BUTTON_WIDTH, 25, "@filesaveas   Export Image");
              exportButton->type(FL_NORMAL_BUTTON);
              exportButton->callback(exportToPNGButtonPressHandler, exportButton);
              exportButton->labelcolor(GUI_BTEXT_COLOR);
              exportButton->labelfont(FL_HELVETICA);
              exportButton->tooltip("Export the displayed arc diagram to image file");
     	      offsetY += 25;
     
              m_cbShowTicks = new Fl_Check_Button(horizCheckBoxPos + 4, offsetY, 
                                                  EXPORT_BUTTON_WIDTH, 25, 
                                                  "Draw Ticks");
              m_cbShowTicks->callback(ShowTickMarksCallback);
              m_cbShowTicks->type(FL_TOGGLE_BUTTON);
              m_cbShowTicks->labelcolor(GUI_BTEXT_COLOR);
              m_cbShowTicks->labelfont(FL_HELVETICA);
              m_cbShowTicks->labelsize(12);
              m_cbShowTicks->selection_color(GUI_TEXT_COLOR); // checkmark color
              m_cbShowTicks->value(showPlotTickMarks);
     	      m_cbShowTicks->tooltip("Show selected labeled tick marks around the bounding circle to denote base numbers");
              offsetY += 25;
                  
              m_cbDrawBases = new Fl_Check_Button(horizCheckBoxPos + 4, offsetY, 
                                                  EXPORT_BUTTON_WIDTH, 25, 
                                                  "Draw Bases");
              m_cbDrawBases->callback(DrawBasesCallback);
              m_cbDrawBases->type(FL_TOGGLE_BUTTON);
              m_cbDrawBases->labelcolor(GUI_BTEXT_COLOR);
              m_cbDrawBases->labelfont(FL_HELVETICA);
              m_cbDrawBases->labelsize(12);
              m_cbDrawBases->selection_color(GUI_TEXT_COLOR); // checkmark color
              m_cbDrawBases->value(0);
              m_cbDrawBases->tooltip("Draw selected bases from the sequence around the bounding circle");
     	      offsetY += 35;
     
              baseColorPaletteImg = new Fl_RGB_Image(
                                 BaseColorPaletteButtonImage.pixel_data, 
                                 BaseColorPaletteButtonImage.width, 
                                 BaseColorPaletteButtonImage.height, 
                                 BaseColorPaletteButtonImage.bytes_per_pixel
                            );
              int btnXPos = w() - 1.3 * BaseColorPaletteButtonImage.width;
              baseColorPaletteImgBtn = new Fl_Button(btnXPos, offsetY, 
                                             BaseColorPaletteButtonImage.width, 
                                             BaseColorPaletteButtonImage.height, "");
              baseColorPaletteImgBtn->color(GUI_WINDOW_BGCOLOR);
              baseColorPaletteImgBtn->labelcolor(GUI_BTEXT_COLOR);
              baseColorPaletteImgBtn->image(baseColorPaletteImg);
              baseColorPaletteImgBtn->deimage(baseColorPaletteImg);
              baseColorPaletteImgBtn->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
              baseColorPaletteImgBtn->labeltype(_FL_ICON_LABEL);
              baseColorPaletteImgBtn->shortcut(FL_CTRL + 'b');
              baseColorPaletteImgBtn->box(FL_NO_BOX);
              baseColorPaletteImgBtn->tooltip("Change the arc colors in the diagram");
     	      baseColorPaletteImgBtn->callback(ChangeBaseColorPaletteCallback);
              baseColorPaletteImgBtn->redraw();
              offsetY += BaseColorPaletteButtonImage.height + 6;
     
              baseColorPaletteChangeBtn = new Fl_Button(horizCheckBoxPos + 4, offsetY, 
                                                        EXPORT_BUTTON_WIDTH, 25, 
                                                        "Set Draw Colors @>|");
              baseColorPaletteChangeBtn->type(FL_NORMAL_BUTTON);
              baseColorPaletteChangeBtn->labelcolor(GUI_BTEXT_COLOR);
              baseColorPaletteChangeBtn->labelfont(FL_HELVETICA);
              baseColorPaletteChangeBtn->tooltip("Change the arc colors in the diagram");    
     	      baseColorPaletteChangeBtn->callback(ChangeBaseColorPaletteCallback);
             
	 }
	 this->end();
    
    } 
    else {
        // Cache the current active index, if any
        for (int j = 0; j < 3; ++j) {
            activeMenuIndex[j] = (intptr_t) m_menus[j]->mvalue()->user_data();
            activeSet[j] = false;
        }
    }

    // Remove any unnecessary menu items
    for (int i = m_structures.size() + 1; i < m_menuItemsSize; ++i) {
        m_menuItems[i].label(0);
    }

    // Reallocate if necessary
    if ((int) m_structures.size() + 2 > m_menuItemsSize) {
        
	m_menuItemsSize = m_structures.size() + 2;
        Free(m_menuItems);
        m_menuItems = (Fl_Menu_Item *) malloc(sizeof(Fl_Menu_Item) * m_menuItemsSize);
        m_menuItems[0].label("None");
        m_menuItems[0].shortcut(0);
        m_menuItems[0].user_data((void *) -1);

        for (int i = 0; i < m_menuItemsSize; ++i) {
            m_menuItems[i].callback((Fl_Callback *) 0);
            m_menuItems[i].labeltype(FL_NORMAL_LABEL);
            m_menuItems[i].labelsize(m_menus[0]->textsize());
            m_menuItems[i].labelcolor(GUI_BTEXT_COLOR);
            m_menuItems[i].labelfont(m_menus[0]->textfont());
            m_menuItems[i].flags = 0;
        }

        m_menus[0]->menu(m_menuItems);
        m_menus[1]->menu(m_menuItems);
        m_menus[2]->menu(m_menuItems);
    
    }

    // Add entries
    StructureManager *structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui) {
        RNAStructure *structure = structureManager->GetStructure(m_structures[ui]);

        m_menuItems[ui + 1].label(structure->GetFilenameNoExtension());
        m_menuItems[ui + 1].user_data((void *) m_structures[ui]);
        m_menuItems[ui + 1].shortcut(0);

        for (int j = 0; j < 3; ++j) {
            if (activeMenuIndex[j] == m_structures[ui]) {
                m_menus[j]->value(ui + 1);
                activeSet[j] = true;
            }
        }
    }

    // Reset active entries that have not already been set, and set the last 
    // entry to NULL label
    m_menuItems[m_structures.size() + 1].label(0);
    for (int j = 0; j < 3; ++j) {
        if (!activeSet[j]) {
            m_menus[j]->value(m_menuItems);
        }
    }

}

void DiagramWindow::MenuCallback(Fl_Widget *widget, void *userData) {
    // make sure the diagram is drawn right away:
    DiagramWindow *window = (DiagramWindow *) widget->parent();
    window->ResetWindow(false);
    window->m_redrawStructures = true;
    window->redraw();
}

void DiagramWindow::ChangeBaseColorPaletteCallback(Fl_Widget *btn, void *udata) {
     DisplayConfigWindow *cfgWindow = new DisplayConfigWindow(); 
     cfgWindow->show();
     cfgWindow->redraw();
     while(!cfgWindow->isDone() && cfgWindow->visible()) 
          Fl::wait();
     cfgWindow->hide();
     Delete(cfgWindow, DisplayConfigWindow);
     DiagramWindow *dwin = (DiagramWindow *) btn->parent();
     dwin->m_redrawStructures = true;
     dwin->redraw();
}

int DiagramWindow::handle(int flEvent) {

     switch(flEvent) { 
      case FL_SHOW:
           m_redrawStructures = true;
           redraw();
           Fl::flush();
           return 1;
      case FL_HIDE:
           return 1;
      case FL_PUSH: { // mouse down
           if(Fl::event_x() < GLWIN_TRANSLATEX || 
              Fl::event_y() < (int) 1.25 * GLWIN_TRANSLATEY) {
	        Fl_Cairo_Window::handle(flEvent);
	        return 1;
	   }
	   Fl::lock();
	   DiagramWindow::lastClickX = Fl::event_x();
	   DiagramWindow::lastClickY = Fl::event_y();
	   DiagramWindow::lastClickSequenceLength = sequenceLength;
	   DiagramWindow::currentDiagramWindowInstance = this;
	   //if(!zoomButtonDown) {
	   //     DiagramWindow::currentCursor = FL_CURSOR_HELP;
	   //     this->cursor(FL_CURSOR_HELP);
	   //}
	   Fl::unlock();
	   Fl::remove_timeout(DiagramWindow::DisplayBaseIndexTimerCallback);
	   Fl::add_timeout(0.6, DiagramWindow::DisplayBaseIndexTimerCallback);
	   if(!zoomButtonDown) {
                zoomButtonDown = true;
                initZoomX = Fl::event_x();
                initZoomY = Fl::event_y();
           }
           Fl_Cairo_Window::handle(flEvent);
	   return 1;
      }
      case FL_RELEASE:
           Fl::lock();
	   DiagramWindow::lastClickX = -1;
	   DiagramWindow::lastClickY = -1;
	   Fl::unlock();
	   if(zoomButtonDown) {
                DiagramWindow::currentCursor = DIAGRAMWIN_DEFAULT_CURSOR;
	        this->cursor(DIAGRAMWIN_DEFAULT_CURSOR);
		lastZoomX = Fl::event_x();
                lastZoomY = Fl::event_y();
                zoomButtonDown = false;
                haveZoomBuffer = true;
                HandleUserZoomAction();
           }
	   return 1;
      case FL_DRAG:
           if(zoomButtonDown) {
	       DiagramWindow::currentCursor = FL_CURSOR_MOVE;
	       this->cursor(FL_CURSOR_MOVE);
               lastZoomX = Fl::event_x();
               lastZoomY = Fl::event_y();
               zx0 = initZoomX;
               zy0 = initZoomY;
               zx1 = lastZoomX;
               zy1 = lastZoomY;
               zw = ABS(initZoomX - lastZoomX);
               zh = ABS(initZoomY - lastZoomY);
               haveZoomBuffer = true;
               redraw();
               break;
           }
      case FL_FOCUS:
           Fl_Cairo_Window::handle(flEvent);
           return 1;
      case FL_UNFOCUS:
           Fl_Cairo_Window::handle(flEvent);
           return 1;
      case FL_KEYDOWN: {
                            if(Fl::event_length() == 1 && *(Fl::event_text()) == 'G') { 
                                      if(!haveZoomBuffer || !zoomBufferContainsArc) { 
                                           const char *errMsg = "Select a zoom area containing a displayed arc before trying to view the structure's CT file contents!";
                                           AddNewErrorMessageToDisplay(string(errMsg));
                                           return 1;
                                      }
                            int structIndex = RNAStructure::ActionOpenCTFileViewerWindow(folderIndex, 
                                                         zoomBufferMinArcIndex, zoomBufferMaxArcIndex);
                            if(structIndex < 0) {
                                      return 1;
                            }
                            int minArcPairIndex = MIN(zoomBufferMinArcIndex, zoomBufferMaxArcIndex);
                            if(minArcPairIndex <= 0) {
                                      const char *errMsg = "Invalid arc index bounds selected! Try zooming again.";
                                      AddNewErrorMessageToDisplay(string(errMsg));
                                      return 1;
                            }
                            else if(!RNAStructure::ScrollOpenCTFileViewerWindow(structIndex, minArcPairIndex)) { 
                                      const char *errMsg = "CT view operation failed. Try zooming again?";
                                      AddNewErrorMessageToDisplay(string(errMsg));
                                      return 1;
                            }
                           }
                           else if(Fl::event_length() == 1 && *(Fl::event_text()) == 'R') {
                                 if(ctFileSelectWin != NULL) {
			              Delete(ctFileSelectWin, InputWindow);
				 }
                                 ctFileSelectWin = new InputWindow(400, 175, 
                                                                   "Select Structure File to View ...", "", 
                                                                   InputWindow::RADIAL_LAYOUT_FILE_INPUT, 
                                                                   true, folderIndex);
                            while(ctFileSelectWin->visible()) {
                                 Fl::wait();
                            }
			    ctFileSelectWin->hide();
		            if(ctFileSelectWin->isCanceled() || ctFileSelectWin->getFileSelectionIndex() < 0) {
                                 return 1; 
                            }
                            StructureManager *structManager = RNAStructViz::GetInstance()->GetStructureManager();
                            int ctFileSelectIndex = ctFileSelectWin->getFileSelectionIndex();
			    int structIdx = structManager->GetFolderAt(folderIndex)->
                                                                folderStructs[ctFileSelectIndex];
                            RNAStructure *rnaStruct = structManager->GetStructure(structIdx);
                            const char *rnaSeqStr = rnaStruct->GetSequenceString();
                            size_t seqStartPos = (zoomBufferMinArcIndex > 0) ? zoomBufferMinArcIndex - 1 : 0;
                            size_t seqEndPos = (zoomBufferMaxArcIndex > 0) ? zoomBufferMaxArcIndex - 1 : MAX_SIZET;
                            // we have to include all pairs of the clipped / highlighted radial view (increase as needed):
                            size_t nextStartPos = seqStartPos, nextEndPos = seqEndPos;
                            for(int pos = seqStartPos; pos <= seqEndPos; pos++) {
                                      if(rnaStruct->GetBaseAt(pos)->m_pair != RNAStructure::UNPAIRED) {
                                                int pairIndex = rnaStruct->GetBaseAt(pos)->m_pair;
                                                nextStartPos = MIN(nextStartPos, pairIndex);
                                                nextEndPos = MAX(nextEndPos, pairIndex);
                                      }
                            }
                            seqStartPos = nextStartPos;
                            seqEndPos = nextEndPos;
                            if(seqEndPos - seqStartPos + 1 > RadialLayoutDisplayWindow::MAX_SEQUENCE_DISPLAY_LENGTH) {
                                      char numericBuf[MAX_BUFFER_SIZE];
                                      snprintf(numericBuf, MAX_BUFFER_SIZE, "%d\0", 
                                      RadialLayoutDisplayWindow::MAX_SEQUENCE_DISPLAY_LENGTH);
                                      string errMsg = string("DISABLED FEATURE: We only support radial layout ") + 
                                                                          string("for sequences <= ") + string(numericBuf) + 
                                                                          string(" bases (currently ") + 
                                                                          std::to_string(seqStartPos) + 
                                                                          string(" to ") + std::to_string(seqEndPos) + 
                                                                          string(")."); 
                                      AddNewErrorMessageToDisplay(errMsg);
                                      return 1;
                            }
                            radialDisplayWindow = new RadialLayoutDisplayWindow();
                            radialDisplayWindow->SetTitleFormat(
                                                         "Radial Display for %s -- Highlighting Bases #%d to #%d", 
                                                         rnaStruct->GetFilenameNoExtension(), 
                                                         seqStartPos + 1, seqEndPos + 1);
                                                         radialDisplayWindow->SetStructureCTFileName(rnaStruct->GetFilename()
	                    );
                            radialDisplayWindow->SetStructureFolderName(structManager->GetFolderAt(folderIndex)->folderName);
                            radialDisplayWindow->SetParentWindow(this);
                            radialDisplayWindow->DisplayRadialDiagram(rnaSeqStr, seqStartPos, seqEndPos, rnaStruct->GetLength());
                            radialDisplayWindow->show();
                     }
                     return 1;
           }
           default:
                return Fl_Cairo_Window::handle(flEvent);
     }
     return 0;
}

bool DiagramWindow::ParseZoomSelectionArcIndices() {
     
     if(!haveZoomBuffer || zh <= 0 || zw <= 0) {
          return false;
     }
     
     int bddCircCenterX = GLWIN_TRANSLATEX + IMAGE_WIDTH / 2;
     int bddCircCenterY = GLWIN_TRANSLATEY + IMAGE_HEIGHT / 2;
     int bddCircRadius = DIAGRAM_WIDTH / 2 - 5.f;

     // now find the points of intersection so we can determine which 
     // pair indices they correspond to:
     // (1) Intersection with lower horizontal box line;
     // (2) Intersection with LHS vertical box line;
     // (3) Intersection with upper horizontal box line;
     // (4) Intersection with RHS vertical box line.
     int horizLineConsts[2] = { zy0 + zh, zy0 };
     int vertLineConsts[2] = { zx0, zx0 + zw };
     int x0 = bddCircCenterX, y0 = bddCircCenterY, term;
     int radiusR = Square(bddCircRadius);
     vector<Point_t> matchingArcPoints;
     Point_t pointStruct;
     for(int idx = 0; idx < 2; idx++) { 

      int hlineC = horizLineConsts[idx];
      int hlineSqrtTerm = (int) sqrt(abs(radiusR - Square(hlineC - y0)));
      bool alreadyHitPoint = false;
      term = x0 - hlineSqrtTerm;
      if(term >= 0 && term >= zx0 && term <= zx0 + zw) { 
               pointStruct.x = term;
           pointStruct.y = hlineC;
           matchingArcPoints.push_back(pointStruct);
           alreadyHitPoint = true;
      }
      term = x0 + hlineSqrtTerm;
      if(term >= zx0 && term <= zx0 + zw) { 
               pointStruct.x = term;
           pointStruct.y = hlineC;
           matchingArcPoints.push_back(pointStruct);
      }
      int vlineD = vertLineConsts[idx];
      int vlineSqrtTerm = (int) sqrt(abs(radiusR - Square(vlineD - x0)));
      alreadyHitPoint = false;
      term = y0 - vlineSqrtTerm;
      if(term >= 0 && term >= zy0 && term <= zy0 + zh) { 
           pointStruct.x = vlineD;
               pointStruct.y = term;
           matchingArcPoints.push_back(pointStruct);
           alreadyHitPoint = true;
      }
      term = y0 + vlineSqrtTerm;
      if(term >= zy0 && term <= zy0 + zh) { 
           pointStruct.x = vlineD;
               pointStruct.y = term;
           matchingArcPoints.push_back(pointStruct);
      }

     }
     if(matchingArcPoints.size() == 0) {
          return false;
     }

     vector<int> matchingBasePairs;
     for(int vidx = 0; vidx < matchingArcPoints.size(); vidx++) { 
          double pointTheta = atan2(y0 - matchingArcPoints[vidx].y, matchingArcPoints[vidx].x - x0);
      if(pointTheta <= 0) {
           pointTheta += 2.0 * M_PI;
      }
      pointTheta = 3.0 * M_PI_2 - pointTheta;
      if(pointTheta <= 0) {
               pointTheta += 2.0 * M_PI;
      }
      double pairIdxPct = (double) (pointTheta / 2.0 / M_PI);
      int closestIndex = MAX(1, (int) (pairIdxPct * sequenceLength));
      matchingBasePairs.push_back(closestIndex);
     }
     zoomBufferMinArcIndex = *(min_element(matchingBasePairs.begin(), matchingBasePairs.end()));
     zoomBufferMaxArcIndex = *(max_element(matchingBasePairs.begin(), matchingBasePairs.end()));

     return (zoomBufferMinArcIndex > 0) && (zoomBufferMaxArcIndex > 0);

}

void DiagramWindow::RedrawCairoZoomBuffer(cairo_t *curWinContext) {
    
    // draw the zomm buffer onto the lower right corner of the window:
    int zoomBufXPos = this->w() - ZOOM_WIDTH;
    int zoomBufYPos = this->h() - ZOOM_HEIGHT;
    cairo_set_source_surface(curWinContext, cairo_get_target(crZoom), zoomBufXPos, zoomBufYPos);
    cairo_rectangle(curWinContext, zoomBufXPos, zoomBufYPos, 
            ZOOM_WIDTH, ZOOM_HEIGHT);    
    cairo_fill(curWinContext);
    
    // now draw the frame around the zoom buffer:
    const int ZOOM_SUBWIN_BORDER_WIDTH = 6;
    SetCairoColor(curWinContext, CairoColorSpec_t::CR_BLACK);
    cairo_set_line_cap(curWinContext, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(curWinContext, ZOOM_SUBWIN_BORDER_WIDTH);
    cairo_set_line_join(curWinContext, CAIRO_LINE_JOIN_ROUND);
    cairo_rectangle(curWinContext, zoomBufXPos - ZOOM_SUBWIN_BORDER_WIDTH / 2, 
            zoomBufYPos - ZOOM_SUBWIN_BORDER_WIDTH / 2, 
            ZOOM_WIDTH, ZOOM_HEIGHT);
    cairo_stroke(curWinContext);

    SetCairoColor(curWinContext, CairoColorSpec_t::CR_BLACK);
    cairo_select_font_face(curWinContext, "Courier New", 
                   CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(curWinContext, CairoContext_t::FONT_SIZE_HEADER);
    cairo_move_to(curWinContext, zoomBufXPos, zoomBufYPos - 35);
    cairo_show_text(curWinContext, "Drag Mouse to Zoom:");
    cairo_set_font_size(curWinContext, CairoContext_t::FONT_SIZE_SUBHEADER);
    cairo_move_to(curWinContext, zoomBufXPos, zoomBufYPos - 20);
    cairo_show_text(curWinContext, "<SHIFT+G>   : CT View");
    cairo_move_to(curWinContext, zoomBufXPos, zoomBufYPos - 8);
    cairo_show_text(curWinContext, "<SHIFT+R>   : Radial Views");


    // now draw the zoom selection area of the window:
    if(haveZoomBuffer && zw > 0 && zh > 0) {
    cairo_set_line_width(curWinContext, 2);
        SetCairoColor(curWinContext, CairoColorSpec_t::CR_SOLID_BLACK);
    const double boxDashPattern[] = {6.0, 6.0};
    cairo_set_dash(curWinContext, boxDashPattern, 2, 0.0);
    cairo_rectangle(curWinContext, zx0, zy0, zw, zh);
        cairo_stroke(curWinContext);
    }

}

void DiagramWindow::HandleUserZoomAction() {
     
    zw = ABS(initZoomX - lastZoomX);
    zh = ABS(initZoomY - lastZoomY);
    zx0 = initZoomX; zx1 = lastZoomX; zy0 = initZoomY; zy1 = lastZoomY;
    if(initZoomX <= lastZoomX && initZoomY > lastZoomY) { 
        zx0 = initZoomX;
    zy0 = lastZoomY;
    zx1 = lastZoomX;
    zy1 = initZoomY;
    }
    else if(initZoomX > lastZoomX && initZoomY > lastZoomY) {
        zx0 = lastZoomX;
    zy0 = lastZoomY;
        zx1 = initZoomX;
    zy1 = initZoomY;
    }
    else if(initZoomX > lastZoomX && initZoomY <= lastZoomY) {
        zx0 = lastZoomX;
        zy0 = initZoomY;
        zx1 = initZoomX;
        zy1 = lastZoomY;
    }
    
    if(zx0 != zx1 && zy0 != zy1) { 
        zoomBufferContainsArc = ParseZoomSelectionArcIndices();
         cairo_identity_matrix(crZoom);
         cairo_set_source_rgb(crZoom, 
                              GetRed(GUI_WINDOW_BGCOLOR) / 255.0f, 
                              GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
                              GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
         cairo_rectangle(crZoom, 0, 0, ZOOM_WIDTH, ZOOM_HEIGHT);
         cairo_fill(crZoom);
         int copyWidth = MIN(ZOOM_WIDTH, zw);
         int copyHeight = MIN(ZOOM_HEIGHT, zh);
         double contextScaleX = (double) ZOOM_WIDTH / copyWidth;
         double contextScaleY = (double) ZOOM_HEIGHT / copyHeight;
         cairo_scale(crZoom, contextScaleX, contextScaleY);
         cairo_surface_flush(cairo_get_target(crDraw));
         cairo_set_source_surface(crZoom, cairo_get_target(crDraw), 
                             -1 * (zx0 - GLWIN_TRANSLATEX), 
                             -1 * (zy0 - GLWIN_TRANSLATEY));
         cairo_rectangle(crZoom, 0, 0, copyWidth, copyHeight);    
         cairo_fill(crZoom);
    }
    redraw();

}

void DiagramWindow::RedrawStrandEdgeMarker(cairo_t *curWinContext) { 
     
     if(curWinContext == NULL) {
          return;
     }
     // __Give the 5' | 3' distinction note below the diagram:__
     // NOTE: to mark this distinction and avoid uglier artifacts 
     // around the curvature of the circle drawing with pure cairo, 
     // we have just constructed the raw bits for a custom 
     // image we have constructed to distinguish this marker. 
     // Here, we just load up these C-struct-style formatted bits 
     // (with transparency) and copy them over top of the image:
     unsigned int markerImageWidth = FivePrimeThreePrimeEdgeMarker.width;
     unsigned int markerImageHeight = FivePrimeThreePrimeEdgeMarker.height;
     int markerImageStride = cairo_format_stride_for_width(
                                  CAIRO_FORMAT_ARGB32, 
                                  markerImageWidth
                             );
     cairo_surface_t *strandEdgeMarkerSurface = cairo_image_surface_create_for_data(
                  FivePrimeThreePrimeEdgeMarker.pixel_data, 
                  CAIRO_FORMAT_ARGB32, 
                  markerImageWidth, 
                  markerImageHeight, 
                  markerImageStride
              );
     unsigned int markerImageDrawX = (IMAGE_WIDTH - markerImageWidth) / 2;
     unsigned int markerImageDrawY = (IMAGE_HEIGHT + DIAGRAM_HEIGHT) / 2;
     cairo_reset_clip(curWinContext);
     SetCairoColor(curWinContext, CairoColorSpec_t::CR_TRANSPARENT);
     cairo_set_source_surface(curWinContext, strandEdgeMarkerSurface, 
                      GLWIN_TRANSLATEX + markerImageDrawX, 
                      GLWIN_TRANSLATEY + markerImageDrawY);
     cairo_paint_with_alpha(curWinContext, 0.8f);
     cairo_surface_flush(strandEdgeMarkerSurface);
     cairo_surface_flush(cairo_get_target(curWinContext));
     cairo_surface_destroy(strandEdgeMarkerSurface); strandEdgeMarkerSurface = NULL;

}

void DiagramWindow::RedrawStructureTickMarks(cairo_t *curWinContext) {

     if(curWinContext == NULL || m_structures.size() == 0 || !showPlotTickMarks) {
          return;
     }
     
     int firstStructIndex = m_structures[0];
     size_t totalNumTicks = RNAStructViz::GetInstance()->GetStructureManager()->
                                          GetStructure(firstStructIndex)->GetLength();
     size_t numTicks = MIN(totalNumTicks, DWINARC_MAX_TICKS) + 1;
     double DWINARC_LABEL_PCT2 = 1.0 / numTicks;
     double arcOriginX = IMAGE_WIDTH / 2, arcOriginY = IMAGE_HEIGHT / 2; 
     double arcRadius = DIAGRAM_WIDTH / 2, arcRadiusDelta = 2, arcRadiusTextDelta = 4;
     double tickInsetLength = 2;
     char numericLabelStr[MAX_BUFFER_SIZE + 1];
     numericLabelStr[MAX_BUFFER_SIZE] = '\0';
     double tlabelOffset = (double) numTicks / totalNumTicks;

     cairo_set_line_cap(curWinContext, CAIRO_LINE_CAP_ROUND);
     cairo_set_line_width(curWinContext, 2);
     cairo_select_font_face(curWinContext, "Courier New", CAIRO_FONT_SLANT_NORMAL, 
                            CAIRO_FONT_WEIGHT_BOLD);
     cairo_set_font_size(curWinContext, 7);
     SetCairoColor(curWinContext, CairoColorSpec_t::CR_LIGHT_GRAY);
     
     for(int t = 0; t < numTicks; t++) {
          double tickAngle = -M_PI_2 - 2.0 * M_PI * ((double) t / numTicks);
          int tickStartX = (int) (arcOriginX + arcRadius * cos(tickAngle));
          int tickStartY = (int) (arcOriginY - arcRadius * sin(tickAngle));
          int tickOuterX = (int) (arcOriginX + (arcRadius + arcRadiusDelta) * cos(tickAngle));
          int tickOuterY = (int) (arcOriginY - (arcRadius + arcRadiusDelta) * sin(tickAngle));
          cairo_move_to(curWinContext, tickStartX, tickStartY);
          cairo_line_to(curWinContext, tickOuterX, tickOuterY);
          cairo_stroke(curWinContext);
          int numericLabel = (int) ceil((totalNumTicks) * (t + tlabelOffset) * DWINARC_LABEL_PCT2);
          snprintf(numericLabelStr, MAX_BUFFER_SIZE, "%d", numericLabel);
          cairo_text_extents_t textDims;
          cairo_text_extents(curWinContext, numericLabelStr, &textDims);
          int textSize = MAX(textDims.width, textDims.height);
          int tickTextX = (int) (arcOriginX + (arcRadius + arcRadiusTextDelta) * cos(tickAngle));
          int tickTextY = (int) (arcOriginY - (arcRadius + arcRadiusTextDelta) * sin(tickAngle));
          while(tickAngle < 0) {
               tickAngle += 2 * M_PI;
          }
          if(0 <= tickAngle && M_PI_2 > tickAngle) { // Q1:
               //tickTextX += textDims.width;
               //tickTextY -= textDims.height;
          }
          else if(M_PI_2 <= tickAngle && M_PI > tickAngle) { // Q2:
               tickTextX -= textDims.width;
               //tickTextY -= textDims.height;
          }
          else if(M_PI <= tickAngle && 3 * M_PI_2 > tickAngle) { // Q3:
               tickTextX -= textDims.width;
               tickTextY += textDims.height;
          }
          else {
               //tickTextX += textDims.width;
               tickTextY += textDims.height;
          }
          cairo_move_to(curWinContext, tickTextX, tickTextY);
          if(t > 0) { 
               cairo_show_text(curWinContext, numericLabelStr);
          }
     }

}

void DiagramWindow::ShowTickMarksCallback(Fl_Widget *cbw, void *udata) { 
     Fl_Check_Button *cbtn = (Fl_Check_Button *) cbw;
     DiagramWindow *dwin = (DiagramWindow *) cbtn->parent();
     dwin->showPlotTickMarks = cbtn->value() ? true : false;
     dwin->m_redrawStructures = true;
     dwin->redraw();
}

void DiagramWindow::DrawBasesCallback(Fl_Widget *cbw, void *udata) {
     DiagramWindow *dwin = (DiagramWindow *) cbw->parent();
     dwin->m_redrawStructures = true;
     dwin->redraw();
}

void DiagramWindow::WarnUserDrawingConflict() {
    if (!userConflictAlerted && m_drawBranchesIndicator != NULL && m_drawBranchesIndicator->value()) {
        fl_message_title("User Warning ... ");
        int turnOff = fl_ask(
                "You are attempting to draw multiple structures with distinct branch coding enabled. \nWe recommend against this due to readability concerns! \nDo you want to turn off branch / domain color coding now?");
        if (turnOff) {
            m_drawBranchesIndicator->value(0);
        }
        userConflictAlerted = true;
    }
}

std::string DiagramWindow::GetExportPNGFilePath() {
    const char *chooserMsg = "Choose a file name for your PNG output image ...";
    const char *fileExtMask = "*.{png,svg,c}";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[MAX_BUFFER_SIZE];
    defaultFilePath[0] = '\0';
    strcpy(defaultFilePath, (char *) PNG_OUTPUT_DIRECTORY);
    size_t dirPathLength = strlen(defaultFilePath);
    if(dirPathLength > 0 && defaultFilePath[dirPathLength - 1] != '/') {
        strcat(defaultFilePath, "/");
    }
    strftime(defaultFilePath + strlen(defaultFilePath), MAX_BUFFER_SIZE - 1, (char *) PNG_OUTPUT_PATH, 
             tmCurrentTime);
    Fl_Native_File_Chooser fileChooser;
    //char *pngOutputDir = (char *) PNG_OUTPUT_DIRECTORY;
    //fileChooser.directory(pngOutputDir);
    fileChooser.title(chooserMsg);
    fileChooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fileChooser.options(Fl_Native_File_Chooser::NEW_FOLDER | 
                        Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    fileChooser.preset_file(defaultFilePath);
    switch(fileChooser.show()) {
        case -1: // ERROR
             fl_alert("Error selecting file path to save PNG image: \"%s\".\n"
		      "If you are receiving a permissions error trying to save "
		      "the image into the directory you have chosen, try again by "
		      "saving the PNG image into a path in your user home directory.", 
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

void DiagramWindow::DisplayBaseIndexTimerCallback(void *udata) {
     Fl::lock();
     int lastClickX = DiagramWindow::lastClickX, lastClickY = DiagramWindow::lastClickY;
     DiagramWindow::lastClickX = -1;
     DiagramWindow::lastClickY = -1;
     //if(DiagramWindow::currentCursor == FL_CURSOR_HELP) {
     //     DiagramWindow::currentCursor = FL_CURSOR_MOVE;
     //	  DiagramWindow::currentDiagramWindowInstance->cursor(FL_CURSOR_MOVE);
     //}
     //else {
     //     DiagramWindow::currentCursor = DIAGRAMWIN_DEFAULT_CURSOR;
     //	  DiagramWindow::currentDiagramWindowInstance->cursor(DIAGRAMWIN_DEFAULT_CURSOR);
     //}
     Fl::unlock();
     if(lastClickX > 0 && lastClickY > 0) {
	   int bcx, bcy, bcBaseIdx;
	   if(DiagramWindow::GetMouseoverArcDiagramBoundingCircle(lastClickX, lastClickY, bcBaseIdx, bcx, bcy)) {
		int baseIdxLower = MAX(0, bcBaseIdx + 1 - DiagramWindow::lastClickSequenceLength * DWINARC_LABEL_PCT / 2);
		int baseIdxUpper = MIN(DiagramWindow::lastClickSequenceLength, 
				       lastClickSequenceLength * DWINARC_LABEL_PCT / 2 + bcBaseIdx + 1);
		Fl::lock();
		sprintf(DiagramWindow::m_baseIndexTooltipLabel, "Base Index: [%d, %d]", baseIdxLower, baseIdxUpper);
		Fl::unlock();
		Fl::belowmouse(DiagramWindow::currentDiagramWindowInstance);
		Fl_Tooltip::enter_area(DiagramWindow::currentDiagramWindowInstance, 
				       0, bcy, 0, 0, DiagramWindow::m_baseIndexTooltipLabel);
		TerminalText::PrintInfo("Identified target base index as #%d\n", bcBaseIdx + 1);
	   }
     }
}

void DiagramWindow::setAsCurrentDiagramWindow() const {
     DiagramWindow::currentDiagramWindowInstance = (DiagramWindow *) this;
}

void DiagramWindow::RedrawWidgetsTimerCallback(void *udata) {
     DiagramWindow *dwin = (DiagramWindow *) DiagramWindow::currentDiagramWindowInstance;
     if(dwin != NULL && dwin->visible() && dwin->shown()) {
          dwin->drawWidgets(NULL);
          dwin->redraw();
     }
     Fl::repeat_timeout(DWIN_REDRAW_REFRESH, DiagramWindow::RedrawWidgetsTimerCallback);
}

void DiagramWindow::AddNewErrorMessageToDisplay(string errorMsg, float callbackTime) {
     Fl::lock();
     errorMsgQueue.push_back(errorMsg);
     Fl::unlock();
     Fl::add_timeout(callbackTime, DisplayErrorDialogTimerCallback);
}

void DiagramWindow::DisplayErrorDialogTimerCallback(void *handlerRef) {
     if(errorAlertDisplayShown) {
          Fl::add_timeout(0.5, DisplayErrorDialogTimerCallback);
          return;
     }
     Fl::lock();
     string errorMsg = errorMsgQueue.back();
     errorMsgQueue.pop_back();
     Fl::unlock();
     TerminalText::PrintError("%s\n", errorMsg.c_str());
     Fl::lock();
     errorAlertDisplayShown = true;
     Fl::unlock();
     fl_alert("%s", errorMsg.c_str());
     Fl::lock();
     errorAlertDisplayShown = false;
     Fl::unlock();
}
