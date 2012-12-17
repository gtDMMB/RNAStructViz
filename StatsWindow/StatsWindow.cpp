#include "StatsWindow.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include <unistd.h>
#include <iostream>
#include <FL/Fl_Round_Button.H>

StatsWindow::StatsWindow(int wid, int hgt, const char *label, int folderIndex): Fl_Window(wid, hgt, label)
{
    /*Initialize and set up GUI*/
    //SetStructures(folderIndex);
}

StatsWindow::~StatsWindow()
{

}

void SetStructures(int folderIndex)
{
    /* Refer to SetStructures in FolderWindow.cpp and DiagramWindow.cpp*/
}

void StatsWindow::AddStructure(const char* filename, const int index)
{
    /* Refer to AddStructure in FolderWindow.cpp */
}

void StatsWindow::RemoveStructure(const int index)
{
    /* Refer to RemoveStructure in DiagramWindow.cpp*/
}

void StatsWindow::ReferenceCallback(Fl_Widget* widget, void* userData)
{
    /* Refer to SelectCallback in MainWindow.cpp */
}

void StatsWindow::SetReferenceStructure(const int index)
{
    if (index != referenceIndex)
    {
        referenceIndex = index;
    }
}

/*May not be quite right, modified from orignial code*/
void StatsWindow::ClearTruthValues()
{
    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    Folder* folder = structureManager->GetFolderAt(folderIndex);
    int shift = 0;
    for(int i = 0; i < folder->structCount; i++)
    {
        if(folder->folderStructs[(i + shift)] == -1)
            shift++;
        if(folder->folderStructs[(i + shift)] != -1)
        {
            RNAStructure *structure = structureManager->GetStructure((i+shift));
            if(!structure)
                continue;
            int length = structure->GetLength();
            for (int j = 0; j < length; ++j)
            {
                structure->GetBaseAt(j)->m_truth = RNAStructure::TruePositive;
            }
        }
    }
}
/*May not be quite right, modified from orignial code*/
void StatsWindow::UpdateTruthValues()
{
    if (referenceIndex == -1)
    {
        ClearTruthValues();
        return;
    }
    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    Folder* folder = structureManager->GetFolderAt(folderIndex);
    
    RNAStructure* refStructure = structureManager->GetStructure(referenceIndex);
    int length = refStructure->GetLength();
    for (int j = 0; j < length; ++j)
    {
        refStructure->GetBaseAt(j)->m_truth = RNAStructure::TruePositive;
    }
    
    int shift = 0;
    for (int i = 0; i < folder->structCount; ++i)
    {
        if(folder->folderStructs[(i + shift)] == -1)
            shift++;

        if(folder->folderStructs[(i + shift)] != -1)
        {
            RNAStructure* otherStructure = structureManager->GetStructure((i+shift));
            if (!otherStructure || otherStructure == refStructure)
                continue;
            for (int j = 0; j < length; ++j)
            {
                RNAStructure::BaseData* refBase = refStructure->GetBaseAt(j);
                RNAStructure::BaseData* otherBase = otherStructure->GetBaseAt(j);
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
}

