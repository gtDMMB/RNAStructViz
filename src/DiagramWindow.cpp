#include "DiagramWindow.h"
#include "RNAStructViz.h"
#include "BranchTypeIdentification.h"
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Cairo.H>
#include <cairo.h>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <time.h>

const int DiagramWindow::ms_menu_minx[3] = {5, 205, 405};
const int DiagramWindow::ms_menu_width = 190;

void DiagramWindow::Construct(int w, int h, const std::vector<int> &structures) {

    imageStride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, IMAGE_WIDTH);
    //crSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_WIDTH);
    m_imageData[0] = new uchar[imageStride * IMAGE_HEIGHT];
    memset(m_imageData[0], 0, imageStride * IMAGE_HEIGHT);
    crSurface = cairo_image_surface_create_for_data(m_imageData[0], CAIRO_FORMAT_ARGB32, 
                                                    IMAGE_WIDTH, IMAGE_HEIGHT, imageStride);
    //crDraw = cairo_create(crSurface);
    //Fl::cairo_autolink_context(true);
    //Fl::cairo_cc(crDraw, false);
    
    //m_offscreenImage[0] = fl_create_offscreen(2048, 2048);
    //m_imageData[0] = new uchar[2048 * 2048 * 3];
    //memset(m_imageData[0], 0, 2048 * 2048 * 3);
    //m_offscreenImage[1] = fl_create_offscreen(1024, 1024);
    int windowImageBufSize = (IMAGE_WIDTH + 150) * (IMAGE_HEIGHT + 150) * IMAGE_DEPTH;
    m_imageData[1] = new uchar[windowImageBufSize];
    memset(m_imageData[1], 0, windowImageBufSize);
    crMaskSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH + 150, IMAGE_WIDTH + 150);
    crDrawTemp = cairo_create(crMaskSurface);

    pixelWidth = 1; //2;

    m_glWindow = new GLWindow(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY, IMAGE_WIDTH + 150, IMAGE_HEIGHT + 150);
    m_glWindow->mode(FL_DOUBLE | FL_INDEX);
    //m_glWindow->SetTextureData(m_imageData[0], 2048);
    //m_glWindow->SetTextureData(m_imageData[1], 1024);
    CairoDrawBufferToScreen();

    crDraw = Fl::cairo_make_current(m_glWindow); 
    crDraw = cairo_reference(crDraw);
    cairo_translate(crDraw, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    //m_glWindow->position(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(crDraw);
    CairoPrepareDisplay();

    m_menus[0] = m_menus[1] = m_menus[2] = NULL;
    m_menuItems = 0;
    m_menuItemsSize = 0;
    folderIndex = -1;
    m_drawBranchesIndicator = NULL;
    userConflictAlerted = false;

    Fl::visual(FL_RGB);

    //colors the top of the Diagram window where structures are chosen
    color(FL_WHITE);
    //color(FL_BACKGROUND_COLOR);
    //size_range(580, 700);
    size_range(IMAGE_WIDTH + 150, IMAGE_HEIGHT + 150);
    box(FL_NO_BOX);

    //Fl_Box *resizeBox = new Fl_Box(0, 120, w, h - 120);
    //resizable(resizeBox);

    title = (char *) malloc(sizeof(char) * 64);
    SetStructures(structures);
    startRefreshTimer();
}

DiagramWindow::DiagramWindow(int w, int h, const char *label,
                             const std::vector<int> &structures)
        : Fl_Window(w + 150, h, label), m_redrawStructures(true), holdSurfaceRef(false) {
    Construct(w + 150, h, structures);
}

DiagramWindow::DiagramWindow(int x, int y, int w, int h, const char *label,
                             const std::vector<int> &structures)
        : Fl_Window(x, y, w + 150, h, label), m_redrawStructures(true), holdSurfaceRef(false) {
    Construct(w + 150, h, structures);
}

DiagramWindow::~DiagramWindow() {
    delete[] m_imageData[0];
    delete[] m_imageData[1];
    delete m_drawBranchesIndicator;
    cairo_destroy(crDraw);
    cairo_pattern_destroy(circleMask);
    cairo_destroy(crDrawTemp);
    if(holdSurfaceRef)
         cairo_surface_destroy(crSurface);
    cairo_surface_destroy(crMaskSurface);
    free(m_menuItems);
}

void DiagramWindow::SetFolderIndex(int index) {
    folderIndex = index;

    sprintf(title, "Diagrams: %-.48s",
            RNAStructViz::GetInstance()->GetStructureManager()->
                    GetFolderAt(index)->folderName);
    label(title);
}

void DiagramWindow::ResetWindow(bool resetMenus = true) {
    this->size(IMAGE_WIDTH + 150, IMAGE_HEIGHT + 150);

    delete[] m_imageData[0];
    delete[] m_imageData[1];
    //m_glWindow->clear();
    //cairo_destroy(crDraw);
    //if(holdSurfaceRef)
    //     cairo_surface_destroy(crSurface);
    //cairo_pattern_destroy(circleMask);
    cairo_destroy(crDrawTemp);
    cairo_surface_destroy(crMaskSurface);

    imageStride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, IMAGE_WIDTH);
    //crSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_WIDTH);
    m_imageData[0] = new uchar[imageStride * IMAGE_HEIGHT];
    memset(m_imageData[0], 0, imageStride * IMAGE_HEIGHT);
    crSurface = cairo_image_surface_create_for_data(m_imageData[0], CAIRO_FORMAT_ARGB32, 
                                                    IMAGE_WIDTH, IMAGE_HEIGHT, imageStride);
    //crDraw = cairo_create(crSurface);
    crDraw = Fl::cairo_make_current(m_glWindow);
    crDraw = cairo_reference(crDraw);
    cairo_translate(crDraw, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(crDraw);

    //m_glWindow->position(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    CairoPrepareDisplay();

    //m_offscreenImage[0] = fl_create_offscreen(2048, 2048);
    //m_imageData[0] = new uchar[2048 * 2048 * 3];
    //memset(m_imageData[0], 0, 2048 * 2048 * 3);
    //m_offscreenImage[1] = fl_create_offscreen(1024, 1024);
    int windowImageBufSize = (IMAGE_WIDTH + 150) * (IMAGE_HEIGHT + 150) * IMAGE_DEPTH;
    m_imageData[1] = new uchar[windowImageBufSize];
    memset(m_imageData[1], 0, windowImageBufSize);
    crMaskSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH + 150, IMAGE_WIDTH + 150);
    crDrawTemp = cairo_create(crMaskSurface);

    //crSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_WIDTH);
    //crDraw = cairo_create(crSurface);
    //CairoPrepareDisplay();

    //m_glWindow->SetTextureData(m_imageData[0], 2048);
    //m_glWindow->SetTextureData(m_imageData[1], 1024);
    CairoDrawBufferToScreen();

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
        thisWindow->ResetWindow(false);
        cbDrawIndicator->clear_changed();
        thisWindow->redraw();
    }
}

void DiagramWindow::exportToPNGButtonPressHandler(Fl_Widget *, void *v) {
    Fl_Button *buttonPressed = (Fl_Button *) v;
    if (buttonPressed->changed()) {
        DiagramWindow *thisWindow = (DiagramWindow *) buttonPressed->parent();
        char *exportFilePath = thisWindow->GetExportPNGFilePath();
        cairo_surface_write_to_png(cairo_get_target(thisWindow->crDraw), exportFilePath);
        buttonPressed->clear_changed();
    }
}

void DiagramWindow::resize(int x, int y, int w, int h) {
    Fl_Window::resize(x, y, w, h);
    int s;
    if (w > h - 120) s = h - 120;
    else s = w;
    //m_glWindow->size(s,s);
    m_glWindow->resize(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY, w, h);
}

void DiagramWindow::draw() {

    Fl_Color priorColor = fl_color();
    int priorFont = fl_font();
    int priorFontSize = fl_size();

    fl_color(color());
    fl_rectf(0, 0, w(), h());
    fl_color(priorColor);

    //crDraw = Fl::cairo_make_current(m_glWindow);
    //crDraw = cairo_reference(crDraw);
    //cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.5);
    //cairo_paint(crDraw);
    //Fl_Window::draw();
    
    // Get the structures. Be sure the reference structure is first.
    RNAStructure *sequences[3];
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
            } else if (sequences[j]->GetLength() <= 1000 && sequences[j]->GetLength() >= 500) {
                pixelWidth = 2;
            } else {
                pixelWidth = 3;
            }
        }
    }

    int numToDraw = 0;
    if (sequences[0]) {
        if (sequences[1]) {
            if (sequences[2]) {
                numToDraw = 3;
                ComputeNumPairs(sequences, 3);
                DrawKey3();
            } else {
                numToDraw = 2;
                ComputeNumPairs(sequences, 2);
                DrawKey2(0, 1);
            }
        } else {
            if (sequences[2]) {
                sequences[1] = sequences[2];
                ComputeNumPairs(sequences, 2);
                DrawKey2(0, 2);
                numToDraw = 2;
            } else {
                ComputeNumPairs(sequences, 1);
                DrawKey1(0);
                numToDraw = 1;
            }
        }
    } else {
        if (sequences[1]) {
            if (sequences[2]) {
                sequences[0] = sequences[1];
                sequences[1] = sequences[2];
                ComputeNumPairs(sequences, 2);
                DrawKey2(1, 2);
                numToDraw = 2;
            } else {
                sequences[0] = sequences[1];
                ComputeNumPairs(sequences, 1);
                DrawKey1(1);
                numToDraw = 1;
            }
        } else {
            if (sequences[2]) {
                sequences[0] = sequences[2];
                ComputeNumPairs(sequences, 1);
                DrawKey1(2);
                numToDraw = 1;
            } else {
                numToDraw = 0;
            }
        }
    }

    if (m_redrawStructures) {
            //fl_begin_offscreen(m_offscreenImage[0]);
            RedrawBuffer(sequences, numToDraw, IMAGE_WIDTH);
            //fl_read_image(m_imageData[0], 0, 0, 2048, 2048);
            //fl_end_offscreen();
            
            //fl_begin_offscreen(m_offscreenImage[1]);
            //RedrawBuffer(sequences, numToDraw, 1024);
            //fl_read_image(m_imageData[1], 0, 0, 1024, 1024);
            //fl_end_offscreen();

            //m_glWindow->UpdateTexture();
            m_redrawStructures = false;
            m_glWindow->redraw();
            //startRefreshTimer();
            cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.5);
            cairo_paint(crDraw);
            cairo_mask_surface(crDraw, crMaskSurface, IMAGE_WIDTH / 2, IMAGE_WIDTH / 2);

    }

    fl_color(priorColor);
    fl_font(priorFont, priorFontSize);
    fl_line_style(0);

    crDraw = Fl::cairo_make_current(m_glWindow);
    crDraw = cairo_reference(crDraw);
    cairo_translate(crDraw, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    //cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.5);
    //cairo_paint(crDraw);
    //m_glWindow->position(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    Fl_Window::draw();

}

void DiagramWindow::RedrawBuffer(RNAStructure **structures,
                                 const int numStructures, const int resolution) {
    //fl_color(FL_WHITE);
    //fl_rectf(0, 0, resolution, resolution);

    int priorFont = fl_font();
    int priorFontSize = fl_size();
    fl_font(priorFont, 10);

    fl_line_style(0);

    if (numStructures == 1) {
        Draw1(structures, resolution);
    } else if (numStructures == 2) {
        Draw2(structures, resolution);
    } else if (numStructures == 3) {
        Draw3(structures, resolution);
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

    //fl_color(FL_YELLOW);
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

// draws / masks the outer circle on the arcs created by Draw*():
void DiagramWindow::CairoPrepareDisplay() {
    cairo_set_source_rgb(crDrawTemp, 1.0, 1.0, 1.0);
    cairo_fill(crDrawTemp);
    //crDrawTemp = cairo_create(crSurface);
    //cairo_push_group(crDrawTemp);
    //cairo_set_source_rgba(crDrawTemp, 1.0, 1.0, 1.0, 0.0);
    //cairo_paint(crDrawTemp);
    cairo_translate(crDrawTemp, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_arc(crDrawTemp, IMAGE_WIDTH / 2, IMAGE_WIDTH / 2, 0.0, 0.0, 2.0 * M_PI);
    cairo_set_source_rgba(crDrawTemp, 0.4, 0.4, 0.4, 1.0);
    cairo_fill(crDrawTemp);
    //circleMask = cairo_pop_group(crDrawTemp);
    ////cairo_destroy(crDrawTemp);
    //cairo_pop_group_to_source(crDraw);
    //cairo_push_group(crDraw);
    //cairo_set_source_rgba(crDraw, 1.0, 1.0, 1.0, 0.5);
    //cairo_paint(crDraw);
}

void DiagramWindow::CairoBufferFinishingTouches() {
    //cairo_mask(crDraw, circleMask);
}

void DiagramWindow::CairoDrawBufferToScreen() {
    //m_glWindow->SetTextureData(cairo_image_surface_get_data(crSurface), IMAGE_WIDTH);
    //fl_draw_image(cairo_image_surface_get_data(crSurface), 0, 120, IMAGE_WIDTH, IMAGE_HEIGHT,
    //              IMAGE_DEPTH, 0);
    ///char * outputFile = GetExportPNGFilePath();
    ///cairo_surface_write_to_png(crSurface, outputFile);
    ///return;
    //fl_draw_image(m_imageData[0], 0, 120, IMAGE_WIDTH, IMAGE_HEIGHT, imageStride / IMAGE_WIDTH, 0);
}

void DiagramWindow::SetCairoBranchColor(const BranchID_t &branchType, int enabled,
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
    switch (nextColorFlag) {
        case CR_BLACK:
            CairoSetRGB(0, 0, 0);
            break;
        case CR_RED:
            CairoSetRGB(239, 41, 41);
            break;
        case CR_GREEN:
            CairoSetRGB(78, 154, 6);
            break;
        case CR_BLUE:
            CairoSetRGB(52, 101, 164);
            break;
        case CR_YELLOW:
            CairoSetRGB(252, 233, 79);
            break;
        case CR_MAGENTA:
            CairoSetRGB(255, 57, 225);
            break;
        case CR_CYAN:
            CairoSetRGB(60, 208, 237);
            break;
        case CR_BRANCH1:
            CairoSetRGB(92, 160, 215);
            break;
        case CR_BRANCH2:
            CairoSetRGB(183, 127, 77);
            break;
        case CR_BRANCH3:
            CairoSetRGB(243, 153, 193);
            break;
        case CR_BRANCH4:
            CairoSetRGB(123, 204, 153);
            break;
        default:
            break;
    }

}

void DiagramWindow::Draw3(RNAStructure **structures, const int resolution) {
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
                    SetCairoBranchColor(structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLACK);
                    DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                } else {
                    //fl_color(FL_YELLOW);
                    fl_color(fl_rgb_color(255, 200, 0));
                    SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_YELLOW);
                    DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);

                    if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                        baseData3->m_pair > ui) {
                        fl_color(FL_BLUE);
                        SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_BLUE);
                        DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    }
                }
            } else if (baseData1->m_pair == baseData3->m_pair) {
                fl_color(FL_MAGENTA);
                SetCairoBranchColor(structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_MAGENTA);
                DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    fl_color(FL_GREEN);
                    SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_GREEN);
                    DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            } else {
                fl_color(FL_RED);
                SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_RED);
                DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                    baseData2->m_pair > ui) {
                    if (baseData2->m_pair == baseData3->m_pair) {
                        fl_color(FL_CYAN);
                        SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_CYAN);
                        DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);
                    } else {
                        fl_color(FL_GREEN);
                        SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                            (int) m_drawBranchesIndicator->value(), CR_GREEN);
                        DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                                angleDelta, radius);

                        if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                            baseData3->m_pair > ui) {
                            fl_color(FL_BLUE);
                            SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                                (int) m_drawBranchesIndicator->value(), CR_BLUE);
                            DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase,
                                    angleDelta, radius);
                        }
                    }
                } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                           baseData3->m_pair > ui) {
                    fl_color(FL_BLUE);
                    SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLUE);
                    DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair != RNAStructure::UNPAIRED &&
                   baseData2->m_pair > ui) {
            if (baseData2->m_pair == baseData3->m_pair) {
                fl_color(FL_CYAN);
                SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_CYAN);
                DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(FL_GREEN);
                SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_GREEN);
                DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                    baseData3->m_pair > ui) {
                    fl_color(FL_BLUE);
                    SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_BLUE);
                    DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData3->m_pair != RNAStructure::UNPAIRED &&
                   baseData3->m_pair > ui) {
            fl_color(FL_BLUE);
            SetCairoBranchColor(structures[2]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_BLUE);
            DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
        }
    }
    CairoBufferFinishingTouches();
    CairoDrawBufferToScreen();
}

void DiagramWindow::Draw2(RNAStructure **structures, const int resolution) {
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
                SetCairoBranchColor(structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_BLACK);
                DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);
            } else {
                fl_color(FL_RED);
                SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                    (int) m_drawBranchesIndicator->value(), CR_RED);
                DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                        angleDelta, radius);

                if (baseData2->m_pair !=
                    RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
                    fl_color(FL_GREEN);
                    SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                        (int) m_drawBranchesIndicator->value(), CR_GREEN);
                    DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                            angleDelta, radius);
                }
            }
        } else if (baseData2->m_pair !=
                   RNAStructure::UNPAIRED && baseData2->m_pair > ui) {
            fl_color(FL_GREEN);
            SetCairoBranchColor(structures[1]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_GREEN);
            DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
        }
    }
    CairoBufferFinishingTouches();
    CairoDrawBufferToScreen();
}

void DiagramWindow::Draw1(RNAStructure **structures, const int resolution) {
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
            SetCairoBranchColor(structures[0]->GetBranchTypeAt(ui)->getBranchID(),
                                (int) m_drawBranchesIndicator->value(), CR_BLACK);
            DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase,
                    angleDelta, radius);
            counter++;
        }
    }

    CairoBufferFinishingTouches();
    CairoDrawBufferToScreen();

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
    cairo_set_line_width(crDraw, MAX(1, pixelWidth));
    
    if (arc2 - arc1 > 180.0)
        arc1 += 360.0;
    if (arc1 - arc2 > 180.0)
        arc2 += 360.0;
    if (arc2 > arc1) {
        //fl_arc(boundX, boundY, boundSize, boundSize, arc1, arc2);
        //if (pixelWidth > 1) 
        //    fl_arc(boundX+1,boundY+1,boundSize,boundSize,arc1,arc2);
        //if (pixelWidth > 2)
        //    fl_arc(boundX+1,boundY,boundSize,boundSize,arc1,arc2);
        cairo_arc(crDraw, boundingBoxCenterX, boundingBoxCenterY, boundingBoxRadius, arc1, arc2);
    } else {
        //fl_arc(boundX, boundY, boundSize, boundSize, arc2, arc1);
        //if (pixelWidth > 1)
        //    fl_arc(boundX+1, boundY+1, boundSize, boundSize, arc2, arc1);
        //if (pixelWidth > 2)
        //    fl_arc(boundX+1, boundY, boundSize, boundSize, arc2, arc1);
        cairo_arc(crDraw, boundingBoxCenterX, boundingBoxCenterY, boundingBoxRadius, arc2, arc1);
    }
    //cairo_close_path(crDraw);
    //cairo_translate(crDraw, GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
    cairo_stroke(crDraw);
    //cairo_move_to (crDraw, 0, 0);
    //cairo_line_to (crDraw, 10, 10);
    //cairo_stroke (crDraw);
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


    //fl_color(128, 128, 128);
    fl_color(FL_WHITE);
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
        label->labelcolor(FL_BLACK);
        label = new Fl_Box(ms_menu_minx[1], 0, ms_menu_width, 25, "Structure 2");
        label->labelcolor(FL_BLACK);
        label = new Fl_Box(ms_menu_minx[2], 0, ms_menu_width, 25, "Structure 3");
        label->labelcolor(FL_BLACK);
        m_menus[0] = new Fl_Choice(ms_menu_minx[0], 25, ms_menu_width, 25);
        m_menus[1] = new Fl_Choice(ms_menu_minx[1], 25, ms_menu_width, 25);
        m_menus[2] = new Fl_Choice(ms_menu_minx[2], 25, ms_menu_width, 25);
        m_menus[0]->callback(MenuCallback);
        m_menus[1]->callback(MenuCallback);
        m_menus[2]->callback(MenuCallback);
        activeMenuIndex[0] = -1;
        activeMenuIndex[1] = -1;
        activeMenuIndex[2] = -1;
        activeSet[0] = false;
        activeSet[1] = false;
        activeSet[2] = false;

        int horizCheckBoxPos = ms_menu_minx[2] + ms_menu_width + 15;
        m_drawBranchesIndicator = new Fl_Check_Button(horizCheckBoxPos, 5, 20, 20,
                                                      "Draw (16S) Domains");
        m_drawBranchesIndicator->callback(checkBoxChangedStateCallback, m_drawBranchesIndicator);
        m_drawBranchesIndicator->tooltip(
                "Set whether to color code the four domains in 16S structures?");

        exportButton = new Fl_Button(horizCheckBoxPos, 35, 150, 25, "@filesaveas Export PNG");
        exportButton->type(FL_NORMAL_BUTTON);
        exportButton->callback(exportToPNGButtonPressHandler, exportButton);

        //m_glWindow->position(GLWIN_TRANSLATEX, GLWIN_TRANSLATEY);
        m_glWindow->show();
        m_glWindow->callback(MenuCallback); // redraw the window when anything changes

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
            m_menuItems[i].labelcolor(FL_BLACK);
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
    window->redraw();
}

void DiagramWindow::WarnUserDrawingConflict() {
    if (!userConflictAlerted && m_drawBranchesIndicator->value()) {
        int turnOff = fl_ask(
                "You are attempting to draw multiple structures with distinct branch coding enabled. \nWe recommend against this due to readability concerns! \nDo you want to turn off branch / domain color coding now?");
        if (turnOff) {
            m_drawBranchesIndicator->value(0);
        }
        userConflictAlerted = true;
    }
}

void DiagramWindow::CairoSetRGB(unsigned short R, unsigned short G, unsigned short B) {
    //fprintf(stderr, "(%g, %g, %g)\n", R / 255.0, G / 255.0, B / 255.0);
    //fprintf(stderr, "Cairo Enabled? %s\n", Fl::cairo_autolink_context() ? "Yes" : "No");
    cairo_set_source_rgb(crDraw, R / 255.0, G / 255.0, B / 255.0);
}

char *DiagramWindow::GetExportPNGFilePath() {
    const char *chooserMsg = "Choose a file name for your PNG output image";
    const char *fileExtMask = "*.png";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[256];
    strftime(defaultFilePath, 255, "RNAStructViz-GUIView-%F-%H%M%S.png", tmCurrentTime);
    int pathNameType = 1; // 0 (absolute), otherise (relative)
    return fl_file_chooser(chooserMsg, fileExtMask, defaultFilePath, pathNameType);
}

