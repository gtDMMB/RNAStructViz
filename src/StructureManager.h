/*
    A class that manages the RNA structures that have been loaded.
*/

#ifndef STRUCTUREMANAGER_H
#define STRUCTUREMANAGER_H

#include "RNAStructure.h"
#include "FolderStructure.h"
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <vector>

class StructureManager
{
    public:
        StructureManager();
        ~StructureManager();

        /*
	    Add a new structure, given as a file name to load.
        */
        void AddFile(const char* filename);

        /*
	    Remove a structure.
        */
        void RemoveStructure(const int index);
    
        /*
         Remove a folder.
        */
        void RemoveFolder(const int folder, const int index);

        /*
	    Get the number of structures. Some may be NULL.
        */
        inline int GetStructureCount() const
        {
	        return m_structureCount;
        }

        /*
	     Get a structure.
        */
        inline RNAStructure* GetStructure(const int index)
        {
	        return m_structures[index];
        }

	inline RNAStructure* LookupStructureByCTPath(const char *ctPath) {
	     if(ctPath == NULL) {
	          return NULL;
	     }
	     for(int sidx = 0; sidx < m_structureCount; sidx++) {
	          RNAStructure *rnaStruct = GetStructure(sidx);
		  if((rnaStruct != NULL) && !strcmp(rnaStruct->GetFilename(), ctPath)) {
	               return rnaStruct;
		  }
	     }
	     return NULL;
	}

        /*
	    Popup (or bring to front) a window displaying the file contents.
        */
        void DisplayFileContents(const int index, 
			         const char *displaySuffix = NULL);
    
        /*
         Update Diagram Window, Stats Window, and Folder Window for a folder.
         */
        void AddNewStructure(const int folderIndex, const int index);
    
        inline Folder* GetFolderAt(int index)
        {
            return folders[index];
        }
    
        /*
         Returns a pointer to all the folders.
         */
        inline const std::vector<Folder*>& GetFolders() const
        {
            return folders;
        }
    
        /*
         Decrease the structCount of a folder by 1;
         */
        void DecreaseStructCount(const int index);
    
        /*
         Function for testing purposes.
         */
        void PrintFolders();

    private:
        /*
         Add structures to program
         Initializes folders and m_structures on the first occurance
         */
        int AddFirstEmpty(RNAStructure* structure);
    
        // Creates a new folder for that structure
        void AddFolder(RNAStructure* structure, const int index);
    
        // The allocated size of m_structures
        int m_structureCount;

        // A non-packed array of pointers to structures. Can have 0 entries.
        RNAStructure** m_structures;
    
        // Vector of folders
        std::vector<Folder*> folders;
    
        /*
         Compares two RNAStructures, if the sequences of the structures are the same
         returns true, and false otherwise.
         */
        bool SequenceCompare(RNAStructure* struct1, RNAStructure* struct2) const;

};

#endif //STRUCTUREMANAGER_H
