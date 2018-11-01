#include "DiagramWindow.h"
#include "RNAStructViz.h"
#include "BranchTypeIdentification.h"
#include "ConfigOptions.h"
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <string.h>
#include <stdio.h>
#include <time.h>

const int DiagramWindow::ms_menu_minx[3] = {5, 205, 405};
const int DiagramWindow::ms_menu_width = 190;

void DiagramWindow::Construct(int w, int h, const std::vector<int> &structures) {

    pixelWidth = 1; //2;

    m_menus[0] = m_menus[1] = m_menus[2] = NULL;
    m_menuItems = 0;
    m_menuItemsSize = 0;
    folderIndex = -1;
    m_drawBranchesIndicator = NULL;
    userConflictAlerted = false;

    Fl::visual(FL_RGB);

    //colors the top of the Diagram window where structures are chosen
    color(GUI_WINDOW_BGCOLOR);
    size_range(w, h);
    box(FL_NO_BOX);

    title = (char *) malloc(sizeof(char) * 64);
    SetStructures(structures);
    
    imageStride = cairo_format_stride_for_width( 
		      CAIRO_FORMAT_ARGB32, IMAGE_WIDTH);
    imageData = new uchar[imageStride * IMAGE_HEIGHT];
    memset(imageData, 0, imageStride * IMAGE_HEIGHT);
    cairo_surface_t *crSurface = cairo_image_surface_create_for_data( 
			imageData, CAIRO_FORMAT_ARGB32, 
                        IMAGE_WIDTH, IMAGE_HEIGHT, imageStride);
    crDraw = cairo_create(crSurface);   
    //Fl::cairo_cc(crDraw, false);
    set_draw_cb(Draw); // cairo
}

DiagramWindow::DiagramWindow(int w, int h, const char *label,
                             const std::vector<int> &structures) 
        : Fl_Cairo_Window(w + WINW_EXTENSION, h), 
          m_redrawStructures(true) {
    copy_label(label);
    Construct(w + WINW_EXTENSION, h, structures);
}

DiagramWindow::DiagramWindow(int x, int y, int w, int h, const char *label,
                             const std::vector<int> &structures)
        : Fl_Cairo_Window(w + WINW_EXTENSION, h), 
          m_redrawStructures(true) {
    copy_label(label);
    resize(x, y, w + WINW_EXTENSION, h);
    Construct(w + WINW_EXTENSION, h, structures);
}

DiagramWindow::~DiagramWindow() {
    delete m_drawBranchesIndicator;
    //cairo_destroy(crDraw);
    free(m_menuItems);
    cairo_surface_destroy(crSurface);
    delete imageData;

}

void DiagramWindow::SetFolderIndex(int index) {
    folderIndex = index;

    sprintf(title, "Diagrams: %-.48s",
            RNAStructViz::GetInstance()->GetStructureManager()->
            GetFolderAt(index)->folderName);
    label(title);
}

void DiagramWindow::ResetWindow(bool resetMenus = true) {
    
    //this->size(IMAGE_WIDTH + 150, IMAGE_HEIGHT + 150);
    if (resetMenus) {
        m_menus[0]->value(0);
        m_menus[1]->value(0);
        m_menus[2]->value(0);
        m_drawBranchesIndicator->clear();
        userConflictAlerted = false;
    }
    redraw();

}

void DiagramWindow::checkBoxChangedStateCallback(Fl_Widget *, void *v) {
    Fl_Check_Button *cbDrawIndicator = (Fl_Check_Button *) v;
    if (cbDrawIndicator->changed()) {
        DiagramWindow *thisWindow = (DiagramWindow *) cbDrawIndicator->parent();
        thisWindow->m_redrawStructures = true;
        cbDrawIndicator->clear_changed();
        thisWindow->redraw();
    }
}

void DiagramWindow::exportToPNGButtonPressHandler(Fl_Widget *, void *v) {
    Fl_Button *buttonPressed = (Fl_Button *) v;
    if (buttonPressed->changed()) {
        
        DiagramWindow *thisWindow = (DiagramWindow *) buttonPressed->parent();
        char *exportFilePath = thisWindow->GetExportPNGFilePath();
        Fl::wait();
        thisWindow->m_redrawStructures = true;
        thisWindow->redraw();
 
        memset(thisWindow->imageData, 0, thisWindow->imageStride * 
			                 IMAGE_HEIGHT);
        thisWindow->m_redrawStructures = true;
        thisWindow->cairoTranslate = false;
        DiagramWindow::Draw((Fl_Cairo_Window *) thisWindow, thisWindow->crDraw);
        cairo_surface_write_to_png(cairo_get_target(thisWindow->crDraw), 
			           exportFilePath);

        buttonPressed->clear_changed();
    
    }
}

void DiagramWindow::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
}

void DiagramWindow::drawWidgets(bool fillWin = true) {
    
    for(int m = 0; m < 3; m++) {
        if(m_menus[m] != NULL) {
	    m_menus[m]->redraw();
	}
    }
    m_redrawStructures = true;

    if(fillWin) {
         Fl_Color priorColor = fl_color();
         fl_color(color());
         //fl_color(GUI_WINDOW_BGCOLOR);
	 fl_rectf(0, 0, w(), h());
         fl_color(priorColor);
         Fl_Double_Window::draw();
    }

}

void DiagramWindow::Draw(Fl_Cairo_Window *thisCairoWindow, cairo_t *cr) {

    DiagramWindow *thisWindow = (DiagramWindow *) thisCairoWindow;
    //thisWindow->crDraw = cr;
    //cairo_reference(thisWindow->crDraw);    
    thisWindow->drawWidgets(true);

    Fl_Color priorColor = fl_color();
    int priorFont = fl_font();
    int priorFontSize = fl_size();
    
    // Get the structures. Be sure the reference structure is first.
    RNAStructure *sequences[3];
    StructureManager *structureManager =
            RNAStructViz::GetInstance()->GetStructureManager();
    for (int j = 0; j < 3; ++j) {
        if ((intptr_t) (thisWindow->m_menuItems[thisWindow->m_menus[j]->value()].user_data()) == -1) {
            sequences[j] = 0;
        } else {
            sequences[j] = structureManager->GetStructure(
                    (intptr_t) (thisWindow->m_menuItems[thisWindow->m_menus[j]->value()].user_data()));
            if (sequences[j]->GetLength() > 1000) {
                thisWindow->pixelWidth = 1;
            } else if (sequences[j]->GetLength() <= 1000 && 
                       sequences[j]->GetLength() >= 500) {
                thisWindow->pixelWidth = 2;
            } else {
                thisWindow->pixelWidth = 3;
            }
        }
    }

    int numToDraw = 0, keyA = 0, keyB = 0;
    if (sequences[0]) {
        if (sequences[1]) {
            if (sequences[2]) {
                numToDraw = 3;
                thisWindow->ComputeNumPairs(sequences, 3);
                thisWindow->DrawKey3();
            } else {
                numToDraw = 2;
                thisWindow->ComputeNumPairs(sequences, 2);
                keyA = 0; keyB = 1;
		thisWindow->DrawKey2(0, 1);
            }
        } else {
            if (sequences[2]) {
                sequences[1] = sequences[2];
                thisWindow->ComputeNumPairs(sequences, 2);
                keyA = 0; keyB = 2;
		thisWindow->DrawKey2(0, 2);
                numToDraw = 2;
            } else {
                thisWindow->ComputeNumPairs(sequences, 1);
                keyA = 0;
		thisWindow->DrawKey1(0);
                numToDraw = 1;
            }
        }
    } else {
        if (sequences[1]) {
            if (sequences[2]) {
                sequences[0] = sequences[1];
                sequences[1] = sequences[2];
                thisWindow->ComputeNumPairs(sequences, 2);
                keyA = 1; keyB = 2;
		thisWindow->DrawKey2(1, 2);
                numToDraw = 2;
            } else {
                sequences[0] = sequences[1];
                thisWindow->ComputeNumPairs(sequences, 1);
                keyA = 1;
		thisWindow->DrawKey1(1);
                numToDraw = 1;
            }
        } else {
            if (sequences[2]) {
                sequences[0] = sequences[2];
                thisWindow->ComputeNumPairs(sequences, 1);
                keyA = 2;
		thisWindow->DrawKey1(2);
                numToDraw = 1;
            } else {
                numToDraw = 0;
            }
        }
    }
    thisWindow->Fl_Double_Window::draw();

    //thisWindow->drawWidgets(false);
    if (thisWindow->m_redrawStructures) {
            cairo_push_group(cr);
            int drawParams[] = { numToDraw, keyA, keyB };
	    if(thisWindow->cairoTranslate)
	        cairo_translate(cr, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
	    thisWindow->RedrawBuffer(cr, sequences, drawParams, IMAGE_WIDTH);
	    cairo_pop_group_to_source(cr);
            if(thisWindow->cairoTranslate)            
                 cairo_translate(cr, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
            cairo_arc(cr, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, IMAGE_WIDTH / 2 - 15.f, 0.0, 2.0 * M_PI);
            cairo_clip(cr);
            cairo_paint(cr);
	    thisWindow->m_redrawStructures = false;
            thisWindow->cairoTranslate = true;
    }

    fl_color(priorColor);
    fl_font(priorFont, priorFontSize);
    fl_line_style(0);
     
    /*thisWindow->drawWidgets(false);
    if(numToDraw == 1) {
	thisWindow->DrawKey1(keyA);
    }
    else if(numToDraw == 2) { 
	thisWindow->DrawKey2(keyA, keyB);
    }
    else if(numToDraw == 3) {
        thisWindow->DrawKey3();
    }*/

}

void DiagramWindow::RedrawBuffer(cairo_t *cr, RNAStructure **structures,
                                 const int *structParams, 
				 const int resolution) {

    int priorFont = fl_font();
    int priorFontSize = fl_size();
    fl_font(priorFont, 10);
    fl_line_style(0);
    
    //cairo_scale(cr, this->w(), this->h());
    cairo_set_source_rgb(cr, 
		         GetRed(GUI_WINDOW_BGCOLOR) / 255.0f, 
			 GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
			 GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
    //CairoRectangle(cr, 0, 0, this->w(), this->h());
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
    fl_draw(mystr, m_menus[2]->x() + m_menus[2]->w() + 40, yPosn + 3);
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
                nextColorFlag = CR_BRANCH1;
                break;
            case BRANCH2:
                nextColorFlag = CR_BRANCH2;
                break;
            case BRANCH3:
                nextColorFlag = CR_BRANCH3;
                break;
            case BRANCH4:
                nextColorFlag = CR_BRANCH4;
                break;
            default:
                break;
        }
    }
    SetCairoColor(cr, nextColorFlag);

}
    
void DiagramWindow::SetCairoColor(cairo_t *cr, int nextColorFlag) {

    switch (nextColorFlag) {
        case CR_BLACK:
            CairoSetRGB(cr, 0, 0, 0);
            break;
        case CR_RED:
            CairoSetRGB(cr, 239, 41, 41);
            break;
        case CR_GREEN:
            CairoSetRGB(cr, 78, 154, 6);
            break;
        case CR_BLUE:
            CairoSetRGB(cr, 52, 101, 164);
            break;
        case CR_YELLOW:
            CairoSetRGB(cr, 252, 233, 79);
            break;
        case CR_MAGENTA:
            CairoSetRGB(cr, 255, 57, 225);
            break;
        case CR_CYAN:
            CairoSetRGB(cr, 60, 208, 237);
            break;
        case CR_BRANCH1:
            CairoSetRGB(cr, 92, 160, 215);
            break;
        case CR_BRANCH2:
            CairoSetRGB(cr, 183, 127, 77);
            break;
        case CR_BRANCH3:
            CairoSetRGB(cr, 243, 153, 193);
            break;
        case CR_BRANCH4:
            CairoSetRGB(cr, 123, 204, 153);
            break;
        default:
            break;
    }

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
        //DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
        //         radius + 7.5f);

        const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
        const RNAStructure::BaseData *baseData3 = structures[2]->GetBaseAt(ui);

        if (baseData1->m_pair != RNAStructure::UNPAIRED &&
            baseData1->m_pair > ui) {
            if (baseData1->m_pair == baseData2->m_pair) {
                if (baseData1->m_pair == baseData3->m_pair) {
                    fl_color(FL_BLACK);
                    SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLACK);
                    DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                } else {
                    //fl_color(FL_YELLOW);
                    fl_color(fl_rgb_color(255, 200, 0));
                    SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_YELLOW);
                    DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);

                    if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                        baseData3->m_pair > ui) {
                        fl_color(FL_BLUE);
                        SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_BLUE);
                        DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    }
                }
            } else if (baseData1->m_pair == baseData3->m_pair) {
                fl_color(FL_MAGENTA);
                SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_MAGENTA);
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    fl_color(FL_GREEN);
                    SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_GREEN);
                    DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            } else {
                fl_color(FL_RED);
                SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_RED);
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    if (baseData2->m_pair == baseData3->m_pair) {
                        fl_color(FL_CYAN);
                        SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_CYAN);
                        DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    } else {
                        fl_color(FL_GREEN);
                        SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_GREEN);
                        DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);

                        if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                            baseData3->m_pair > ui) {
                            fl_color(FL_BLUE);
                            SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                                (int) m_drawBranchesIndicator->value(), CR_BLUE);
                            DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                                    angleDelta, radius);
                        }
                    }
                } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                           baseData3->m_pair > ui) {
                    fl_color(FL_BLUE);
                    SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLUE);
                    DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                   baseData2->m_pair > ui) {
            if (baseData2->m_pair == baseData3->m_pair) {
                fl_color(FL_CYAN);
                SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_CYAN);
                DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(FL_GREEN);
                SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_GREEN);
                DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                    baseData3->m_pair > ui) {
                    fl_color(FL_BLUE);
                    SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLUE);
                    DrawArc(cr, ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                   baseData3->m_pair > ui) {
            fl_color(FL_BLUE);
            SetCairoBranchColor(cr, structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_BLUE);
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
        //DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
        //         radius + 7.5f);

        const RNAStructure::BaseData *baseData2 = structures[1]->GetBaseAt(ui);
        if (baseData1->m_pair != RNAStructure::UNPAIRED
            && baseData1->m_pair > ui) {
            if (baseData1->m_pair == baseData2->m_pair) {
                fl_color(FL_BLACK);
                SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_BLACK);
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(FL_RED);
                SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_RED);
                DrawArc(cr, ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair !=
                    RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
                    fl_color(FL_GREEN);
                    SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_GREEN);
                    DrawArc(cr, ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair !=
                   RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
            fl_color(FL_GREEN);
            SetCairoBranchColor(cr, structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_GREEN);
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
        //DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta,
        //         radius + 7.5f);

        if (baseData1->m_pair != RNAStructure::UNPAIRED
            && baseData1->m_pair > ui) {
            fl_color(FL_BLACK);
            SetCairoBranchColor(cr, structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_BLACK);
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
    float xPosn1 = centerX + cos(angle1) * radius;
    float yPosn1 = centerY - sin(angle1) * radius - fl_descent() + 0.5 *
                                                                   fl_height();
    fl_color(GUI_WINDOW_BGCOLOR);
    switch (base) {
        case RNAStructure::A:
            fl_draw("A", (int) (xPosn1 - fl_width('A') * 0.5f), (int) yPosn1);
            break;
        case RNAStructure::C:
            fl_draw("C", (int) (xPosn1 - fl_width('C') * 0.5f), (int) yPosn1);
            break;
        case RNAStructure::G:
            fl_draw("G", (int) (xPosn1 - fl_width('G') * 0.5f), (int) yPosn1);
            break;
        case RNAStructure::U:
            fl_draw("U", (int) (xPosn1 - fl_width('U') * 0.5f), (int) yPosn1);
            break;
    }
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
    radius = centerX < centerY ? centerX - 15.f : centerY - 15.f;
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
            m_menus[m]->selection_color(FL_LIGHT2);
	    activeMenuIndex[m] = -1;
            activeSet[m] = false;
	}
        int horizCheckBoxPos = ms_menu_minx[2] + ms_menu_width + WIDGET_SPACING;
        m_drawBranchesIndicator = new Fl_Check_Button(horizCheckBoxPos, 5, 
			          20, 20,"Draw (16S) Domains");
        m_drawBranchesIndicator->callback(checkBoxChangedStateCallback, 
			         m_drawBranchesIndicator);
        m_drawBranchesIndicator->tooltip(
                "Whether to color code the four domains in 16S structures");
        m_drawBranchesIndicator->labelcolor(GUI_TEXT_COLOR);
	m_drawBranchesIndicator->hide(); // make this option invisible

        exportButton = new Fl_Button(horizCheckBoxPos, 25, 
		       EXPORT_BUTTON_WIDTH, 25, "@filesaveas   Export PNG");
        exportButton->type(FL_NORMAL_BUTTON);
        exportButton->callback(exportToPNGButtonPressHandler, exportButton);
        exportButton->labelcolor(GUI_BTEXT_COLOR);

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
    
    DiagramWindow *window = (DiagramWindow *) widget->parent();
    window->m_redrawStructures = true;
    widget->redraw();
    
    // make sure the diagram is drawn right away:
    window->cairoTranslate = false;
    DiagramWindow::Draw((Fl_Cairo_Window *) window, window->crDraw);

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

void DiagramWindow::CairoSetRGB(cairo_t *cr, unsigned short R, unsigned short G, unsigned short B) {
    cairo_set_source_rgb(cr, R / 255.0, G / 255.0, B / 255.0);
}

char *DiagramWindow::GetExportPNGFilePath() {
    const char *chooserMsg = "Choose a file name for your PNG output image";
    const char *fileExtMask = "*.png";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[MAX_BUFFER_SIZE];
    char pngImagePath[MAX_BUFFER_SIZE]; 
    snprintf(pngImagePath, MAX_BUFFER_SIZE - 1, "%s/%s", PNG_OUTPUT_DIRECTORY, PNG_OUTPUT_PATH);
    strftime(defaultFilePath, MAX_BUFFER_SIZE - 1, pngImagePath, tmCurrentTime);
    int pathNameType = 1; // 0 (absolute), otherise (relative)
    return fl_file_chooser(chooserMsg, fileExtMask, defaultFilePath, pathNameType);
}

