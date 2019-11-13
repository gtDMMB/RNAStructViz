/**
    The main application class for RNAStructViz.
*/

#ifndef RNASTRUCTVIZ_H
#define RNASTRUCTVIZ_H

#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include <vector>
using std::vector;

#include "StructureManager.h"
#include "MainWindow.h"
#include "DiagramWindow.h"
#include "StatsWindow.h"
#include "CommonDialogs.h"

#define DEFAULT_TITLE_STRING_SIZE       (64)

class RNAStructViz
{
    public:
        RNAStructViz();
        ~RNAStructViz();

        static bool Initialize(int argc, char** argv);
        static void Shutdown();

	static void ExitApplication(bool promptUser = true);
        static void ExitApplication() { RNAStructViz::ExitApplication(true); }

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


    public:
	static int HandleGlobalKeypressEvent(int eventCode);
	
	static std::string LocateSampleStructuresOnSystem();
	static int CopySampleStructures();
        static int BackupAndUnlinkLocalConfigFiles(bool clearStickyFolderNamesOnly = false);

    private:
        static RNAStructViz* ms_instance;
        StructureManager* m_structureManager;
        std::vector<DiagramWindow*> m_diagramWindows;
        std::vector<StatsWindow*> m_statsWindows;

};

typedef struct {
	     
     const char *keyPressIntentDesc;
     const char *keyPressComboDesc;
     int modifierStateKey; 
     int keyPressValue;
     int charKeyValue;
     bool checkOnlyFirstTextChar;
     void (*onPressActionFunc)();
	
} GlobalKeyPressHandlerData_t;

const int MODKEY_NONE   = ((unsigned int) -1);
const int KEYVALUE_NONE = ((unsigned int) -1);
const int CHARKEY_NONE  = ((unsigned int) -1);

#endif //RNASTRUCTVIZ_H
