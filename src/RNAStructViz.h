/**
    The main application class for RNAStructViz.
*/

#ifndef RNASTRUCTVIZ_H
#define RNASTRUCTVIZ_H

#include <vector>
using std::vector;

#include "StructureManager.h"
#include "MainWindow.h"
#include "DiagramWindow.h"
#include "StatsWindow.h"

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

        void AddDiagramWindow(int index);

        inline const std::vector<StatsWindow*>& GetStatsWindows() const
        {
            return m_statsWindows;
        }

	void AddStatsWindow(int index);
        void TestFolders();

    private:
        static RNAStructViz* ms_instance;
        StructureManager* m_structureManager;
        std::vector<DiagramWindow*> m_diagramWindows;
        std::vector<StatsWindow*> m_statsWindows;

};

#endif //RNASTRUCTVIZ_H
