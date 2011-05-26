#include "RNAStructViz.h"
#include "MainWindow.h"
#include <stdlib.h>

RNAStructViz* RNAStructViz::ms_instance = 0;

RNAStructViz::RNAStructViz()
{
    m_structureManager = new StructureManager();
    Fl::visual(FL_DOUBLE|FL_RGB8);
    Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_ALPHA);
}

RNAStructViz::~RNAStructViz()
{
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
	delete m_diagramWindows[i];

    delete m_structureManager;
}

bool RNAStructViz::Initialize(int argc, char** argv)
{
    if (!ms_instance)
    {
	ms_instance = new RNAStructViz();
    }

    MainWindow::Initialize(argc, argv);

    return true;
}

void RNAStructViz::Shutdown()
{
    MainWindow::Shutdown();

    if (ms_instance)
    {
	delete ms_instance;
	ms_instance = 0;
    }

    exit(0);
}

void RNAStructViz::AddDiagramWindow()
{
    std::vector<int> structures;
    for (int i = 0; i < m_structureManager->GetStructureCount(); ++i)
    {
	if (m_structureManager->GetStructure(i))
	{
	    structures.push_back(i);
	}
    }

    DiagramWindow* diagram = 0;
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
    {
	diagram = m_diagramWindows[i];
	if (!diagram->visible())
	{
	    diagram->SetStructures(structures);
	    diagram->show();
	    return;
	}
    }

    char* title = (char*)malloc(sizeof(char) * 64);
    snprintf(title, 64, "Structure Diagram %lu", m_diagramWindows.size() + 1);
    diagram = new DiagramWindow(800, 800, title, structures);
    m_diagramWindows.push_back(diagram);
    diagram->show();
}


