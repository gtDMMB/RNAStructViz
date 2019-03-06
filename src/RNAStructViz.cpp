#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "FolderStructure.h"
#include <stdlib.h>
#include <unistd.h>

RNAStructViz* RNAStructViz::ms_instance = NULL;

RNAStructViz::RNAStructViz()
{
    m_structureManager = new StructureManager();
    Fl::visual(FL_RGB8 | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE);
    Fl::gl_visual(FL_RGB | FL_DEPTH | FL_DOUBLE | FL_ALPHA | FL_MULTISAMPLE);
}

RNAStructViz::~RNAStructViz()
{
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
	delete m_diagramWindows[i];
	for (unsigned int i = 0; i < m_statsWindows.size(); ++i)
	delete m_statsWindows[i];
    delete m_structureManager;
}

bool RNAStructViz::Initialize(int argc, char** argv)
{
    if (!ms_instance) {
        ms_instance = new RNAStructViz();
        MainWindow::Initialize(argc, argv);
    }
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
void RNAStructViz::AddDiagramWindow(int index)
{
    std::vector<int> structures;
    const std::vector<Folder*>& folders = m_structureManager->GetFolders();
    int shift = 0;
    
    if(folders.empty())
    {
        m_diagramWindows.erase(m_diagramWindows.begin(),m_diagramWindows.end());
        return;
    }
        
    for (int i = 0; i < folders.at(index)->structCount; ++i)
    {
        if(folders[index]->folderStructs[(i + shift)] == -1)
            shift++;
        if (m_structureManager->
        	GetStructure(folders[index]->folderStructs[(i + shift)]))
        {
            structures.push_back(folders[index]->folderStructs[(i + shift)]);
        }
    }
    
    DiagramWindow* diagram = 0;
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
    {
        diagram = m_diagramWindows[i];
        if (!diagram->visible())
        {
            diagram->SetStructures(structures);
            diagram->SetFolderIndex(index);
            diagram->ResetWindow(true);
	    diagram->setAsCurrentDiagramWindow();
	    diagram->show();
	    return;
        }
    }
    
    char* title = (char*) malloc(sizeof(char) * 64);
    snprintf(title, 64, "Structure Diagram %lu", m_diagramWindows.size() + 1);
    diagram = new DiagramWindow(3 * DiagramWindow::ms_menu_width, 
		                IMAGE_HEIGHT + GLWIN_TRANSLATEY + 35, 
				title, structures);
    free(title); 
    
    diagram->SetFolderIndex(index);
    m_diagramWindows.push_back(diagram);
    diagram->show();
}

void RNAStructViz::AddStatsWindow(int index)
{
    std::vector<int> structures;
    const std::vector<Folder*>& folders = m_structureManager->GetFolders();
    int shift = 0;
    
    if(folders.empty())
    {
        m_statsWindows.erase(m_statsWindows.begin(), m_statsWindows.end());
        return;
    }
    
    for (int i = 0; i < folders.at(index)->structCount; ++i)
    {
        if(folders[index]->folderStructs[(i + shift)] == -1)
            shift++;
        if(m_structureManager->
        	GetStructure(folders[index]->folderStructs[(i + shift)]))
        {
            structures.push_back(folders[index]->folderStructs[(i + shift)]);
        }
    }
    
    StatsWindow* stats = 0;
    for (unsigned int i = 0; i < m_statsWindows.size(); ++i)
    {
        stats = m_statsWindows[i];
        if (!stats->visible())
        {
            stats->SetStructures(structures);
            stats->SetFolderIndex(index);
            stats->ResetWindow();
            stats->show();
            return;
        }
    }
    
    char* title = (char*)malloc(sizeof(char) * 64);
    snprintf(title, 64, "Structure Statistics %lu", m_statsWindows.size() + 1);
    stats = new StatsWindow(1000, 700, title, structures);
    free(title); //title = NULL;
    
    stats->SetFolderIndex(index);
    m_statsWindows.push_back(stats);
    stats->show();
    
}

void RNAStructViz::TestFolders()
{
    m_structureManager->PrintFolders();
}



