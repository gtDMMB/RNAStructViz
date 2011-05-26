#include <FL/Fl_ask.h>
#include "StructureManager.h"
#include "MainWindow.h"
#include "RNAStructViz.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

StructureManager::StructureManager()
    : m_structureCount(0)
    , m_structures(0)
    , m_referenceIndex(-1)
{
}

StructureManager::~StructureManager()
{
    for (int i = m_structureCount; i > 0; --i)
	RemoveStructure(i - 1);
    free(m_structures);
}

void StructureManager::AddFile(const char* filename)
{
    if (!filename)
	return;

    char* localCopy = strdup(filename);
    if (!localCopy)
	return;

    // Strip any trailing directory markers. Maybe we could load all files in the directory in this case,
    // but it hardly seems worth the effort.
    while (strlen(localCopy) > 0 && localCopy[strlen(localCopy) - 1] == '/')
    	localCopy[strlen(localCopy) - 1] = 0;

    // Get the base file name
    const char* basename = strrchr(localCopy, '/');
    if (!basename)
	basename = localCopy;
    else
	basename++;

    // TODO: Check for duplicate file name.

    // Figure out what kind of file we have and try to load it.
    const char* extension = strrchr(basename, '.');
    RNAStructure* structure = 0;
    if (extension && !strncmp(extension, ".bpseq", 6))
    {
	structure = RNAStructure::CreateFromFile(localCopy, true);
    }
    else if (extension && !strncmp(extension, ".ct", 3))
    {
	structure = RNAStructure::CreateFromFile(localCopy, false);
    }
    else
    {
	if (strlen(filename) > 1000)
	    fl_message("Unknown file type: <file name too long>");
	else
	    fl_message("Unknown file type: %s", filename);
	return;
    }

    if (structure)
    {
	// TODO: Check for duplicates.

	int index = AddFirstEmpty(structure);
	MainWindow::AddStructure(structure->GetFilename(), index, false);
    }

    free(localCopy);
}

void StructureManager::RemoveStructure(const int index)
{
    if (index == m_referenceIndex)
    {
	SetReferenceStructure(-1);
    }

    RNAStructure* structure = m_structures[index];
    m_structures[index] = 0;
    delete structure;
}

void StructureManager::SetReferenceStructure(const int index)
{
    if (index != m_referenceIndex)
    {
	m_referenceIndex = index;
    }
}

int StructureManager::AddFirstEmpty(RNAStructure* structure)
{
    if (!m_structures)
    {
	m_structures = (RNAStructure**)malloc(sizeof(RNAStructure*));
	m_structures[0] = structure;
	m_structureCount = 1;
	return 0;
    }

    for (int i = 0; i < m_structureCount; ++i)
    {
	if (!m_structures[i])
	{
	    m_structures[i] = structure;
	    return i;
	}
    }

    m_structureCount++;
    m_structures = (RNAStructure**)realloc(m_structures, sizeof(RNAStructure*) * m_structureCount);
    m_structures[m_structureCount - 1] = structure;
    return m_structureCount - 1;
}

void StructureManager::DisplayFileContents(const int index)
{
    m_structures[index]->DisplayFileContents();
}

void StructureManager::ClearTruthValues()
{
    for (int i = 0; i < m_structureCount; ++i)
    {
	RNAStructure* structure = m_structures[i];
	if (!structure)
	    continue;
	int length = structure->GetLength();
	for (int j = 0; j < length; ++j)
	{
	    structure->GetBaseAt(j)->m_truth = RNAStructure::TruePositive;
	}
    }
}

void StructureManager::UpdateTruthValues()
{
    if (m_referenceIndex == -1)
    {
	ClearTruthValues();
	return;
    }

    RNAStructure* refStructure = m_structures[m_referenceIndex];
    int length = refStructure->GetLength();
    for (int j = 0; j < length; ++j)
    {
	refStructure->GetBaseAt(j)->m_truth = RNAStructure::TruePositive;
    }

    for (int i = 0; i < m_structureCount; ++i)
    {
	RNAStructure* otherStructure = m_structures[i];
	if (!otherStructure || otherStructure == refStructure)
	    continue;
	for (int j = 0; j < length; ++j)
	{
	    RNAStructure::BaseData* refBase = refStructure->GetBaseAt(j);
	    RNAStructure::BaseData* otherBase = refStructure->GetBaseAt(j);
	    if (refBase->m_pair == RNAStructure::UNPAIRED)
	    {
		if (otherBase->m_pair == RNAStructure::UNPAIRED)
		{
		    otherBase->m_truth = RNAStructure::TrueNegative;
		}
		else
		{
		    otherBase->m_truth = RNAStructure::FalsePositive;
		}
	    }
	    else
	    {
		if (otherBase->m_pair == RNAStructure::UNPAIRED)
		{
		    otherBase->m_truth = RNAStructure::FalseNegative;
		}
		else if (otherBase->m_pair == refBase->m_pair)
		{
		    otherBase->m_truth = RNAStructure::TruePositive;
		}
		else
		{
		    otherBase->m_truth = RNAStructure::FalsePositive;
		}
	    }
	}
    }
}

