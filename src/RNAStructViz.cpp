#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <FL/names.h>

#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "FolderStructure.h"
#include "ConfigParser.h"
#include "TerminalPrinting.h"

static GlobalKeyPressHandlerData_t GLOBAL_STRUCTVIZ_KEYPRESS_HANDLER_DATA[] = {
             {
	          "Standard help key shortcut", 
		  "[F1]", 
		  MODKEY_NONE, 
		  FL_F + 1,
		  CHARKEY_NONE, 
		  false,
		  CommonDialogs::DisplayHelpDialog,
	     },
	     {
		  "Close window shortcut", 
		  "[ALT]+[F4]", 
		  FL_ALT, 
		  FL_F + 4,
		  CHARKEY_NONE,
		  false, 
		  RNAStructViz::ExitApplication,
	     },
	     {
		  "Close window shortcut (standard terminal key combo)",
		  "[CTRL]+c",
		  FL_CTRL,
		  KEYVALUE_NONE,
		  (unsigned int) 'c', 
		  true,
		  RNAStructViz::ExitApplication,
	     },
	     {
		  "Help command shortcut II", 
		  "[CTRL]+h", 
		  FL_CTRL,
		  KEYVALUE_NONE,
		  (unsigned int) 'h', 
		  true, 
		  CommonDialogs::DisplayHelpDialog,
	     },
	     {
		  "Open file dialog shortcut", 
		  "[CTRL]+o", 
		  FL_CTRL,
		  KEYVALUE_NONE,
		  (unsigned int) 'o',
		  true,
		  MainWindow::OpenFileCallback,
	     },
	     {
		  "About the application shortcut", 
		  "[CTRL]+i", 
		  FL_CTRL,
		  KEYVALUE_NONE,
		  (unsigned int) 'i',
		  true,
		  CommonDialogs::DisplayInfoAboutDialog,
	     },
	     {
		  "Configuration settings shortcut", 
		  "[HOME] | [WINDOWS KEY]", 
                  FL_META,
		  FL_Home,
		  CHARKEY_NONE,
		  false,
		  MainWindow::ConfigOptionsCallback,
	     },
};

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
    ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
    MainWindow::Shutdown();
    //exit(EXIT_SUCCESS);
}

void RNAStructViz::ExitApplication(bool promptUser) {
     if(!promptUser) {
          RNAStructViz::Shutdown();
	  return;
     }
     const char *promptQMsg = "Are you really sure you want to quit? All unsaved data and loaded structures will be lost!";
     int userResp = fl_choice(promptQMsg, "Yes, QUIT!", "No, CANCEL", NULL);
     switch(userResp) {
          case 0:
	       RNAStructViz::Shutdown();
	       return;
	  case 1:
	  default:
	       break;
     }
     return;
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
    
    DiagramWindow* diagram = NULL;
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
    {
        diagram = m_diagramWindows[i];
        if ((diagram != NULL) && (diagram->GetFolderIndex() == index) && !diagram->visible())
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
    diagram->SetFolderIndex(index);
    diagram->setAsCurrentDiagramWindow();
    m_diagramWindows.push_back(diagram);
    diagram->show();
    free(title); 
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
        if ((stats != NULL) && (stats->GetFolderIndex() == index) && 
	    !stats->visible())
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
    stats = new StatsWindow(1250, 700, title, structures);
    free(title); //title = NULL;
    
    stats->SetFolderIndex(index);
    m_statsWindows.push_back(stats);
    stats->show();
    
}

void RNAStructViz::TestFolders()
{
    m_structureManager->PrintFolders();
    TerminalText::PrintDebug("Number of Diagram/Stats Windows: %d/%d\n\n", 
                             m_diagramWindows.size(), m_statsWindows.size());
}

int RNAStructViz::HandleGlobalKeypressEvent(int eventCode) {
     
     if(eventCode == FL_FOCUS || eventCode == FL_UNFOCUS) {
          return 1;
     }
     else if(eventCode != FL_KEYBOARD) {
	  TerminalText::PrintDebug("Non-keyboard event triggered (%s).\n", fl_eventnames[eventCode]);
          return 0;
     }

     const char *keyCharText = Fl::event_text();
     int charTextLength = Fl::event_length();
     int keyPressed = Fl::event_original_key();
     int keyState = Fl::event_state();
     for(int kp = 0; kp < GetArrayLength(GLOBAL_STRUCTVIZ_KEYPRESS_HANDLER_DATA); kp++) {
          GlobalKeyPressHandlerData_t kpHandler = GLOBAL_STRUCTVIZ_KEYPRESS_HANDLER_DATA[kp];
	  if((kpHandler.modifierStateKey != MODKEY_NONE && kpHandler.modifierStateKey == keyState) && 
	     (kpHandler.keyPressValue != KEYVALUE_NONE && kpHandler.keyPressValue == keyPressed) && 
	     (kpHandler.charKeyValue != CHARKEY_NONE && 
	      kpHandler.checkOnlyFirstTextChar && charTextLength > 0 && (int) keyCharText[0] == kpHandler.charKeyValue)) {
	       TerminalText::PrintDebug("Global keypress recognized %s : %s\n", 
			                kpHandler.keyPressComboDesc, kpHandler.keyPressIntentDesc);
	       kpHandler.onPressActionFunc();
	       break;
	  }
     }
     TerminalText::PrintDebug("Non-globally handled key pressed (#%d)\n", keyPressed);
     return 1;

}
