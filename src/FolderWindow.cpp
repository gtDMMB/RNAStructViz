#include <unistd.h>

#include <iostream>

#include <FL/Fl_Button.H>

#include "FolderWindow.h"
#include "FolderStructure.h"
#include "RNAStructure.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "ThemesConfig.h"
#include "StructureType.h"

#include "pixmaps/RNAStructVizLogo.c"
#include "pixmaps/StructureOperationIcon.c"

vector<StructureData*> FolderWindow::m_storedStructDisplayData;

void FolderWindow::Construct(int w, int h, int folderIndex) {}

FolderWindow::FolderWindow(int x, int y, int wid, int hgt, 
                   const char *label, int folderIndex) : 
          Fl_Group(x, y, wid, hgt, label), 
          folderScroll(NULL), folderPack(NULL), 
          fileOpsLabel(NULL), fileLabel(NULL), 
	  structIconBox(NULL), structureIcon(NULL), 
	  statsButton(NULL), diagramButton(NULL) 
{

    // label configuration happens inside FolderWindow::AddStructure ... 

    // icon configuration:
    structureIcon = new Fl_RGB_Image(StructureOperationIcon.pixel_data, 
            StructureOperationIcon.width, StructureOperationIcon.height, 
            StructureOperationIcon.bytes_per_pixel);
    structureIcon->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.45), 0.45);
    
    structIconBox = new Fl_Box(x, y - 39, structureIcon->w(), structureIcon->h());
    structIconBox->image(structureIcon);

    int dividerTextHeight = 0, spacingHeight = NAVBUTTONS_SPACING;
    int fileOpsLabelHeight = 2 * NAVBUTTONS_BHEIGHT; 
    int fileOpsLabelWidth = 2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING + 4;
    int initYOffset = NAVBUTTONS_OFFSETY + RNAStructVizLogo.height + 5; 
    const char *fileOpsLabelText = "@reload   Structure Operations.\n  Each comparision operation\n  button click opens a new window.";
    fileOpsLabel = new Fl_Box(x - 2 + NAVBUTTONS_SPACING / 2, initYOffset, 
                              fileOpsLabelWidth + 10, fileOpsLabelHeight + spacingHeight / 2, 
                              fileOpsLabelText);
    fileOpsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);  
    fileOpsLabel->color(GUI_BGCOLOR);
    fileOpsLabel->labelcolor(GUI_BTEXT_COLOR);
    fileOpsLabel->labelfont(FL_HELVETICA_BOLD);
    fileOpsLabel->labelsize(LOCAL_TEXT_SIZE);
    fileOpsLabel->box(FL_RSHADOW_BOX);
    fileOpsLabel->labeltype(FL_SHADOW_LABEL);

    int opButtonWidth = 135;
    int yOffset = initYOffset + fileOpsLabelHeight + spacingHeight / 2;
    diagramButton = new Fl_Button(x + NAVBUTTONS_OFFSETX + 7, yOffset + spacingHeight,
                                             opButtonWidth, 30,
                                             "@circle  Diagrams @>|");
    diagramButton->callback(DiagramCallback);
    diagramButton->labelcolor(GUI_BTEXT_COLOR);
    diagramButton->labelfont(FL_HELVETICA);
    diagramButton->box(FL_RSHADOW_BOX);
    //diagramButton->box(FL_PLASTIC_UP_BOX);
    diagramButton->labeltype(FL_SHADOW_LABEL);
    diagramButton->tooltip("Open the arc and radial layout diagram viewer windows");

    statsButton = new Fl_Button(x + NAVBUTTONS_OFFSETX + 7 + opButtonWidth + 
                                           spacingHeight, yOffset + spacingHeight, 
                                           opButtonWidth, 30,
                                           "@square  Statistics @>|");
    statsButton->callback(StatsCallback);
    statsButton->labelcolor(GUI_BTEXT_COLOR);
    statsButton->labelfont(FL_HELVETICA);
    statsButton->box(FL_RSHADOW_BOX);
    //statsButton->box(FL_PLASTIC_UP_BOX);
    statsButton->labeltype(FL_SHADOW_LABEL);
    statsButton->tooltip("Open a window to generate comparitive statistics about the selected sequence");

    const char *fileInstText = "@filenew   Files.\n  Click on the file buttons to view\n  " 
	                       "CT-style structure pairing data\n  in new window.";
    fileLabel = new Fl_Box(x - 2 + NAVBUTTONS_SPACING / 2, y + yOffset + spacingHeight, 
                           fileOpsLabelWidth + 10, fileOpsLabelHeight + spacingHeight, fileInstText);
    fileLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    fileLabel->color(GUI_BGCOLOR);
    fileLabel->labelcolor(GUI_BTEXT_COLOR);
    fileLabel->labelfont(FL_HELVETICA_BOLD);
    fileLabel->labelsize(LOCAL_TEXT_SIZE);
    fileLabel->box(FL_RSHADOW_BOX);
    fileLabel->labeltype(FL_SHADOW_LABEL);

    folderScroll = new Fl_Scroll(x+10, y + yOffset + fileOpsLabelHeight + 
                         dividerTextHeight + 4 * spacingHeight, 
                         280, 310 - 2 * fileOpsLabelHeight - dividerTextHeight - 
                         3 * spacingHeight - NAVBUTTONS_BHEIGHT);
    folderScroll->type(Fl_Scroll::VERTICAL_ALWAYS);

    folderPack = new Fl_Pack(x+10, y + yOffset + fileOpsLabelHeight + 
                             dividerTextHeight + 4 * spacingHeight, 260, 
                             290 - 2 * fileOpsLabelHeight - dividerTextHeight - 
                             3 * spacingHeight - NAVBUTTONS_BHEIGHT);
    folderPack->type(Fl_Pack::VERTICAL);
    folderPack->align(FL_ALIGN_TOP);

    folderScroll->color((Fl_Color) GUI_WINDOW_BGCOLOR);
    folderScroll->labelcolor((Fl_Color) GUI_BTEXT_COLOR);
    
    this->resizable(folderScroll);
    this->color(GUI_WINDOW_BGCOLOR);
    
    SetStructures(folderIndex);

}

FolderWindow::~FolderWindow() {
     //HideFolderWindowGUIDisplay(true);
     label("");
     //remove(folderPack);
     //remove(folderScroll);
     //remove(fileOpsLabel);
     //remove(fileLabel);
     Delete(folderPack, Fl_Pack);
     Delete(folderScroll, Fl_Scroll);
     Delete(fileOpsLabel, Fl_Box);
     Delete(fileLabel, Fl_Box);
     Delete(structIconBox, Fl_Box);
     Delete(structureIcon, Fl_RGB_Image);
     Delete(statsButton, Fl_Button);
     Delete(diagramButton, Fl_Button);
}

void FolderWindow::SetStructures(int folderIndex) {

    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    Folder* folder = structureManager->GetFolderAt(folderIndex);
    m_folderIndex = folderIndex;
    int shift = 0;
    for(int ui = 0; ui < folder->structCount; ui++)
    {
        if(folder->folderStructs[(ui+shift)] == -1)
        {
            shift++;
        }
        int i = folder->folderStructs[(ui+shift)];
        RNAStructure *strct = structureManager->GetStructure(i);
	AddStructure(strct->GetPathname(), i);
    }
    copy_label(folder->folderName);
    char structLabel[MAX_BUFFER_SIZE];
    snprintf(structLabel, MAX_BUFFER_SIZE - 1, "%s%s", STRUCT_PANE_LABEL_PREFIX, folder->folderName);
    ConfigParser::nullTerminateString(structLabel);
    copy_label(structLabel);
    labelfont(FL_HELVETICA_BOLD);
    labeltype(_FL_SHADOW_LABEL);
    labelsize(1.25 * LOCAL_TEXT_SIZE);
    align(FL_ALIGN_CENTER | FL_ALIGN_TOP);

}

void FolderWindow::AddStructure(const char* filename, const int index) {
     StructureData *nextStructData = StructureData::AddStructureFromData(this, filename, index);
     RNAStructure *structure = nextStructData->structure;
     if(FolderWindow::m_storedStructDisplayData.size() > index) {
	  Delete(FolderWindow::m_storedStructDisplayData[index], StructureData);
	  FolderWindow::m_storedStructDisplayData[index] = nextStructData;
     }
     else {
          for(int sdi = FolderWindow::m_storedStructDisplayData.size(); sdi < index; sdi++) {
	       FolderWindow::m_storedStructDisplayData.push_back(NULL);
	  }
	  FolderWindow::m_storedStructDisplayData.push_back(nextStructData);
     }
}

void FolderWindow::ShowFileCallback(Fl_Widget* widget, void* userData) {
    RNAStructViz::GetInstance()->GetStructureManager()->DisplayFileContents((intptr_t)userData);
}

void FolderWindow::RemoveCallback(Fl_Widget* widget, void* userData)
{
    FolderWindow* fwindow = (FolderWindow*)(widget->parent()->parent()->parent()->parent());
    Fl_Pack* pack = fwindow->folderPack;
    for(int i = 0; i < pack->children(); ++i)
    {
	Fl_Group* tempGroup = (Fl_Group*)pack->child(i);
        Fl_Button* childButton = (Fl_Button*) tempGroup->child(1); // <--- here
	intptr_t userDataIdx = (intptr_t) childButton->user_data();
        if ((Fl_Widget *) childButton == widget)
        {
	    RNAStructViz* appInstance = RNAStructViz::GetInstance();
            const std::vector<DiagramWindow*>& diagrams = appInstance->GetDiagramWindows();
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == fwindow->m_folderIndex)
                {
                    diagrams[ui]->RemoveStructure(userDataIdx);
                    break;
                }
            }
            const std::vector<StatsWindow*>& stats = appInstance->GetStatsWindows();
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == fwindow->m_folderIndex)
                {
                    stats[ui]->RemoveStructure(userDataIdx);
                    break;
                }
            }
            Fl_Group* toRemove = (Fl_Group*) pack->child(i); // <--- here
            int toRemoveHeight = toRemove->h();
            int toRemoveYPos = toRemove->y();
            for(int j = i + 1; j < pack->children(); j++) {
                 Fl_Group* groupToMove = (Fl_Group*)pack->child(j);
                 groupToMove->resize(groupToMove->x(), groupToMove->y() - toRemoveHeight, 
                                     groupToMove->w(), groupToMove->h());
            }
            pack->remove(toRemove);
	    pack->resize(pack->x(), pack->y(), pack->w(), pack->h() - toRemoveHeight);
            pack->redraw();
	    int scrollToX = fwindow->folderScroll->xposition(), scrollToY = fwindow->folderScroll->yposition();
	    fwindow->folderScroll->scroll_to(0, 0);
            fwindow->folderScroll->scroll_to(scrollToX, scrollToY); 
            fwindow->folderScroll->scrollbar.align();
            fwindow->folderScroll->redraw();
            
            Folder *folder = appInstance->GetStructureManager()->GetFolderAt(fwindow->m_folderIndex);
	    if(folder != NULL && folder->structCount > 1) {
	         appInstance->GetStructureManager()->RemoveStructure(userDataIdx);
	    }
	    appInstance->GetStructureManager()->DecreaseStructCount(fwindow->m_folderIndex);
	    break;
        }
    }	    
    if(USE_SCHEDULED_DELETION) {
        RNAStructViz::ScheduledDeletion::ScheduleUpcomingDeletion();
    }

}

void FolderWindow::DiagramCallback(Fl_Widget* widget, void* userData)
{
    Fl_Group* folderGroup = (Fl_Group*)(widget->parent());
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    unsigned int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName, folderGroup->label() + strlen(STRUCT_PANE_LABEL_PREFIX)))
            break;
    }
    RNAStructViz::GetInstance()->AddDiagramWindow(index);
    
}

void FolderWindow::StatsCallback(Fl_Widget* widget, void* userData)
{
    Fl_Group* folderGroup = (Fl_Group*)(widget->parent());
    
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    unsigned int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName, folderGroup->label() + strlen(STRUCT_PANE_LABEL_PREFIX)))
            break;
    }
    RNAStructViz::GetInstance()->AddStatsWindow(index);
}

void FolderWindow::RethemeFolderWindow() {
     Fl_Color nextBGColor = GUI_WINDOW_BGCOLOR;
     Fl_Color nextLabelColor = GUI_BTEXT_COLOR;
     if(folderScroll != NULL) {
      folderScroll->color(nextBGColor);
      folderScroll->labelcolor(nextLabelColor);
     }
     if(fileOpsLabel != NULL) {
          fileOpsLabel->color(nextBGColor);
      fileOpsLabel->labelcolor(nextLabelColor);
          fileOpsLabel->redraw();
     }
     if(fileLabel != NULL) {
          fileLabel->color(nextBGColor);
      fileLabel->labelcolor(nextLabelColor);
          fileLabel->redraw();
     }
}
