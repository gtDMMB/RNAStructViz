/*
    A class that manages the RNA structures that have been loaded.
*/

#ifndef STRUCTUREMANAGER_H
#define STRUCTUREMANAGER_H

#include "RNAStructure.h"

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

    /*
	Get the reference structure.

	An index of -1 means there is no reference structure.
    */
    inline int GetReferenceStructure()
    {
	return m_referenceIndex;
    }

    /*
	Set the reference structure.

	Giving an index of -1 clears the reference structure.
    */
    void SetReferenceStructure(const int index);

    /*
	Popup (or bring to front) a window displaying the file contents.
    */
    void DisplayFileContents(const int index);

private:
    int AddFirstEmpty(RNAStructure* structure);

    void ClearTruthValues();
    void UpdateTruthValues();

    // The allocated size of m_structures
    int m_structureCount;

    // A non-packed array of pointers to structures. Can have 0 entries.
    RNAStructure** m_structures;

    // Index of the reference sequence. Can be -1 for no sequence.
    int m_referenceIndex;
};

#endif STRUCTUREMANAGER_H
