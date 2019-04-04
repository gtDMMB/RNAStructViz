#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Enumerations.H>
#include <FL/names.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

#include "DiagramWindow.h"
#include "RNAStructViz.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "ThemesConfig.h"
#include "CairoDrawingUtils.h"
#include "DisplayConfigWindow.h"

#include "pixmaps/FivePrimeThreePrimeStrandEdgesMarker.c"
#include "pixmaps/BaseColorPaletteButtonImage.c"

#include <FL/x.H>
#ifdef __APPLE__
     #include <cairo-quartz.h>
#else
     #include <cairo-xlib.h>
#endif

const int DiagramWindow::ms_menu_minx[3] = {5, 205, 405};
const int DiagramWindow::ms_menu_width = 190;

volatile DiagramWindow * DiagramWindow::currentDiagramWindowInstance = NULL;
bool DiagramWindow::redrawRefreshTimerSet = false;

void DiagramWindow::Construct(int w, int h, const std::vector<int> &structures) {

    pixelWidth = 1; //2;
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

    Fl::visual(FL_RGB | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE);
    default_cursor(DIAGRAMWIN_DEFAULT_CURSOR);
    cursor(DIAGRAMWIN_DEFAULT_CURSOR);
    //set_non_modal();

    //colors the top of the Diagram window where structures are chosen
    color(GUI_WINDOW_BGCOLOR);
    fl_rectf(0, 0, w, h);
    size_range(w, h, w, h);
    box(FL_NO_BOX);
    setAsCurrentDiagramWindow();
    if(!redrawRefreshTimerSet) { 
        Fl::add_timeout(DWIN_REDRAW_REFRESH, DiagramWindow::RedrawWidgetsTimerCallback);
        redrawRefreshTimerSet = true;
    }

    title = (char *) malloc(sizeof(char) * 64);
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
    redraw();
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
    delete m_drawBranchesIndicator;
    free(m_menuItems);
    if(imageData != NULL) {
        delete imageData;
    }
    if(crSurface != NULL) {
        cairo_surface_destroy(crSurface);
	cairo_destroy(crDraw);
	cairo_surface_destroy(crZoomSurface);
        cairo_destroy(crZoom);
	cairo_surface_destroy(crBasePairsSurface);
	cairo_destroy(crBasePairsOverlay);
    }
    Delete(m_drawBranchesIndicator);
    Delete(m_cbShowTicks);
    Delete(m_cbDrawBases);
    Delete(baseColorPaletteImg);
    Delete(baseColorPaletteImgBtn);
    Delete(baseColorPaletteChangeBtn);
}

void DiagramWindow::SetFolderIndex(int folderIndex) {
    this->structureFolderIndex = this->folderIndex = folderIndex;
    struct Folder *dwinFolder = RNAStructViz::GetInstance()->
	                        GetStructureManager()->
                                GetFolderAt(folderIndex);
    const char *structureNameFull = dwinFolder->folderName;
    RNAStructure *rnaStruct = RNAStructViz::GetInstance()->GetStructureManager()->
			      GetStructure(folderIndex);
    int basePairCount = (rnaStruct != NULL) ? rnaStruct->GetLength() : 0;
    this->sequenceLength = basePairCount;
    sprintf(title, "Comparison of Arc Diagrams: %-.48s  -- % 5d Bases", 
	    structureNameFull, basePairCount);
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
        m_drawBranchesIndicator->clear();
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
    if (buttonPressed->changed()) {
        DiagramWindow *thisWindow = (DiagramWindow *) buttonPressed->parent();
	bool userImageSaved = false;
	std::string exportFilePath = thisWindow->GetExportPNGFilePath();
	if(exportFilePath.size() == 0) {
	     buttonPressed->clear_changed();     
             return;
	}
	cairo_surface_t *pngInitSource = cairo_get_target(thisWindow->crDraw);
        cairo_surface_t *pngSource = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
		                     IMAGE_WIDTH, IMAGE_HEIGHT + STRAND_MARKER_IMAGE_HEIGHT + 
				     PNG_FOOTER_HEIGHT);
	cairo_t *crImageOutput = cairo_create(pngSource);
        thisWindow->SetCairoColor(crImageOutput, CairoColorSpec_t::CR_TRANSPARENT);
	cairo_rectangle(crImageOutput, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT + 
			STRAND_MARKER_IMAGE_HEIGHT + PNG_FOOTER_HEIGHT);
	cairo_fill(crImageOutput);
	thisWindow->RedrawStrandEdgeMarker(crImageOutput);
	// now draw the footer data:
        thisWindow->SetCairoColor(crImageOutput, CairoColorSpec_t::CR_SOLID_WHITE);
	cairo_rectangle(crImageOutput, 0, IMAGE_HEIGHT + STRAND_MARKER_IMAGE_HEIGHT, 
			IMAGE_WIDTH, PNG_FOOTER_HEIGHT);
	cairo_fill(crImageOutput);
	thisWindow->SetCairoColor(crImageOutput, CairoColorSpec_t::CR_SOLID_BLACK);
	cairo_set_line_width(crImageOutput, 2);
	cairo_move_to(crImageOutput, 0, IMAGE_HEIGHT + STRAND_MARKER_IMAGE_HEIGHT); 
	cairo_line_to(crImageOutput, IMAGE_WIDTH, IMAGE_HEIGHT + STRAND_MARKER_IMAGE_HEIGHT);
	cairo_stroke(crImageOutput);
	cairo_select_font_face(crImageOutput, "Courier New", 
		               CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(crImageOutput, 12);
	int offsetY = IMAGE_HEIGHT  + STRAND_MARKER_IMAGE_HEIGHT + 25;
	for(int s = 0; s < 3; s++) {
             char curStructLabel[MAX_BUFFER_SIZE];
	     int menuChoiceIdx = thisWindow->m_menus[s]->value();
	     const char *curStructName = thisWindow->m_menus[s]->menu()[menuChoiceIdx].label();
	     snprintf(curStructLabel, MAX_BUFFER_SIZE - 1, 
		      " * Structure %d:     %s", 
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
	cairo_set_source_surface(crImageOutput, pngInitSource, 0, 0);
        cairo_rectangle(crImageOutput, 0, 0, IMAGE_HEIGHT, IMAGE_WIDTH + 
			STRAND_MARKER_IMAGE_HEIGHT);
        cairo_fill(crImageOutput);
	if(cairo_surface_write_to_png(pngSource, exportFilePath.c_str()) != 
			              CAIRO_STATUS_SUCCESS) {
             fl_alert("ERROR WRITING PNG TO FILE: %s\n", strerror(errno));
	}
        buttonPressed->clear_changed();
        thisWindow->redraw();
    }
}

void DiagramWindow::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
}

void DiagramWindow::drawWidgets(bool fillWin = true) {
    
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
    if(fillWin) {
         Fl_Color priorColor = fl_color();
	 fl_color(GUI_WINDOW_BGCOLOR);
    	 fl_rectf(0, 0, w(), h());
         fl_color(priorColor);
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
            } else if (sequences[j]->GetLength() <= 1000 && 
                       sequences[j]->GetLength() >= 500) {
                pixelWidth = 2;
            } else {
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
                ComputeNumPairs(sequences, 2);
                *keyA = 1; *keyB = 2;
                *numToDraw = 2;
            } else {
                sequences[0] = sequences[1];
                ComputeNumPairs(sequences, 1);
                *keyA = 1;
                *numToDraw = 1;
            }
        } else {
            if (sequences[2]) {
                sequences[0] = sequences[2];
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

void DiagramWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr) {

    DiagramWindow *thisWindow = (DiagramWindow *) thisCairoWindow;
    thisWindow->drawWidgets(true);

    Fl_Color priorColor = fl_color();
    int priorFont = fl_font();
    int priorFontSize = fl_size();
    
    RNAStructure *sequences[3];
    int numToDraw, keyA, keyB;
    thisWindow->computeDrawKeyParams(sequences, &numToDraw, &keyA, &keyB);
    
    if(numToDraw == 1) {
        thisWindow->DrawKey1(keyA);
    }
    else if(numToDraw == 2) {
        thisWindow->DrawKey2(keyA, keyB);
    }
    else if(numToDraw == 3) {
        thisWindow->DrawKey3();
    }
   
    if (thisWindow->m_redrawStructures) {
	    // __Draw the actual arc diagram pixels and frame 
	    //   them in a circular frame:__ 
	    cairo_identity_matrix(thisWindow->crDraw);
            thisWindow->SetCairoToFLColor(thisWindow->crDraw, thisWindow->color());
	    cairo_rectangle(thisWindow->crDraw, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
            cairo_fill(thisWindow->crDraw);
	    thisWindow->SetCairoToFLColor(thisWindow->crBasePairsOverlay, thisWindow->color());
	    cairo_rectangle(thisWindow->crBasePairsOverlay, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
	    cairo_fill(thisWindow->crBasePairsOverlay);
	    cairo_push_group(thisWindow->crDraw);
	    int drawParams[] = { numToDraw, keyA, keyB };
	    thisWindow->RedrawBuffer(thisWindow->crDraw, sequences, drawParams, IMAGE_WIDTH);
	    cairo_pop_group_to_source(thisWindow->crDraw);
            if(thisWindow->m_cbDrawBases->value()) {
	         cairo_save(thisWindow->crDraw);
	         cairo_set_source_surface(thisWindow->crDraw, 
				          cairo_get_target(thisWindow->crBasePairsOverlay), 0, 0);
                 cairo_rectangle(thisWindow->crDraw, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);    
                 cairo_fill(thisWindow->crDraw);
                 cairo_restore(thisWindow->crDraw);
	    }
	    cairo_arc(thisWindow->crDraw, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 
		      IMAGE_WIDTH / 2 - 25.f, 0.0, 2.0 * M_PI);
            cairo_clip(thisWindow->crDraw);
            cairo_paint(thisWindow->crDraw);
	    cairo_reset_clip(thisWindow->crDraw);
	    cairo_arc(thisWindow->crDraw, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 
		      IMAGE_WIDTH / 2 - 25.f, 0.0, 2.0 * M_PI);
            thisWindow->SetCairoColor(thisWindow->crDraw, CairoColorSpec_t::CR_BLACK);
	    cairo_stroke(thisWindow->crDraw);
	    thisWindow->RedrawStructureTickMarks(thisWindow->crDraw);
	    thisWindow->m_redrawStructures = false;
    }
    cairo_set_source_surface(cr, cairo_get_target(thisWindow->crDraw), 
		             GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_rectangle(cr, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY, 
		    IMAGE_WIDTH, IMAGE_HEIGHT);    
    cairo_fill(cr);
    thisWindow->RedrawStrandEdgeMarker(cr);
    thisWindow->RedrawCairoZoomBuffer(cr);
    
    fl_color(priorColor);
    fl_font(priorFont, priorFontSize);
    fl_line_style(0);
    
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

void DiagramWindow::DrawKey3() {
    int yPosn = 55;
    char mystr[10] = "";

    fl_color(FL_BLACK);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    sprintf(mystr, "%d", numPairs[0]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    yPosn += 10;

    fl_color(FL_RED);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[1]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    yPosn += 10;

    fl_color(FL_GREEN);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[2]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    yPosn += 10;

    fl_color(FL_BLUE);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    sprintf(mystr, "%d", numPairs[6]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    yPosn += 10;

    fl_color(fl_rgb_color(255, 200, 0));
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    sprintf(mystr, "%d", numPairs[3]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    yPosn += 10;

    fl_color(FL_MAGENTA);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    sprintf(mystr, "%d", numPairs[4]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
    yPosn += 10;

    fl_color(FL_CYAN);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    sprintf(mystr, "%d", numPairs[5]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);

}

void DiagramWindow::DrawKey2(const int a, const int b) {
    int yPosn = 55;
    char mystr[10] = "";

    fl_color(FL_BLACK);
    fl_rectf(m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    fl_rectf(m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
    sprintf(mystr, "%d", numPairs[0]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    yPosn += 10;

    fl_color(FL_RED);
    fl_rectf(m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    sprintf(mystr, "%d", numPairs[1]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);
    yPosn += 10;

    fl_color(FL_GREEN);
    fl_rectf(m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
    sprintf(mystr, "%d", numPairs[2]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, yPosn + 3);

}

void DiagramWindow::DrawKey1(const int a) {
    fl_color(FL_BLACK);
    fl_rectf(m_menus[a]->x(), 55, m_menus[a]->w(), 3);

    char mystr[10] = "";
    sprintf(mystr, "%d", numPairs[0]);
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 10, 55 + 3);
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
    
void DiagramWindow::SetCairoColor(cairo_t *cr, int nextColorFlag) {
    CairoColor_t::GetCairoColor((CairoColorSpec_t) nextColorFlag).ApplyRGBAColor(cr);
}

void DiagramWindow::SetCairoToFLColor(cairo_t *cr, Fl_Color flc) {
     if(cr == NULL) {
          return;
     }
     CairoColor_t::FromFLColorType(flc).SetAlpha(0x99).ApplyRGBAColor(cr);
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
            } else if (baseData1->m_pair == baseData3->m_pair) {
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
            } else {
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

    unsigned int numBases = structures[0]->GetLength();
    if (numStructures == 1) {
        unsigned int counter1 = 0;
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            const RNAStructure::BaseData *baseData1 =
                    structures[0]->GetBaseAt(ui);
            if (baseData1->m_pair != RNAStructure::UNPAIRED &&
                baseData1->m_pair > ui)
                counter1++;
        }
        numPairs[0] = counter1;
    } else if (numStructures == 2) {
        unsigned int counter1 = 0; //black
        unsigned int counter2 = 0; //red
        unsigned int counter3 = 0; //green
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
            if (baseData1->m_pair != RNAStructure::UNPAIRED &&
                baseData1->m_pair > ui) {
                if (baseData1->m_pair == baseData2->m_pair) {
                    counter1++;
                } else {
                    if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                        baseData2->m_pair > ui) {
                        counter3++;
                    }
                    counter2++;
                }
            } else if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                       baseData2->m_pair > ui) {
                counter3++;
            }
        }
        numPairs[0] = counter1;
        numPairs[1] = counter2;
        numPairs[2] = counter3;
    } else if (numStructures == 3) {
        unsigned int counter1 = 0; //black = in all 3 structures
        unsigned int counter2 = 0; //red = in only structure 1
        unsigned int counter3 = 0; //green = in only structure 2 
        unsigned int counter4 = 0; //yellow = in structures 1 & 2
        unsigned int counter5 = 0; //magenta = in structures 1 & 3
        unsigned int counter6 = 0; //cyan = in structures 2 & 3
        unsigned int counter7 = 0; //blue = in only structure 3
        for (unsigned int ui = 0; ui < numBases; ++ui) {
            const RNAStructure::BaseData *baseData1 = structures[0]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
            const RNAStructure::BaseData *baseData3 = structures[2]->GetBaseAt(ui);
            if (baseData1->m_pair != RNAStructure::UNPAIRED &&
                baseData1->m_pair > ui) {
                if (baseData1->m_pair == baseData2->m_pair) {
                    if (baseData1->m_pair == baseData3->m_pair) {
                        counter1++;
                    } else {
                        counter4++;

                        if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                            baseData3->m_pair > ui) {
                            counter7++;
                        }
                    }
                } else if (baseData1->m_pair == baseData3->m_pair) {
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

    if (denom < 0.001) {
        cX = cY = 0.0f;
        r = 0.0f;
    }

    double sq1 = x1 * x1 + y1 * y1;
    double sq2 = x2 * x2 + y2 * y2;
    double sq3 = x3 * x3 + y3 * y3;

    cX = (sq1 * (y2 - y3) - y1 * (sq2 - sq3) + sq2 * y3 - y2 * sq3) / (2.0 *
                                                                       denom);
    cY = (x1 * (sq2 - sq3) - sq1 * (x2 - x3) + sq3 * x2 - x3 * sq2) / (2.0 *
                                                                       denom);

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
    ComputeCircle(xPosn1, yPosn1, xPosn2, yPosn2, xPosn3, yPosn3, arcX, arcY,
                  arcR);

    int boundX = (int) (arcX - arcR);
    int boundY = (int) (arcY - arcR);
    int boundSize = (int) (2.0f * arcR);
    double arc1 = 180.0 / M_PI * atan2(arcY - yPosn1, xPosn1 - arcX);
    double arc2 = 180.0 / M_PI * atan2(arcY - yPosn2, xPosn2 - arcX);

    int boundingBoxCenterX = boundX + boundSize / 2;
    int boundingBoxCenterY = boundY + boundSize / 2;
    float boundingBoxRadius = boundSize / 2.0;
    cairo_set_line_width(cr, pixelWidth);
    
    if (arc2 - arc1 > 180.0)
        arc1 += 360.0;
    if (arc1 - arc2 > 180.0)
        arc2 += 360.0;
    if (arc2 > arc1) {
        cairo_arc(cr, boundingBoxCenterX, boundingBoxCenterY, 
		  boundingBoxRadius, arc1, arc2);
    } else {
        cairo_arc(cr, boundingBoxCenterX, boundingBoxCenterY, 
	          boundingBoxRadius, arc2, arc1);
    }
    cairo_stroke(cr);
}

void DiagramWindow::DrawBase(
        const unsigned int index,
        const RNAStructure::Base base,
        const float centerX,
        const float centerY,
        const float angleBase,
        const float angleDelta,
        const float radius) {
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
    cairo_select_font_face(crBasePairsOverlay, "Courier New", 
		           CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crBasePairsOverlay, CairoContext_t::FONT_SIZE_SMALL);
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
    angleDelta = (M_PI * 2.0f - 0.05f) / (float) numBases;
    angleBase = 1.5f * M_PI - 0.025f;
    centerX = (float) resolution / 2.0f;
    centerY = (float) resolution / 2.0f;
    radius = centerX < centerY ? centerX - 25.f : centerY - 25.f;
}

void DiagramWindow::AddStructure(const int index) {
    if (std::find(m_structures.begin(), m_structures.end(), index) ==
        m_structures.end()) {
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
        label->labelcolor(GUI_TEXT_COLOR);
        label = new Fl_Box(ms_menu_minx[1], 0, ms_menu_width, 25, "Structure 2");
        label->labelcolor(GUI_TEXT_COLOR);
        label = new Fl_Box(ms_menu_minx[2], 0, ms_menu_width, 25, "Structure 3");
        label->labelcolor(GUI_TEXT_COLOR);
        for(int m = 0; m < 3; m++) { 
	    m_menus[m] = new Fl_Choice(ms_menu_minx[m], 25, ms_menu_width, 25);
            m_menus[m]->callback(MenuCallback);
	    m_menus[m]->labelcolor(GUI_BTEXT_COLOR);
	    m_menus[m]->textcolor(GUI_BTEXT_COLOR);
            m_menus[m]->selection_color(Lighter(GUI_BTEXT_COLOR, 0.5f));
	    m_menus[m]->activate();
	    activeMenuIndex[m] = -1;
            activeSet[m] = false;
	}
        int horizCheckBoxPos = ms_menu_minx[2] + ms_menu_width + 2 * WIDGET_SPACING;
        m_drawBranchesIndicator = new Fl_Check_Button(horizCheckBoxPos, 5, 
			          20, 20,"Draw (16S) Domains");
        m_drawBranchesIndicator->callback(checkBoxChangedStateCallback, 
			         m_drawBranchesIndicator);
        m_drawBranchesIndicator->tooltip(
                "Whether to color code the four domains in 16S structures");
        m_drawBranchesIndicator->labelcolor(GUI_TEXT_COLOR);
	if(!PERFORM_BRANCH_TYPE_ID) {
	     m_drawBranchesIndicator->hide(); // make this option invisible
	}

	if(exportButton == NULL || m_cbShowTicks == NULL || baseColorPaletteImg == NULL) { 
	     
	     int offsetY = 25;
             exportButton = new Fl_Button(horizCheckBoxPos, offsetY, 
	                	          EXPORT_BUTTON_WIDTH, 25, "@filesaveas   Export PNG");
             exportButton->type(FL_NORMAL_BUTTON);
             exportButton->callback(exportToPNGButtonPressHandler, exportButton);
             exportButton->labelcolor(GUI_BTEXT_COLOR);
	     offsetY += 25;

	     m_cbShowTicks = new Fl_Check_Button(horizCheckBoxPos + 4, offsetY, 
			                         EXPORT_BUTTON_WIDTH, 25, 
					         "Draw Ticks");
	     m_cbShowTicks->callback(ShowTickMarksCallback);
	     m_cbShowTicks->type(FL_TOGGLE_BUTTON);
	     m_cbShowTicks->labelcolor(GUI_TEXT_COLOR);
	     m_cbShowTicks->selection_color(GUI_BTEXT_COLOR); // checkmark color
	     m_cbShowTicks->value(showPlotTickMarks);
	     offsetY += 25;
             
	     m_cbDrawBases = new Fl_Check_Button(horizCheckBoxPos + 4, offsetY, 
	     		                         EXPORT_BUTTON_WIDTH, 25, 
	     				         "Draw Bases");
	     m_cbDrawBases->callback(DrawBasesCallback);
	     m_cbDrawBases->type(FL_TOGGLE_BUTTON);
	     m_cbDrawBases->labelcolor(GUI_TEXT_COLOR);
	     m_cbDrawBases->selection_color(GUI_BTEXT_COLOR); // checkmark color
	     m_cbDrawBases->value(0);
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
             baseColorPaletteImgBtn->align(FL_ALIGN_IMAGE_BACKDROP | 
			                   FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
             baseColorPaletteImgBtn->labeltype(_FL_ICON_LABEL);
             baseColorPaletteImgBtn->shortcut(FL_CTRL + 'b');
	     baseColorPaletteImgBtn->box(FL_NO_BOX);
             baseColorPaletteImgBtn->callback(ChangeBaseColorPaletteCallback);
	     baseColorPaletteImgBtn->redraw();
	     offsetY += BaseColorPaletteButtonImage.height + 6;

	     baseColorPaletteChangeBtn = new Fl_Button(horizCheckBoxPos + 4, offsetY, 
			                               EXPORT_BUTTON_WIDTH, 25, 
						       "Set Draw Colors");
	     baseColorPaletteChangeBtn->type(FL_NORMAL_BUTTON);
	     baseColorPaletteChangeBtn->labelcolor(GUI_BTEXT_COLOR);
             baseColorPaletteChangeBtn->callback(ChangeBaseColorPaletteCallback);
	
	}

        this->end();
    } else {
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
        free(m_menuItems);
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
    StructureManager *structureManager =
            RNAStructViz::GetInstance()->GetStructureManager();
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui) {
        RNAStructure *structure = structureManager->GetStructure(m_structures[ui]);

        m_menuItems[ui + 1].label(structure->GetFilename());
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
     delete cfgWindow;
     DiagramWindow *dwin = (DiagramWindow *) btn->parent();
     dwin->m_redrawStructures = true;
     dwin->redraw();
}

int DiagramWindow::handle(int flEvent) {

     switch(flEvent) { 
	  case FL_SHOW:
	       make_current();
	       m_redrawStructures = true;
               redraw();
	       Fl::flush();
	       return 1;
	  case FL_HIDE:
	       return 1;
	  case FL_PUSH: // mouse down
               if(!zoomButtonDown && Fl::event_x() >= GLWIN_TRANSLATEX && 
	          Fl::event_y() >= (int) 1.25 * GLWIN_TRANSLATEY) {
	            zoomButtonDown = true;
		    initZoomX = Fl::event_x();
		    initZoomY = Fl::event_y();
		    this->cursor(FL_CURSOR_MOVE);
		    Fl_Cairo_Window::handle(flEvent);
		    return 1;
	       }
	  case FL_RELEASE:
	       if(zoomButtonDown) {
		    lastZoomX = Fl::event_x();
		    lastZoomY = Fl::event_y();
		    this->cursor(DIAGRAMWIN_DEFAULT_CURSOR);
		    zoomButtonDown = false;
		    haveZoomBuffer = true;
		    HandleUserZoomAction();
	       }
	  case FL_DRAG:
	       if(zoomButtonDown) {
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
	  case FL_KEYDOWN:
	       {
	            if(Fl::event_length() == 1 && *(Fl::event_text()) == 'G') { 
		         if(!haveZoomBuffer || !zoomBufferContainsArc) { 
		              fprintf(stderr, 
				      "Select a zoom area containing a displayed arc "
			      	      "before trying to view the structure's "
			      	      "CT file contents!\n");
			      return 1;
			 }
			 int structIndex = RNAStructure::ActionOpenCTFileViewerWindow(folderIndex, 
				           zoomBufferMinArcIndex, zoomBufferMaxArcIndex);
			 if(structIndex < 0) {
			      return 1;
			 }
			 int minArcPairIndex = MIN(zoomBufferMinArcIndex, zoomBufferMaxArcIndex);
			 if(minArcPairIndex <= 0) {
			      fprintf(stderr, "Invalid arc index bounds selected! Try zooming again.\n");
			      return 1;
			 }
			 else if(!RNAStructure::ScrollOpenCTFileViewerWindow(structIndex, minArcPairIndex)) { 
			      fprintf(stderr, "CT view operation failed. Try zooming again?\n");
			      return 1;
			 }
		    }
		    else if(Fl::event_length() == 1 && *(Fl::event_text()) == 'R') {
		         
			 InputWindow *ctFileSelectWin = new InputWindow(400, 175, 
					                "Select CT File to View ...", "", 
							InputWindow::RADIAL_LAYOUT_FILE_INPUT, 
							folderIndex);
			 while(ctFileSelectWin->visible()) {
		              Fl::wait();
			 }
			 if(ctFileSelectWin->isCanceled() || ctFileSelectWin->getFileSelectionIndex() < 0) {
			      break;
			 }
			 StructureManager *structManager = RNAStructViz::GetInstance()->
				                           GetStructureManager();
		         int ctFileSelectIndex = ctFileSelectWin->getFileSelectionIndex();
			 Delete(ctFileSelectWin);
			 int structIdx = structManager->GetFolderAt(folderIndex)->
				         folderStructs[ctFileSelectIndex];
		         RNAStructure *rnaStruct = structManager->GetStructure(structIdx);
			 const char *rnaSeqStr = rnaStruct->GetSequenceString();
			 size_t seqStartPos = (zoomBufferMinArcIndex > 0) ? zoomBufferMinArcIndex - 1 : 0;
			 size_t seqEndPos = (zoomBufferMaxArcIndex > 0) ? zoomBufferMaxArcIndex - 1 : MAX_SIZET;
			 radialDisplayWindow = new RadialLayoutDisplayWindow();
			 radialDisplayWindow->SetTitleFormat(
			 		      "Radial Display for %s -- Highlighting Bases #%d to #%d", 
			 		      rnaStruct->GetFilenameNoExtension(), 
			 		      seqStartPos + 1, seqEndPos + 1);
                         radialDisplayWindow->SetParentWindow(this);
			 radialDisplayWindow->DisplayRadialDiagram(rnaSeqStr, seqStartPos, seqEndPos);
			 radialDisplayWindow->show();

		    }
		    return 1;
	       }
	  default:
               return Fl_Cairo_Window::handle(flEvent);
     }

}

bool DiagramWindow::ParseZoomSelectionArcIndices() {
     
     if(!haveZoomBuffer || zh <= 0 || zw <= 0) {
          return false;
     }
     
     int bddCircCenterX = GLWIN_TRANSLATEX + IMAGE_WIDTH / 2;
     int bddCircCenterY = GLWIN_TRANSLATEY + IMAGE_HEIGHT / 2;
     int bddCircRadius = IMAGE_WIDTH / 2 - 25.f;

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
    SetCairoColor(curWinContext, CairoColorSpec_t::CR_BLUE);
    cairo_set_line_cap(curWinContext, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(curWinContext, 5);
    cairo_set_line_join(curWinContext, CAIRO_LINE_JOIN_ROUND);
    cairo_rectangle(curWinContext, zoomBufXPos, zoomBufYPos, 
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
     // NOTE: to mark this distinction and avoid "ugly" artifacts 
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
     unsigned int markerImageDrawX = (IMAGE_WIDTH - markerImageWidth + 4) / 2;
     unsigned int markerImageDrawY = IMAGE_HEIGHT - 25.f - 2;
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

double DiagramWindow::TranslateAngleFromUserAxes(double theta) { 
     return theta - M_PI_2;
}

void DiagramWindow::TranslateUTMCoordinates(double *xc, double *yc, double *x0, double *y0) {
     if(yc == NULL || y0 == NULL) { 
          return;
     }	  
     *yc = -1 * (*yc);
}

void DiagramWindow::RedrawStructureTickMarks(cairo_t *curWinContext) {

     if(curWinContext == NULL || m_structures.size() == 0 || !showPlotTickMarks) {
          return;
     }
     
     int firstStructIndex = m_structures[0];
     size_t totalNumTicks = RNAStructViz::GetInstance()->GetStructureManager()->
	                                  GetStructure(firstStructIndex)->GetLength();
     size_t numTicks = MIN(totalNumTicks, DWINARC_MAX_TICKS) + 1;
     int tickLabelMod = MAX((int) (numTicks - 1) * DWINARC_LABEL_PCT, 1), numTickLabels = 0;
     double arcOriginX = IMAGE_WIDTH / 2, arcOriginY = IMAGE_HEIGHT / 2; 
     double arcRadius = IMAGE_WIDTH / 2 - 25.f + 1;
     double tickInsetLength = 2;
     char numericLabelStr[MAX_BUFFER_SIZE + 1];
     numericLabelStr[MAX_BUFFER_SIZE] = '\0';

     cairo_set_line_cap(curWinContext, CAIRO_LINE_CAP_ROUND);
     cairo_set_line_width(curWinContext, 2);
     cairo_translate(curWinContext, arcOriginX, arcOriginY);
     cairo_select_font_face(curWinContext, "Courier New", CAIRO_FONT_SLANT_NORMAL, 
		            CAIRO_FONT_WEIGHT_BOLD);
     cairo_set_font_size(curWinContext, 7);
     SetCairoColor(curWinContext, CairoColorSpec_t::CR_LIGHT_GRAY);
     
     for(int t = 1; t <= numTicks; t++) { 
          
	  double rotationAngle = (double) (-1.0 * (2.0 * t * M_PI) / numTicks);
	  double rotationAngle2 = TranslateAngleFromUserAxes(rotationAngle);
	  double nextStartX = (arcRadius - tickInsetLength) * cos(rotationAngle2);
	  double nextStartY = (arcRadius - tickInsetLength) * sin(rotationAngle2);
          TranslateUTMCoordinates(&nextStartX, &nextStartY, &arcOriginX, &arcOriginY);
	  double nextFinishX = arcRadius * cos(rotationAngle2);
          double nextFinishY = arcRadius * sin(rotationAngle2);
          TranslateUTMCoordinates(&nextFinishX, &nextFinishY, &arcOriginX, &arcOriginY);
	  cairo_move_to(curWinContext, nextStartX, nextStartY);
          cairo_line_to(curWinContext, nextFinishX, nextFinishY);
	  cairo_stroke(curWinContext);
	  if(t % tickLabelMod == 0) { // draw rotated numeric label:

	       while(rotationAngle2 < 0) { 
                    rotationAngle2 += 2.0 * M_PI;
               }
               while(rotationAngle2 >= 2.0 * M_PI) { 
                    rotationAngle2 -= 2.0 * M_PI;
               }
	       numTickLabels++;
	       int numericLabel = (int) (totalNumTicks * numTickLabels * DWINARC_LABEL_PCT);
               snprintf(numericLabelStr, MAX_BUFFER_SIZE, "%d \0", numericLabel);
	       cairo_text_extents_t textDims;
	       cairo_text_extents(curWinContext, numericLabelStr, &textDims);
	       if(rotationAngle2 >= 0 && rotationAngle2 < M_PI_2) { 
		    nextFinishX += textDims.width / 2;
		    nextFinishY -= 0.5 * textDims.height;
	       }
	       else if(rotationAngle2 >= M_PI_2 && rotationAngle2 < M_PI) { 
		    nextFinishX -= textDims.width;
		    nextFinishY -= textDims.height;
	       }
	       else if(rotationAngle2 >= M_PI && rotationAngle2 < 3.0 * M_PI_2) {
		    nextFinishX -= 1.5 * textDims.width;
		    nextFinishY += textDims.height;
	       }
	       else {
		    nextFinishX += textDims.width / 3;
		    nextFinishY += textDims.height;
	       }
	       cairo_move_to(curWinContext, nextFinishX, nextFinishY);
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
    if (!userConflictAlerted && m_drawBranchesIndicator->value()) {
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
    const char *fileExtMask = "*.png";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[MAX_BUFFER_SIZE];
    strftime(defaultFilePath, MAX_BUFFER_SIZE - 1, (char *) PNG_OUTPUT_PATH, 
		              tmCurrentTime);
    Fl_Native_File_Chooser fileChooser;
    fileChooser.title(chooserMsg);
    fileChooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fileChooser.options(Fl_Native_File_Chooser::NEW_FOLDER | 
		        Fl_Native_File_Chooser::SAVEAS_CONFIRM | 
		        Fl_Native_File_Chooser::PREVIEW);
    fileChooser.directory((char *) PNG_OUTPUT_DIRECTORY);
    fileChooser.preset_file(defaultFilePath);
    switch(fileChooser.show()) {
        case -1: // ERROR
             fl_alert("Error selecting file path to save PNG image: \"%s\".\nIf you are receiving a permissions error trying to save the image into the directory you have chosen, try again by saving the PNG image into a path in your user home directory.", fileChooser.errmsg());
	     return string("");
	case 1: // CANCEL
	     return string("");
	default:
	     std::string outfilePath = string(fileChooser.filename());
	     strncpy((char *) PNG_OUTPUT_DIRECTORY, fileChooser.directory(), 
			      MAX_BUFFER_SIZE - 1);
	     ConfigParser::nullTerminateString((char *) PNG_OUTPUT_DIRECTORY);
	     return outfilePath;
    }
}

void DiagramWindow::setAsCurrentDiagramWindow() const {
     DiagramWindow::currentDiagramWindowInstance = (volatile DiagramWindow *) this;
}

void DiagramWindow::RedrawWidgetsTimerCallback(void *udata) {
     DiagramWindow *dwin = (DiagramWindow *) DiagramWindow::currentDiagramWindowInstance;
     if(dwin != NULL && dwin->visible() && dwin->shown()) {
          dwin->drawWidgets(false);
          dwin->redraw();
     }
     Fl::repeat_timeout(DWIN_REDRAW_REFRESH, DiagramWindow::RedrawWidgetsTimerCallback);
}

