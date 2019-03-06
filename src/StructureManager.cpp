#include <FL/fl_ask.H>
#include "StructureManager.h"
#include "FolderStructure.h"
#include "FolderWindow.h"
#include "MainWindow.h"
#include "RNAStructViz.h"
#include "InputWindow.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

StructureManager::StructureManager()
    : m_structureCount(0), m_structures(0) {}

StructureManager::~StructureManager()
{
    for (int i = m_structureCount; i > 0; --i)
    {
		RemoveStructure(i - 1);
	}
    free(m_structures); m_structures = NULL;
    for(int i = 0; i < (int)folders.size(); i++)
    {
        free(folders[i]->folderStructs); folders[i]->folderStructs = NULL;
        free(folders[i]);
    }
}

void StructureManager::AddFile(const char* filename)
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
            if (!strcmp(m_structures[i]->GetFilename(), basename))
            {
            	fl_message("Already have a structure loaded with filename: %s", 
			   basename);
            	free(localCopy);
		return;
            }
        }
    }
    
    // Figure out what kind of file we have and try to load it.
    const char* extension = strrchr(basename, '.');
    extension = extension ? extension : "";
    RNAStructure* structure = NULL;
    if (extension && !strncmp(extension, ".bpseq", 6))
    {
		structure = RNAStructure::CreateFromFile(localCopy, true);
    }
    else if (extension && !strncmp(extension, ".ct", 3))
    {
		structure = RNAStructure::CreateFromFile(localCopy, false);
    }
    else if (extension && !strncmp(extension, ".nopct", 6))
    {
		structure = RNAStructure::CreateFromFile(localCopy, false);
    }
    else
    {
		if (strlen(filename) > 1000)
		    fl_message("Unknown file type: <file name too long>");
		else
		    fl_message("Unknown file type: %s . %s [%s]", filename, extension, basename);
		return;
    }

    if (structure)
    {
    	int count = (int)folders.size();
	AddFirstEmpty(structure);
        if(count == (int) folders.size()-1)
        {
            InputWindow* input_window = new InputWindow(400, 150, 
			 "New Folder Added", folders[count]->folderName, 
			 InputWindow::FOLDER_INPUT);
            while (input_window->visible() && !GUI_USE_DEFAULT_FOLDER_NAMES)
            {
                Fl::wait();
            }
            
            bool same = false;
            for(unsigned int ui = 0; ui < folders.size(); ui++)
            {
            	if (!strcmp(folders[ui]->folderName,input_window->getName())
                    && strcmp(input_window->getName(),""))
            	{
            		same = true;
            		break;
            	}
            }
            
            while (same) {
                fl_message("Already have a folder with the name: %s, please choose another name.", 
                           input_window->getName());
                delete input_window;
                input_window = new InputWindow(400, 150, "New Folder Added", 
	            	               folders[count]->folderName, InputWindow::FOLDER_INPUT);
                while (input_window->visible() && !GUI_USE_DEFAULT_FOLDER_NAMES)
                {
                    Fl::wait();
                }
                same = false;
                for(unsigned int ui = 0; ui < folders.size(); ui++)
            	{
            		if (!strcmp(folders[ui]->folderName, input_window->getName()))
	            	{
    	        		same = true;
        	    		break;
            		}
	            }
            }
                        
            if(strcmp(input_window->getName(), ""))
            	strcpy(folders[count]->folderName, input_window->getName());

            MainWindow::AddFolder(folders[count]->folderName, count, false);
	    delete input_window;
        }

    }
    free(localCopy); localCopy = NULL;

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
    delete structure;
    
}

void StructureManager::DecreaseStructCount(const int index)
{
    folders[index]->structCount = folders[index]->structCount - 1;
    if (folders[index]->structCount == 0) 
    {
        MainWindow::RemoveFolderByIndex(index, true);
    }
    else {
        sprintf(folders[index]->folderNameFileCount, "(+% 2d) %-.48s", 
                folders[index]->structCount, folders[index]->folderName);
        Fl::redraw();
    }
}

void StructureManager::RemoveFolder(const int folder, const int index)
{
    if (folders[index]->folderName != NULL) {
        free(folders[index]->folderName); 
        folders[index]->folderName = "";
    }
    if (folders[index]->folderNameFileCount != NULL) {
        free(folders[index]->folderNameFileCount); 
        folders[index]->folderNameFileCount = "";
    }
    if (folders[index]->folderStructs != NULL) {
        free(folders[index]->folderStructs); 
        folders[index]->folderStructs = NULL;
    }
    
    if(folders[index]->folderWindow)
    {
        delete folders[index]->folderWindow;
        folders[index]->folderWindow = NULL;
    }
    folders[index]->structCount = 0;
    folders.erase(folders.begin() + index);
}

void StructureManager::AddFolder(RNAStructure* structure, const int index)
{
	Folder *temp = (Folder*)malloc(sizeof(Folder));
    temp->folderName = (char*)malloc(sizeof(char)*64);
    if(strlen(structure->GetFilename()) < 60) {
        strcpy(temp->folderName,structure->GetFilename());
    }
    else {
        strncpy(temp->folderName,structure->GetFilename(),60);
        temp->folderName[50]='\0';
    }
    temp->folderNameFileCount = (char*)malloc(sizeof(char)*72);
    temp->folderStructs = (int*)malloc(sizeof(int)*128);
    temp->folderStructs[0] = index;
    temp->structCount = 1;
    sprintf(temp->folderNameFileCount, "(+% 2d) %-.48s", 
	    temp->structCount, temp->folderName);
    temp->selected = false;
    temp->folderWindow = 0;
    folders.push_back(temp);
    
}

int StructureManager::AddFirstEmpty(RNAStructure* structure)
{
    int index;
    bool added = false;
    bool found = false;
    if (!m_structures && folders.empty())
    {
		m_structures = (RNAStructure**)malloc(sizeof(RNAStructure*));
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
	    m_structures = (RNAStructure**)realloc(m_structures, sizeof(RNAStructure*) * m_structureCount);
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
                if(folders[ui]->structCount >= 128) {
                    folders[ui]->folderStructs = (int*) realloc(folders[ui]->folderStructs, 
                                                  sizeof(int) * folders[ui]->structCount);
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
                    sprintf(folders[ui]->folderNameFileCount, "(+% 2d) %-.48s", 
                            folders[ui]->structCount, folders[ui]->folderName);
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

