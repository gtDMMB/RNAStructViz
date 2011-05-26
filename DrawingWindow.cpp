#include "DrawingWindow.h"
#include "RNAStructViz.h"
#include <FL/Fl_draw.h>
#include <algorithm>
#include <math.h>

DrawingWindow::DrawingWindow(int w, int h, const char *label)
    : Fl_Double_Window(w, h, label)
    , m_rebuildRequired(false)
{
    color(FL_WHITE);
}

DrawingWindow::DrawingWindow(int x, int y, int w, int h, const char *label)
    : Fl_Double_Window(x, y, w, h, label)
{
    color(FL_WHITE);
}

DrawingWindow::~DrawingWindow()
{
}

void DrawingWindow::draw()
{
    Fl_Double_Window::draw();

    if (m_activeIndices.empty())
	return;

    // Get the structures. Be sure the reference structure is first.
    RNAStructure** sequences = (RNAStructure**)malloc(sizeof(RNAStructure*) * m_activeIndices.size());
    StructureManager* structureManager = RNAStructViz::GetStructureManager();
    int posn = 0;
    for (unsigned int i = 0; i < m_activeIndices.size(); ++i)
    {
	if (m_activeIndices[i] == structureManager->GetReferenceStructure())
	{
	    sequences[posn++] = structureManager->GetStructure(m_activeIndices[i]);
	}
    }
    for (unsigned int i = 0; i < m_activeIndices.size(); ++i)
    {
	if (m_activeIndices[i] != structureManager->GetReferenceStructure())
	{
	    sequences[posn++] = structureManager->GetStructure(m_activeIndices[i]);
	}
    }

    fl_color(FL_BLACK);

    // Draw the circle of bases
    int numBases = sequences[0]->GetLength();
    float angleDelta = (M_PI * 2.0f - 0.2f) / (float)numBases;
    float centerX = (float)w() / 2.0f;
    float centerY = (float)h() / 2.0f;
    float radius = centerX < centerY ? centerX - 20.f : centerY - 20.f;
    float angle = 1.5f * M_PI - 0.1f;
    for (int i = 0; i < numBases; ++i)
    {
	float xPosn = centerX + cos(angle) * radius;
	float yPosn = centerY - sin(angle) * radius;
	switch (sequences[0]->GetBaseAt(i)->m_base)
	{
	    case RNAStructure::A:
		fl_draw("A", xPosn, yPosn);
		break;
	    case RNAStructure::C:
		fl_draw("C", xPosn, yPosn);
		break;
	    case RNAStructure::G:
		fl_draw("G", xPosn, yPosn);
		break;
	    case RNAStructure::U:
		fl_draw("U", xPosn, yPosn);
		break;
	}
	angle -= angleDelta;
	fl_point((int)xPosn, (int)yPosn);
    }

    // Clean up
    free(sequences);
}

void DrawingWindow::AddActive(const int index)
{
    if (std::find(m_activeIndices.begin(), m_activeIndices.end(), index) != m_activeIndices.end())
	return;

    m_activeIndices.push_back(index);

    m_rebuildRequired = true;
}

void DrawingWindow::RemoveActive(const int index)
{
    std::vector<int>::iterator iter = std::find(m_activeIndices.begin(), m_activeIndices.end(), index);

    if (iter != m_activeIndices.end())
    {
	m_activeIndices.erase(iter);
	m_rebuildRequired = true;
    }
}

