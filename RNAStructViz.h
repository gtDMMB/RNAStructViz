/**
    The main application class for RNAStructViz.
*/

#ifndef RNASTRUCTVIZ_H
#define RNSSTRUCTVIZ_H

#include "StructureManager.h"
#include "DiagramWindow.h"
#include <vector>

class RNAStructViz
{
public:
    RNAStructViz();

    ~RNAStructViz();

    static bool Initialize(int argc, char** argv);

    static void Shutdown();

    inline static RNAStructViz* GetInstance()
    {
	return ms_instance;
    }

    inline StructureManager* GetStructureManager()
    {
	return m_structureManager;
    }

    inline const std::vector<DiagramWindow*>& GetDiagramWindows() const
    {
	return m_diagramWindows;
    }

    void AddDiagramWindow();

private:
    static RNAStructViz* ms_instance;

    StructureManager* m_structureManager;

    std::vector<DiagramWindow*> m_diagramWindows;
};

#endif
