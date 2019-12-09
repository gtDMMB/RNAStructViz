#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <FL/names.h>

#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "FolderStructure.h"
#include "ConfigParser.h"
#include "BaseSequenceIDs.h"
#include "TerminalPrinting.h"
#include "ConfigParser.h"

vector<RNAStructure *> RNAStructViz::ScheduledDeletion::rnaStructObjs;
vector<Folder *> RNAStructViz::ScheduledDeletion::folderStructObjs;
vector<StructureData *> RNAStructViz::ScheduledDeletion::structureDataObjs;
vector<Fl_Widget *> RNAStructViz::ScheduledDeletion::widgetObjs;

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
    //Fl::visual(FL_RGB8 | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE);
    Fl::visual(FL_RGB);
    //Fl::gl_visual(FL_RGB | FL_DEPTH | FL_DOUBLE | FL_ALPHA | FL_MULTISAMPLE);
}

RNAStructViz::~RNAStructViz()
{
    Delete(m_structureManager, StructureManager);
    for (unsigned int i = 0; i < m_diagramWindows.size(); ++i)
         Delete(m_diagramWindows[i], DiagramWindow);
    for (unsigned int i = 0; i < m_statsWindows.size(); ++i)
         Delete(m_statsWindows[i], StatsWindow);
    Fl::remove_timeout(RNAStructViz::ScheduledDeletion::PerformScheduledDeletion, NULL);
    RNAStructViz::ScheduledDeletion::PerformScheduledDeletion(NULL);
}

bool RNAStructViz::Initialize(int argc, char** argv)
{
    if (!ms_instance) {
        ms_instance = new RNAStructViz();
        MainWindow::Initialize(argc, argv);
    }
    return ConfigParser::ParseAutoloadStructuresDirectory();
}

void RNAStructViz::Shutdown()
{
    Fl::remove_timeout(RNAStructViz::ScheduledDeletion::PerformScheduledDeletion, NULL);
    RNAStructViz::ScheduledDeletion::PerformScheduledDeletion(NULL);
    ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
    Delete(RNAStructViz::ms_instance, RNAStructViz);
    MainWindow::Shutdown();
    Delete(RNAStructure::m_ctFileSelectionWin, InputWindow);
    Delete(StatsWindow::overviewLegendImage, Fl_RGB_Image);
}

void RNAStructViz::ExitApplication(bool promptUser) {
     if(!promptUser) {
          RNAStructViz::Shutdown();
      return;
     }
     const char *promptQMsg = "Are you really sure you want to quit? All unsaved data and loaded structures will be lost!";
     int userResp = fl_choice("%s", "Yes, QUIT!", "No, CANCEL", NULL, promptQMsg);
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
        m_diagramWindows.erase(m_diagramWindows.begin(), m_diagramWindows.end());
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
	    diagram->redraw();
            return;
        }
    }
    
    char* title = (char*) malloc(DEFAULT_TITLE_STRING_SIZE * sizeof(char));
    snprintf(title, DEFAULT_TITLE_STRING_SIZE, "Structure Diagram %lu", m_diagramWindows.size() + 1);
    diagram = new DiagramWindow(3 * DiagramWindow::ms_menu_width, 
                                IMAGE_HEIGHT + GLWIN_TRANSLATEY + 35, 
                                title, structures);
    diagram->SetFolderIndex(index);
    diagram->setAsCurrentDiagramWindow();
    m_diagramWindows.push_back(diagram);
    diagram->show();
    diagram->redraw();
    Free(title); 
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
        if(m_structureManager->GetStructure(folders[index]->folderStructs[(i + shift)]))
        {
            structures.push_back(folders[index]->folderStructs[(i + shift)]);
        }
    }
    
    StatsWindow* stats = NULL;
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
    
    char *title = (char *) malloc(DEFAULT_TITLE_STRING_SIZE * sizeof(char));
    snprintf(title, DEFAULT_TITLE_STRING_SIZE, "Structure Statistics %lu", m_statsWindows.size() + 1);
    stats = new StatsWindow(DEFAULT_STATSWIN_WIDTH, DEFAULT_STATSWIN_HEIGHT, title, structures);
    Free(title); 
    
    stats->SetFolderIndex(index);
    m_statsWindows.push_back(stats);
    stats->show();
    
}

void RNAStructViz::RemoveStructure(int folderIndex, int structureIndex) {
     if(folderIndex < 0) {
          return;
     }
     int dwinIdx = GetDiagramWindowForFolderIndex(folderIndex);
     if(dwinIdx >= 0) {
          m_diagramWindows[dwinIdx]->RemoveStructure(structureIndex);
     }
     int swinIdx = GetStatsWindowForFolderIndex(folderIndex);
     if(swinIdx >= 0) {
          m_statsWindows[swinIdx]->RemoveStructure(structureIndex);
     }
}

void RNAStructViz::RemoveFolderData(int index) {

     if(index < 0) {
          return;
     }
     int diagramWinIdx = GetDiagramWindowForFolderIndex(index);
     DiagramWindow *diagramWin = diagramWinIdx >= 0 ? m_diagramWindows[diagramWinIdx] : NULL;
     if(diagramWin != NULL) {
          diagramWin->hide();
          m_diagramWindows.erase(m_diagramWindows.begin() + diagramWinIdx);
     }
     int statsWinIdx = GetStatsWindowForFolderIndex(index);
     StatsWindow   *statsWin = statsWinIdx >= 0 ? m_statsWindows[statsWinIdx] : NULL;
     if(statsWin != NULL) {
	  statsWin->hide();
          m_statsWindows.erase(m_statsWindows.begin() + statsWinIdx);
     }
     while((diagramWin != NULL && diagramWin->visible()) || 
           (statsWin != NULL && statsWin->visible())) {
          Fl::wait(1.0);
     }
     if(USE_SCHEDULED_DELETION) {
          ScheduledDeletion::AddWidget(diagramWin);
	  ScheduledDeletion::AddWidget(statsWin);
     }
     else {
          Delete(diagramWin, DiagramWindow);
          Delete(statsWin, StatsWindow);
     }

}

void RNAStructViz::TestFolders()
{
    m_structureManager->PrintFolders();
    TerminalText::PrintDebug("Number of Diagram / Stats Windows: %d/%d\n\n", 
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

std::string RNAStructViz::LocateSampleStructuresOnSystem() {
     
     bool isApplePlatform = false;
     #ifdef __TARGETOS_APPLE__
          isApplePlatform = true; // brew folders are easy to locate!
     #endif
     if(isApplePlatform) {
      const char *brewStructDir = "/usr/local/opt/rnastructviz/sample-structures";
          if(ConfigParser::directoryExists(brewStructDir)) {
               return std::string(brewStructDir);
      }
     }
     else {
      const char *unixSudoInstallDir = "/usr/local/share/RNAStructViz/sample-structures";
      if(ConfigParser::directoryExists(unixSudoInstallDir)) {
           return std::string(unixSudoInstallDir);
      }
     }
     char curCWDPath[MAX_BUFFER_SIZE];
     getcwd(curCWDPath, MAX_BUFFER_SIZE);
     std::string cwdPath = std::string(curCWDPath);
     size_t srcDirPos = cwdPath.find("src");
     if(srcDirPos == std::string::npos) {
          TerminalText::PrintWarning("Unable to locate sample structures on platform I [%s]\n", TARGETOS);
      return "";
     }
     std::string prefixPath = cwdPath.substr(0, srcDirPos - 1);
     prefixPath += std::string("/sample-structures");
     if(ConfigParser::directoryExists(prefixPath.c_str())) {
          return prefixPath;
     }      
     TerminalText::PrintWarning("Unable to locate sample structures on platform II [%s] : %s\n", TARGETOS, prefixPath.c_str());
     return "";

}

int RNAStructViz::CopySampleStructures() {
     
     std::string sampleStructDir = RNAStructViz::LocateSampleStructuresOnSystem();
     if(sampleStructDir.length() == 0) {
          return EINVAL;
     }
     if(ConfigParser::directoryExists(USER_SAMPLE_STRUCTS_PATH.c_str())) {
          TerminalText::PrintInfo("User home sample structure directory already exists: \"%s\"\n", 
                                  USER_SAMPLE_STRUCTS_PATH.c_str());
     }
     else {
	  fs::path lastDirPath(USER_SAMPLE_STRUCTS_PATH);
          if(!fs::create_directories(lastDirPath)) {
               TerminalText::PrintError("Unable to create user home sample directory: \"%s\"\n", 
				         USER_SAMPLE_STRUCTS_PATH.c_str());
               return -1;
          }
     }
     strcpy((char *) CTFILE_SEARCH_DIRECTORY, USER_SAMPLE_STRUCTS_PATH.c_str());
     ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);

     // have system path, made local directory, now copy the files:
     fs::directory_iterator ssFile(sampleStructDir.c_str());
     boost::filesystem::path const & destDir(USER_SAMPLE_STRUCTS_PATH.c_str());
     while(ssFile != fs::directory_iterator()) {
           fs::path ssCurPath(ssFile->path());
           if(!fs::is_directory(ssCurPath)) {
                try {
                     fs::copy_file(ssCurPath, destDir / ssCurPath.filename());
                } catch(boost::filesystem::filesystem_error fse) {
                     TerminalText::PrintInfo("Cannot copy \"%s\" into home directory (file already exists)\n", 
                                             ssCurPath.filename().c_str());
                }
           }
           ++ssFile;
     }
     std::string successMsg = std::string("Successfully copied the sample structure files! ") + 
                  std::string("They are now located in \n« ") + 
                  std::string(USER_SAMPLE_STRUCTS_PATH) + 
                  std::string(" ». \nClick on the \"Load Files\" button to start selecting ") + 
                  std::string("the sample \nstructures we just copied!");
     fl_message("%s", successMsg.c_str());

     return EXIT_SUCCESS;

}

int RNAStructViz::BackupAndUnlinkLocalConfigFiles(bool clearStickyFolderNamesOnly) {
     time_t timeOutput = time(NULL);
     struct tm *curTimeStruct = localtime(&timeOutput);
     char dateStampSuffix[MAX_BUFFER_SIZE];
     strftime(dateStampSuffix, MAX_BUFFER_SIZE, "-%F-%H%M%S.bak", curTimeStruct);
     std::string configPath = std::string(USER_CONFIG_PATH);
     std::string configBackupPath = std::string(USER_CONFIG_PATH) + std::string(dateStampSuffix);
     std::string stickyFolderPath = std::string(GetStickyFolderConfigPath(DEFAULT_STICKY_FOLDERNAME_CFGFILE));
     std::string stickyFolderBackupPath = 
             std::string(GetStickyFolderConfigPath(DEFAULT_STICKY_FOLDERNAME_CFGFILE)) + 
             std::string(dateStampSuffix);
     if(!clearStickyFolderNamesOnly && rename(configPath.c_str(), configBackupPath.c_str()) || 
        rename(stickyFolderPath.c_str(), stickyFolderBackupPath.c_str())) {
          TerminalText::PrintInfo("Unable to rename backup config files \"%s\" and/or \"%s\" : %s\n", 
                                  configBackupPath.c_str(), stickyFolderPath.c_str(), strerror(errno));
          return errno;
     }
     return EXIT_SUCCESS;
}

bool RNAStructViz::CopyStructureFileToAutoloadDirectory(const char *structFileBaseName, const char *structFileDiskPath) {
     std::string autoloadStructPath = std::string(USER_AUTOLOAD_PATH) + std::string(structFileBaseName); 
     if(ConfigParser::fileExists(autoloadStructPath.c_str()) && 
	ConfigParser::fileExists(structFileDiskPath)) {
          return true;
     }
     try {
	  fs::path toPath(autoloadStructPath.c_str()), fromPath(structFileDiskPath);
	  fs::create_symlink(fromPath, toPath);
	  return true;
     } catch(fs::filesystem_error fse) {
          TerminalText::PrintWarning("Unable to copy file \"%s\" to autoload directory: %s\n", 
			             structFileDiskPath, fse.what());
	  return false;
     }
}

bool RNAStructViz::RemoveStructureFileFromAutoloadDirectory(const char *structFileBaseName) {
     std::string autoloadStructPath = std::string(USER_AUTOLOAD_PATH) + std::string(structFileBaseName);
     if(!ConfigParser::fileExists(autoloadStructPath.c_str(), false)) {
          return true;
     }
     try {
	  fs::path removePath(autoloadStructPath.c_str());
	  fs::remove(removePath);
	  return !ConfigParser::fileExists(autoloadStructPath.c_str(), false);
     } catch(fs::filesystem_error fse) {
	  TerminalText::PrintWarning("Unable to remove file \"%s\": %s\n", 
			             structFileBaseName, fse.what());
	  return false;
     }

}
