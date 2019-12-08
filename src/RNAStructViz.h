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
#include "RNAStructVizTypes.h"
#include "TerminalPrinting.h"

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
        
	void RemoveStructure(int folderIndex, int structureIndex);
	void RemoveFolderData(int index);
	void TestFolders();

	inline int GetDiagramWindowForFolderIndex(int findex) {
	     for(int dw = 0; dw < m_diagramWindows.size(); dw++) {
	          if(m_diagramWindows[dw] != NULL && m_diagramWindows[dw]->GetFolderIndex() == findex) {
		       return dw;
		  }
	     }
	     return -1;
	}

	inline int GetStatsWindowForFolderIndex(int findex) {
	     for(int sw = 0; sw < m_statsWindows.size(); sw++) {
	          if(m_statsWindows[sw] != NULL && m_statsWindows[sw]->GetFolderIndex() == findex) {
		       return sw;
		  }
	     }
	     return -1;
	}

    public:
	static int HandleGlobalKeypressEvent(int eventCode);
	
	static std::string LocateSampleStructuresOnSystem();
	static int CopySampleStructures();
        static int BackupAndUnlinkLocalConfigFiles(bool clearStickyFolderNamesOnly = false);
        static bool CopyStructureFileToAutoloadDirectory(const char *structFileBaseName, const char *structFileDiskPath);
	static bool RemoveStructureFileFromAutoloadDirectory(const char *structFileBaseName);

    private:
        static RNAStructViz* ms_instance;
        StructureManager* m_structureManager;
        std::vector<DiagramWindow*> m_diagramWindows;
        std::vector<StatsWindow*> m_statsWindows;

    public:
	class ScheduledDeletion {
	     
	     public:
	          static vector<RNAStructure *>  rnaStructObjs;
		  static vector<Folder *>        folderStructObjs;
		  static vector<StructureData *> structureDataObjs;
                  static vector<Fl_Widget *>     widgetObjs;

		  static inline void PerformScheduledDeletion(void *) {
		       Fl::lock();
		       time_t startTime = time(NULL), endTime;
		       TerminalText::PrintDebug("Performing scheduled deletion of objects #[%d / %d / %d] ... \n", 
				                rnaStructObjs.size(), folderStructObjs.size(), structureDataObjs.size());
		       for(int rsi = 0; rsi < rnaStructObjs.size(); rsi++) {
		            Delete(rnaStructObjs[rsi], RNAStructure);
		       }
		       rnaStructObjs.clear();
		       for(int fsi = 0; fsi < folderStructObjs.size(); fsi++) {
		            Delete(folderStructObjs[fsi], Folder);
		       }
		       folderStructObjs.clear();
		       for(int sdi = 0; sdi < structureDataObjs.size(); sdi++) {
			    Delete(structureDataObjs[sdi], StructureData);
		       }
		       structureDataObjs.clear();
		       for(int wi = 0; wi < widgetObjs.size(); wi++) {
		           Delete(widgetObjs[wi], Fl_Widget);
		       }
		       widgetObjs.clear();
		       endTime = time(NULL);
		       TerminalText::PrintDebug("Done performing scehduled deletion of objects ... In %g seconds.\n", 
				                difftime(endTime, startTime));
		       Fl::unlock();
		  }

		  static inline void ScheduleUpcomingDeletion(double delaySecs = 4.0) {
		       Fl::remove_timeout(PerformScheduledDeletion, NULL);
		       Fl::add_timeout(delaySecs, PerformScheduledDeletion);
		  }

		  static inline void AddRNAStructure(RNAStructure *s) {
		       Fl::lock();
		       if(s != NULL) rnaStructObjs.push_back(s);
		       Fl::unlock();
		  }

		  static inline void AddFolder(Folder *f) {
		       Fl::lock();
		       if(f != NULL) folderStructObjs.push_back(f);
		       Fl::unlock();
		  }

		  static inline void AddStructureData(StructureData *sd) {
		       Fl::lock();
		       if(sd != NULL) structureDataObjs.push_back(sd);
		       Fl::unlock();
		  }

		  static inline void AddWidget(Fl_Widget *w) {
		       Fl::lock();
		       if(w != NULL) widgetObjs.push_back(w);
		       Fl::unlock();
		  }

	};

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
