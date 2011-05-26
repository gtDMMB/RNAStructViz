#include "DiagramWindow.h"
#include "RNAStructViz.h"
#include <FL/Fl_draw.h>
#include <FL/Fl_Box.h>
#include <algorithm>
#include <math.h>
#include <iostream>

const int DiagramWindow::ms_menu_minx[3] = {5, 205, 405};
const int DiagramWindow::ms_menu_width = 190;

void DiagramWindow::Construct(int w, int h, const std::vector<int>& structures)
{
    m_offscreenImage[0] = fl_create_offscreen(2048, 2048);
    m_imageData[0] = new uchar[2048 * 2048 * 3];
    memset(m_imageData[0], 0, 2048 * 2048 * 3);
    m_offscreenImage[1] = fl_create_offscreen(1024, 1024);
    m_imageData[1] = new uchar[1024 * 1024 * 3];
    memset(m_imageData[1], 0, 1024 * 1024 * 3);

    m_glWindow = new GLWindow(0, 120, w, h - 120);
    m_glWindow->SetTextureData(m_imageData[0], 2048);
    m_glWindow->SetTextureData(m_imageData[1], 1024);

    m_menus[0] = m_menus[1] = m_menus[2] = 0;
    m_menuItems = 0;
    m_menuItemsSize = 0;

    color(FL_BLACK);
    size_range(600, 720);
    box(FL_NO_BOX);

    Fl_Box* resizeBox = new Fl_Box(0, 120, w, h - 120);
    resizable(resizeBox);

    SetStructures(structures);
}

DiagramWindow::DiagramWindow(int w, int h, const char *label, const std::vector<int>& structures)
    : Fl_Window(w, h, label)
    , m_redrawStructures(false)
{
    Construct(w, h, structures);
}

DiagramWindow::DiagramWindow(int x, int y, int w, int h, const char *label, const std::vector<int>& structures)
    : Fl_Window(x, y, w, h, label)
    , m_redrawStructures(false)
{
    Construct(w, h, structures);
}

DiagramWindow::~DiagramWindow()
{
    delete[] m_imageData[0];
    delete[] m_imageData[1];

    free(m_menuItems);
}

void DiagramWindow::draw()
{
    Fl_Color priorColor = fl_color();
    int priorFont = fl_font();
    int priorFontSize = fl_size();

    fl_color(color());
    fl_rectf(0, 0, w(), h());
    fl_color(priorColor);

    Fl_Window::draw();

    // Get the structures. Be sure the reference structure is first.
    RNAStructure* sequences[3];
    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    for (int j = 0; j < 3; ++j)
    {
	if ((int)(m_menuItems[m_menus[j]->value()].user_data()) == -1)
	{
	    sequences[j] = 0;
	}
	else
	{
	    sequences[j] = structureManager->GetStructure((int)(m_menuItems[m_menus[j]->value()].user_data()));
	}
    }

    int numToDraw = 0;
    if (sequences[0])
    {
	if (sequences[1])
	{
	    if (sequences[2])
	    {
		numToDraw = 3;
		DrawKey3();
	    }
	    else
	    {
		numToDraw = 2;
		DrawKey2(0, 1);
	    }
	}
	else
	{
	    if (sequences[2])
	    {
		sequences[1] = sequences[2];
		DrawKey2(0, 2);
		numToDraw = 2;
	    }
	    else
	    {
		DrawKey1(0);
		numToDraw = 1;
	    }
	}
    }
    else
    {
	if (sequences[1])
	{
	    if (sequences[2])
	    {
		sequences[0] = sequences[1];
		sequences[1] = sequences[2];
		DrawKey2(1, 2);
		numToDraw = 2;
	    }
	    else
	    {
		sequences[0] = sequences[1];
		DrawKey1(1);
		numToDraw = 1;
	    }
	}
	else
	{
	    if (sequences[2])
	    {
		sequences[0] = sequences[2];
		DrawKey1(2);
		numToDraw = 1;
	    }
	    else
	    {
		numToDraw = 0;
	    }
	}
    }

    if (m_redrawStructures)
    {
	{
	    fl_begin_offscreen(m_offscreenImage[0]);
	    RedrawBuffer(sequences, numToDraw, 2048);
	    fl_read_image(m_imageData[0], 0, 0, 2048, 2048);
	    fl_end_offscreen();
	}

	{
	    fl_begin_offscreen(m_offscreenImage[1]);
	    RedrawBuffer(sequences, numToDraw, 1024);
	    fl_read_image(m_imageData[1], 0, 0, 1024, 1024);
	    fl_end_offscreen();
	}

	m_glWindow->UpdateTexture();
	m_redrawStructures = false;
    }

    m_glWindow->redraw();

    fl_color(priorColor);
    fl_font(priorFont, priorFontSize);
    fl_line_style(0);
}

void DiagramWindow::RedrawBuffer(RNAStructure** structures, const int numStructures, const int resolution)
{
    fl_color(FL_BLACK);
    fl_rectf(0, 0, resolution, resolution);

    int priorFont = fl_font();
    int priorFontSize = fl_size();
    fl_font(priorFont, 10);

    fl_line_style(0);

    if (numStructures == 1)
	Draw1(structures, resolution);
    else if (numStructures == 2)
	Draw2(structures, resolution);
    else if (numStructures == 3)
	Draw3(structures, resolution);

    fl_font(priorFont, priorFontSize);
}

void DiagramWindow::DrawKey3()
{
    int yPosn = 55;

    fl_color(225, 225, 225);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    yPosn += 10;

    fl_color(FL_RED);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    yPosn += 10;

    fl_color(FL_GREEN);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    yPosn += 10;

    fl_color(FL_BLUE);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    yPosn += 10;

    fl_color(FL_YELLOW);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[2]->x(), yPosn, m_menus[2]->x() + m_menus[2]->w());
    yPosn += 10;

    fl_color(FL_MAGENTA);
    fl_rectf(m_menus[0]->x(), yPosn, m_menus[0]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[1]->x(), yPosn, m_menus[1]->x() + m_menus[1]->w());
    yPosn += 10;

    fl_color(FL_CYAN);
    fl_rectf(m_menus[1]->x(), yPosn, m_menus[1]->w(), 3);
    fl_rectf(m_menus[2]->x(), yPosn, m_menus[2]->w(), 3);
    fl_line_style(FL_DOT);
    fl_xyline(m_menus[0]->x(), yPosn, m_menus[0]->x() + m_menus[0]->w());
}

void DiagramWindow::DrawKey2(const int a, const int b)
{
    int yPosn = 55;

    fl_color(225, 225, 225);
    fl_rectf(m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    fl_rectf(m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
    yPosn += 10;

    fl_color(FL_RED);
    fl_rectf(m_menus[a]->x(), yPosn, m_menus[a]->w(), 3);
    yPosn += 10;

    fl_color(FL_GREEN);
    fl_rectf(m_menus[b]->x(), yPosn, m_menus[b]->w(), 3);
}

void DiagramWindow::DrawKey1(const int a)
{
    fl_color(225, 225, 225);
    fl_rectf(m_menus[a]->x(), 55, m_menus[a]->w(), 3);
}

void DiagramWindow::Draw3(RNAStructure** structures, const int resolution)
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase, angleDelta, radius);

    for (unsigned int ui = 0; ui < numBases; ++ui)
    {
	const RNAStructure::BaseData* baseData1 = structures[0]->GetBaseAt(ui);
	DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta, radius + 7.5f);

	const RNAStructure::BaseData* baseData2 = structures[1]->GetBaseAt(ui);
	const RNAStructure::BaseData* baseData3 = structures[2]->GetBaseAt(ui);
	if (baseData1->m_pair != RNAStructure::UNPAIRED && baseData1->m_pair > ui)
	{
	    if (baseData1->m_pair == baseData2->m_pair)
	    {
		if (baseData1->m_pair == baseData3->m_pair)
		{
		    fl_color(FL_WHITE);
		    DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
		}
		else
		{
		    fl_color(FL_YELLOW);
		    DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
		}
	    }
	    else if (baseData1->m_pair == baseData3->m_pair)
	    {
		fl_color(FL_MAGENTA);
		DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	    else
	    {
		fl_color(FL_RED);
		DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	}
	else if (baseData2->m_pair != RNAStructure::UNPAIRED && baseData2->m_pair > ui)
	{
	    if (baseData2->m_pair == baseData3->m_pair)
	    {
		fl_color(FL_CYAN);
		DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	    else
	    {
		fl_color(FL_GREEN);
		DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	}
	else if (baseData3->m_pair != RNAStructure::UNPAIRED && baseData3->m_pair > ui)
	{
	    fl_color(FL_BLUE);
	    DrawArc(ui, baseData3->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	}
    }
}

void DiagramWindow::Draw2(RNAStructure** structures, const int resolution)
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase, angleDelta, radius);

    for (unsigned int ui = 0; ui < numBases; ++ui)
    {
	const RNAStructure::BaseData* baseData1 = structures[0]->GetBaseAt(ui);
	DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta, radius + 7.5f);

	const RNAStructure::BaseData* baseData2 = structures[1]->GetBaseAt(ui);
	if (baseData1->m_pair != RNAStructure::UNPAIRED && baseData1->m_pair > ui)
	{
	    if (baseData1->m_pair == baseData2->m_pair)
	    {
		fl_color(FL_WHITE);
		DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	    else
	    {
		fl_color(FL_RED);
		DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	    }
	}
	else if (baseData2->m_pair != RNAStructure::UNPAIRED && baseData2->m_pair > ui)
	{
	    fl_color(FL_GREEN);
	    DrawArc(ui, baseData2->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	}
    }
}

void DiagramWindow::Draw1(RNAStructure** structures, const int resolution)
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float angleBase = 0.0f;
    float angleDelta = 0.0f;
    float radius = 0.0f;

    unsigned int numBases = structures[0]->GetLength();
    ComputeDiagramParams(numBases, resolution, centerX, centerY, angleBase, angleDelta, radius);

    for (unsigned int ui = 0; ui < numBases; ++ui)
    {
	const RNAStructure::BaseData* baseData1 = structures[0]->GetBaseAt(ui);
	DrawBase(ui, baseData1->m_base, centerX, centerY, angleBase, angleDelta, radius + 7.5f);

	if (baseData1->m_pair != RNAStructure::UNPAIRED && baseData1->m_pair > ui)
	{
	    fl_color(FL_WHITE);
	    DrawArc(ui, baseData1->m_pair, centerX, centerY, angleBase, angleDelta, radius);
	}
    }
}

void DiagramWindow::ComputeCircle(
    const float& x1,
    const float& y1,
    const float& x2,
    const float& y2,
    const float& x3,
    const float& y3,
    double& cX,
    double& cY,
    double& r)
{
    double denom = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - y2 * x3;

    if (denom < 0.001)
    {
	cX = cY = 0.0f;
	r = 0.0f;
    }

    double sq1 = x1 * x1 + y1 * y1;
    double sq2 = x2 * x2 + y2 * y2;
    double sq3 = x3 * x3 + y3 * y3;

    cX = (sq1 * (y2 - y3) - y1 * (sq2 - sq3) + sq2 * y3 - y2 * sq3) / (2.0 * denom);
    cY = (x1 * (sq2 - sq3) - sq1 * (x2 - x3) + sq3 * x2 - x3 * sq2) / (2.0 * denom);

    r = sqrt((x1 - cX) * (x1 - cX) + (y1 - cY) * (y1 - cY));
}

void DiagramWindow::DrawArc(
    const unsigned int b1,
    const unsigned int b2,
    const float centerX,
    const float centerY,
    const float angleBase,
    const float angleDelta,
    const float radius)
{
    float angle1 = angleBase - (float)b1 * angleDelta;
    float xPosn1 = centerX + cos(angle1) * radius;
    float yPosn1 = centerY - sin(angle1) * radius;

    float angle2 = angleBase - (float)b2 * angleDelta;
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

    int boundX = (int)(arcX - arcR);
    int boundY = (int)(arcY - arcR);
    int boundSize = (int)(2.0f * arcR);
    double arc1 = 180.0 / M_PI * atan2(arcY - yPosn1, xPosn1 - arcX);
    double arc2 = 180.0 / M_PI * atan2(arcY - yPosn2, xPosn2 - arcX);

    if (arc2 - arc1 > 180.0)
	arc1 += 360.0;
    if (arc1 - arc2 > 180.0)
	arc2 += 360.0;
    if (arc2 > arc1)
    {
	fl_arc(boundX, boundY, boundSize, boundSize, arc1, arc2);
    }
    else
    {
	fl_arc(boundX, boundY, boundSize, boundSize, arc2, arc1);
    }
}

void DiagramWindow::DrawBase(
    const unsigned int index,
    const RNAStructure::Base base,
    const float centerX,
    const float centerY,
    const float angleBase,
    const float angleDelta,
    const float radius)
{
    float angle1 = angleBase - (float)index * angleDelta;
    float xPosn1 = centerX + cos(angle1) * radius;
    float yPosn1 = centerY - sin(angle1) * radius - fl_descent() + 0.5 * fl_height();

    fl_color(128, 128, 128);
    switch (base)
    {
	case RNAStructure::A:
	    fl_draw("A", xPosn1 - fl_width('A') * 0.5f, yPosn1);
	    break;
	case RNAStructure::C:
	    fl_draw("C", xPosn1 - fl_width('C') * 0.5f, yPosn1);
	    break;
	case RNAStructure::G:
	    fl_draw("G", xPosn1 - fl_width('G') * 0.5f, yPosn1);
	    break;
	case RNAStructure::U:
	    fl_draw("U", xPosn1 - fl_width('U') * 0.5f, yPosn1);
	    break;
    }
}

void DiagramWindow::ComputeDiagramParams(
    const int numBases,
    const int resolution,
    float& centerX,
    float& centerY,
    float& angleBase,
    float& angleDelta,
    float& radius)
{
    angleDelta = (M_PI * 2.0f - 0.05f) / (float)numBases;
    angleBase = 1.5f * M_PI - 0.025f;
    centerX = (float)resolution / 2.0f;
    centerY = (float)resolution / 2.0f;
    radius = centerX < centerY ? centerX - 15.f : centerY - 15.f;
}

void DiagramWindow::AddStructure(const int index)
{
    if (std::find(m_structures.begin(), m_structures.end(), index) == m_structures.end())
    {
	m_structures.push_back(index);
	RebuildMenus();
	redraw();
    }
}

void DiagramWindow::RemoveStructure(const int index)
{
    std::vector<int>::iterator iter = std::find(m_structures.begin(), m_structures.end(), index);

    if (iter != m_structures.end())
    {
	m_structures.erase(iter);
	RebuildMenus();
	redraw();
    }
}

void DiagramWindow::SetStructures(const std::vector<int>& structures)
{
    m_structures.clear();
    for (unsigned int ui = 0; ui < structures.size(); ++ui)
	m_structures.push_back(structures[ui]);

    RebuildMenus();
    redraw();
}

void DiagramWindow::RebuildMenus()
{
    // Create the menus, if they don't already exist.
    int activeMenuIndex[3];
    bool activeSet[3];
    if (!m_menus[0])
    {
	this->begin();

	Fl_Box* label = new Fl_Box(ms_menu_minx[0], 0, ms_menu_width, 25, "Structure 1");
	label->labelcolor(FL_WHITE);
	label = new Fl_Box(ms_menu_minx[1], 0, ms_menu_width, 25, "Structure 2");
	label->labelcolor(FL_WHITE);
	label = new Fl_Box(ms_menu_minx[2], 0, ms_menu_width, 25, "Structure 3");
	label->labelcolor(FL_WHITE);
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

	this->end();
    }
    else
    {
	// Cache the current active index, if any
	for (int j = 0; j < 3; ++j)
	{
	    activeMenuIndex[j] = (int)m_menus[j]->mvalue()->user_data();
	    activeSet[j] = false;
	}
    }
    
    // Remove any unnecessary menu items
    for (int i = m_structures.size() + 1; i < m_menuItemsSize; ++i)
    {
	m_menuItems[i].label(0);
    }

    // Reallocate if necessary
    if ((int)m_structures.size() + 2 > m_menuItemsSize)
    {
	m_menuItemsSize = m_structures.size() + 2;
	free(m_menuItems);
	m_menuItems = (Fl_Menu_Item*)malloc(sizeof(Fl_Menu_Item) * m_menuItemsSize);
	m_menuItems[0].label("None");
	m_menuItems[0].shortcut(0);
	m_menuItems[0].user_data((void*)-1);

	for (int i = 0; i < m_menuItemsSize; ++i)
	{
	    m_menuItems[i].callback((Fl_Callback*)0);
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
    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui)
    {
	RNAStructure* structure = structureManager->GetStructure(m_structures[ui]);

	m_menuItems[ui + 1].label(structure->GetFilename());
	m_menuItems[ui + 1].user_data((void*)m_structures[ui]);
	m_menuItems[ui + 1].shortcut(0);

	for (int j = 0; j < 3; ++j)
	{
	    if (activeMenuIndex[j] == m_structures[ui])
	    {
		m_menus[j]->value(ui + 1);
		activeSet[j] = true;
	    }
	}
    }

    // Reset active entries that have not already been set, and set the last entry to NULL label
    m_menuItems[m_structures.size() + 1].label(0);
    for (int j = 0; j < 3; ++j)
    {
	if (!activeSet[j])
	{
	    m_menus[j]->value(m_menuItems);
	}
    }
}

void DiagramWindow::MenuCallback(Fl_Widget* widget, void* userData)
{
    DiagramWindow* window = (DiagramWindow*)widget->parent();
    window->m_redrawStructures = true;
    window->redraw();
}

