#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <FL/fl_ask.H>

#include "StructureManager.h"
#include "FolderStructure.h"
#include "FolderWindow.h"
#include "MainWindow.h"
#include "RNAStructViz.h"
#include "InputWindow.h"
#include "BaseSequenceIDs.h"
#include "TerminalPrinting.h"

StructureManager::StructureManager()
    : m_structureCount(0), m_structures(NULL), m_inputWindow(NULL) {}

StructureManager::~StructureManager()
{
    for (int i = m_structureCount; i > 0; --i)
    {
        RemoveStructure(i - 1);
    }
    Free(m_structures); 
    for(int i = 0; i < (int)folders.size(); i++)
    {
         folders[i]->SetPerformWidgetDeletion(false);
         Delete(folders[i], Folder);
    }
    Delete(m_inputWindow, InputWindow);
}

void StructureManager::AddFile(const char* filename, bool removeDuplicateStructs, bool guiQuiet)
{
    if (!filename)
        return;

    char* localCopy = strdup(filename);
    if (!localCopy) {
        return;
    }

    // Get the base file name
    const char* basename = strrchr(localCopy, '/');
    if (!basename)
        basename = localCopy;
    else
        basename++;

    // Don't load if it shares a filename with another loaded structure
    for (int i = 0; i < m_structureCount; ++i)
    {
        if (m_structures[i])
        {
            if (!strcmp(m_structures[i]->GetFilename(true), basename))
            {
                TerminalText::PrintInfo("Skipping ... Already have a structure loaded with filename: %s\n", 
                                        basename);
                free(localCopy);
                return;
            }
        }
    }
    
    // Figure out what kind of file we have and try to load it.
    const char* extension = strrchr(basename, '.');
    extension = extension ? extension : "";
    RNAStructure **structures = (RNAStructure **) malloc(sizeof(RNAStructure *));
    int newStructCount = 0;
    bool isFASTAFile = false;
    if (extension && !strncasecmp(extension, ".bpseq", 6)) {
        *structures = RNAStructure::CreateFromFile(localCopy, true);
        newStructCount = 1;
    }
    else if (extension && !strncasecmp(extension, ".ct", 3)) {
        *structures = RNAStructure::CreateFromFile(localCopy, false);
        newStructCount = 1;
    }
    else if (extension && !strncasecmp(extension, ".nopct", 6)) {
        *structures = RNAStructure::CreateFromFile(localCopy, false);
        newStructCount = 1;
    }
    else if(extension && (!strncasecmp(extension, ".dot", 4) || 
              !strncasecmp(extension, ".bracket", 8) || 
              !strncasecmp(extension, ".dbn", 4))) {
        *structures = RNAStructure::CreateFromDotBracketFile(localCopy);
        newStructCount = 1;
    }
    else if(extension && !strncasecmp(extension, ".boltz", 6)) {
        structures = RNAStructure::CreateFromBoltzmannFormatFile(localCopy, &newStructCount);
    }
    else if(extension && (!strncasecmp(extension, ".helix", 6) || 
              !strncasecmp(extension, ".hlx", 4))) {
        structures = RNAStructure::CreateFromHelixTripleFormatFile(localCopy, &newStructCount);
    }
    #if WITH_FASTA_FORMAT_SUPPORT > 0
    else if(extension && !strncasecmp(extension, ".fasta", 6)) {
        structures = RNAStructure::CreateFromFASTAFile(localCopy, &newStructCount);
	isFASTAFile = true;
    }
    #endif
    else {
        if (strlen(filename) > 1000) {
	    if(!guiQuiet) {
                 fl_message("Unknown file type: <file name too long>");
	    }
	    else {
		 TerminalText::PrintWarning("Unknown file type: <file name too long>");
	    }
	}
        else {
	    if(!guiQuiet) {
                 fl_message("Unknown file type: %s . %s [%s]", filename, extension, basename);
	    }
	    else {
		 TerminalText::PrintWarning("Unknown file type: %s . %s [%s]", filename, extension, basename);
	    }
	}
        return;
    }

    if(structures != NULL && newStructCount > 0)
    {
        for(int s = 0; s < newStructCount; s++) { 
        
           int count = (int) folders.size();
           RNAStructure *structure = structures[s];
           if(!structure) { 
                continue;
           }
           int firstEmptyIdx = AddFirstEmpty(structure);
           if(count == (int) folders.size()-1) // we added a new folder ... 
           {
                 
              off_t stickyFolderExists = FolderNameForSequenceExists(
                                      DEFAULT_STICKY_FOLDERNAME_CFGFILE, 
                                      structure
                     );
              if(stickyFolderExists != LSEEK_NOT_FOUND && GUI_KEEP_STICKY_FOLDER_NAMES) {
                    char *stickyFolderName = LookupStickyFolderNameForSequence(
                                    DEFAULT_STICKY_FOLDERNAME_CFGFILE, 
                                    stickyFolderExists
                           );
                     if(stickyFolderName != NULL && !FolderNameExists(stickyFolderName)) {
                          strcpy(folders[count]->folderName, stickyFolderName);
                          Free(stickyFolderName);
                          MainWindow::AddFolder(folders[count]->folderName, count, false);
                          continue;
                     }
                     Free(stickyFolderName);
                 }
             
	         if(m_inputWindow != NULL) {
	              Delete(m_inputWindow, InputWindow);
		 }
                 m_inputWindow = new InputWindow(525, 210, 
                                                 "New Folder Added", folders[count]->folderName, 
                                                  InputWindow::FOLDER_INPUT);
                 while (m_inputWindow->visible()) {
                       Fl::wait();
                 }
            
                 bool same = false;
                 for(unsigned int ui = 0; ui < folders.size(); ui++)
                 {
                     if (!strcmp(folders[ui]->folderName, m_inputWindow->getName()) && 
                         strcmp(m_inputWindow->getName(), "")) {
                          same = true;
                          break;
                     }
                 }
            
		 bool skipLoadingFile = false;
                 while(same) {
		     if(!guiQuiet) {
                         int choice = fl_choice("Already have a folder with the name: %s, please choose another name.", 
                                                "Skip loading file", "Close", NULL, m_inputWindow->getName());
                         m_inputWindow->Cleanup(false);
                         if(choice == 0) {
                             same = false;
			     skipLoadingFile = true;
                             break;
                         }
		     }
		     else {
		         skipLoadingFile = true;
			 break;
		     }
                     if(m_inputWindow != NULL) {
		          Delete(m_inputWindow, InputWindow);
		     }
		     m_inputWindow = new InputWindow(525, 210, "New Folder Added", 
                                                     folders[count]->folderName, InputWindow::FOLDER_INPUT);
                     while (m_inputWindow->visible()) {
                          Fl::wait();
                     }
                     same = !strcmp(m_inputWindow->getName(), "");
                     for(unsigned int ui = 0; ui < folders.size(); ui++)
                     {
                           if (!strcmp(folders[ui]->folderName, m_inputWindow->getName()))
                           {
                             same = true;
                             break;
                           }
                     }
                 }
                        
                 if(!skipLoadingFile && m_inputWindow != NULL && strcmp(m_inputWindow->getName(), "")) {
                     strcpy(folders[count]->folderName, m_inputWindow->getName());
                     if(GUI_KEEP_STICKY_FOLDER_NAMES) {
                     const char *baseSeq = structure->GetSequenceString();
                     int saveStatus = SaveStickyFolderNameToConfigFile(
                        DEFAULT_STICKY_FOLDERNAME_CFGFILE, 
                        std::string(baseSeq), 
                        std::string(folders[count]->folderName), 
                        LSEEK_NOT_FOUND
                     );
                     if(saveStatus) {
                          TerminalText::PrintWarning("Unable to save sticky folder name \"%s\"\n", 
                                                     folders[count]->folderName);
                     }
                     else {
                          TerminalText::PrintDebug("Saved sticky folder name \"%s\" to local config file\n", 
                                                   folders[count]->folderName);
                     }
               }
           }
           bool haveDuplicateStruct = false;
	   Folder *nextFolder = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(count);
	   if(nextFolder != NULL) {
                for(int si = 0; si < nextFolder->structCount; si++) {
                     int structIdx = nextFolder->folderStructs[si];
		     RNAStructure *compStruct = structIdx != -1 ? 
			                        RNAStructViz::GetInstance()->GetStructureManager()->GetStructure(structIdx) : 
						NULL;
		     if(compStruct == NULL) {
		          continue;
		     }
		     else if(*compStruct == *structure) {
		          haveDuplicateStruct = true;
		          break;
		     }
		}
	   }
	   bool okToLoad = !haveDuplicateStruct || !removeDuplicateStructs;
           if(!skipLoadingFile && okToLoad && m_inputWindow != NULL) {
	      MainWindow::AddFolder(folders[count]->folderName, count, false);
              m_inputWindow->Cleanup(false);
	      while(m_inputWindow->visible()) { Fl::wait(); }
              Delete(m_inputWindow, InputWindow);
           }
	   else if(firstEmptyIdx != -1) {
                m_structures[firstEmptyIdx] = NULL;
		m_structureCount -= 1;
	   }
       }

      }
    }
    else if(isFASTAFile) {
	 TerminalText::PrintWarning("Skipping loading of FASTA / TXT file data from \"%s\" ...", localCopy);
    
    }
    else {
         fl_alert("Error adding structure \"%s\"! Could not parse the specified format for this file.\n", 
                  localCopy);
    }
    Free(localCopy); 
    Free(structures);

}

void StructureManager::RemoveStructure(const int index)
{
    RNAStructure* structure = m_structures[index];
    m_structures[index] = NULL;

    bool found = false;
    for(int i = 0; i < (int)folders.size(); i++)
    {
        int shift = 0;
        for(int j = 0; j < folders[i]->structCount; j++)
        {   
            if(folders[i]->folderStructs[(j+shift)] == -1)
            {
                shift++;
            }
            if(folders[i]->folderStructs[(j+shift)] == index)
            {
                folders[i]->folderStructs[(j+shift)] = -1;                
                found = true;
                break;
            }
        }
        if(found)
            break;
    }
    Delete(structure, RNAStructure);
    
}

void StructureManager::DecreaseStructCount(const int index)
{
    folders[index]->structCount = folders[index]->structCount - 1;
    if (folders[index]->structCount == 0) 
    {
        MainWindow::RemoveFolderByIndex(index, true);
    }
    else {
        folders[index]->SetTooltipTextData();
	folders[index]->SetFolderLabel();
	Fl::redraw();
    }
}

void StructureManager::RemoveFolder(const int folder, const int index) {
    Delete(folders[index], Folder);
    folders.erase(folders.begin() + index);
}

void StructureManager::AddFolder(RNAStructure* structure, const int index) {
     Folder *nextFolder = Folder::AddNewFolderFromData(structure, index, false);
     folders.push_back(nextFolder);
}

int StructureManager::AddFirstEmpty(RNAStructure* structure)
{
    int index;
    bool added = false;
    bool found = false;
    if (!m_structures && folders.empty())
    {
        m_structures = (RNAStructure**) malloc(sizeof(RNAStructure*));
        m_structures[0] = structure;
        m_structureCount = 1;
        added = true;
        AddFolder(structure, 0);
        found = true;
        return 0;
    }
    for (int i = 0; i < m_structureCount; ++i)
    {
        if (!m_structures[i])
        {
            m_structures[i] = structure;
            index = i;
            added = true;
            break;
        }
    }
    if(!added)
    {
        m_structureCount++;
        m_structures = (RNAStructure **) realloc(m_structures, sizeof(RNAStructure*) * m_structureCount);
        m_structures[m_structureCount - 1] = structure;
        index = m_structureCount - 1;
    }
    
    for(unsigned int ui = 0; ui < folders.size(); ui++)
    {
        for(int j = 0; j < folders[ui]->structCount; j++)
        {

            if(folders[ui]->folderStructs[j] > -1 &&
               SequenceCompare(m_structures[folders[ui]->folderStructs[j]], structure))
            {

                folders[ui]->structCount++;
                if(folders[ui]->structCount > folders[ui]->capacity + 1) {
		    folders[ui]->capacity *= 2;
                    folders[ui]->folderStructs = (int *) realloc(folders[ui]->folderStructs, 
                                                                 sizeof(int) * folders[ui]->capacity);
                }
                bool emptySlot = false;
                for(int i = 0; i < folders[ui]->structCount-1; i++)
                {
                    if(folders[ui]->folderStructs[i] == -1)
                    {
                        folders[ui]->folderStructs[i] = index;
                        emptySlot = true;
                        break;
                    }
                }
                if(!emptySlot)
                    folders[ui]->folderStructs[folders[ui]->structCount - 1] = index;
                AddNewStructure(ui, index);
                
                if (folders[ui]->folderNameFileCount != NULL) {
                    folders[ui]->SetTooltipTextData();
	            folders[ui]->SetFolderLabel();
		}
                found = true;
                break;
            }
        }
        if(found)
            break;
    }
    if(!found)
    {
        AddFolder(structure, index);
    }
    return index;

}

void StructureManager::AddNewStructure(const int folderIndex, const int index)
{
    const std::vector<DiagramWindow*>& diagrams = 
        RNAStructViz::GetInstance()->GetDiagramWindows();
    for(unsigned int ui = 0; ui < diagrams.size(); ui++)
    {
        if(diagrams[ui]->GetFolderIndex() == folderIndex)
            diagrams[ui]->AddStructure(index);
    }
    
    const std::vector<StatsWindow*>& stats = RNAStructViz::GetInstance()->GetStatsWindows();
    for(unsigned int ui = 0; ui < stats.size(); ui++)
    {
        if(stats[ui]->GetFolderIndex() == folderIndex)
            stats[ui]->AddStructure(index);
    }
    MainWindow::ShowFolderSelected();
}

bool StructureManager::SequenceCompare(RNAStructure* struct1, RNAStructure* struct2) const
{
    if(struct1->GetCharSeqSize() != struct2->GetCharSeqSize())
        return false;
    const char* ptr1 = struct1->GetCharSeq();
    const char* ptr2 = struct2->GetCharSeq();
    for(unsigned int i = 0; i < struct1->GetCharSeqSize(); i++)
    {
        if(ptr1[i] != ptr2[i])
            return false;
    }
    return true;
}

void StructureManager::DisplayFileContents(const int index, 
                                   const char *displaySuffix)
{
    m_structures[index]->DisplayFileContents(displaySuffix);
}

void StructureManager::PrintFolders()
{
    for(int i = 0; i < (int)folders.size(); i++)
    {
        int shift = 0;
        printf("folder: %s\n", folders[i]->folderName);
        for(int j = 0; j < folders[i]->structCount; j++)
        {
            if(folders[i]->folderStructs[(j+shift)] == -1)
            {
                shift++;
            }
            if(m_structures[folders[i]->folderStructs[(j+shift)]])
                printf("\tstruct %d: %s\n", (j+shift), m_structures[folders[i]->folderStructs[(j+shift)]]->GetFilename());
        }
    }
}

