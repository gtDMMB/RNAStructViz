#include <unistd.h>

#include <iostream>

#include <FL/Fl_Button.H>

#include "FolderWindow.h"
#include "RNAStructure.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

#include "pixmaps/RNAStructVizLogo.c"
#include "pixmaps/StructureOperationIcon.c"

void FolderWindow::Construct(int w, int h, int folderIndex) {}

FolderWindow::FolderWindow(int x, int y, int wid, int hgt, 
		           const char *label, int folderIndex) : 
	      Fl_Group(x, y, wid, hgt, label), 
	      folderScroll(NULL), folderPack(NULL), 
	      fileOpsLabel(NULL), fileLabel(NULL)
{

    // label configuration happens inside FolderWindow::AddStructures ... 

    // icon configuration:
    structureIcon = new Fl_RGB_Image(StructureOperationIcon.pixel_data, 
		    StructureOperationIcon.width, StructureOperationIcon.height, 
		    StructureOperationIcon.bytes_per_pixel);
    structureIcon->color_average(FL_WHITE, 0.61);
    Fl_Box *structIconBox = new Fl_Box(x, y - 39, structureIcon->w(), structureIcon->h());
    structIconBox->image(structureIcon);
    
    int dividerTextHeight = 0, spacingHeight = NAVBUTTONS_SPACING;
    int fileOpsLabelHeight = 2 * NAVBUTTONS_BHEIGHT; 
    int fileOpsLabelWidth = 2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING + 4;
    int initYOffset = NAVBUTTONS_OFFSETY + RNAStructVizLogo.height + 5; 
    const char *fileOpsLabelText = "@reload Structure Operations.\nEach operation opens a new window.";
    fileOpsLabel = new Fl_Box(x - 2 + NAVBUTTONS_SPACING, initYOffset, 
		              fileOpsLabelWidth, fileOpsLabelHeight, 
			      fileOpsLabelText);
    fileOpsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);  
    fileOpsLabel->color(GUI_BGCOLOR);
    fileOpsLabel->labelcolor(GUI_BTEXT_COLOR);
    fileOpsLabel->labelfont(LOCAL_BFFONT);
    fileOpsLabel->labelsize(LOCAL_TEXT_SIZE);
    fileOpsLabel->box(FL_RSHADOW_BOX);

    int opButtonWidth = 128;
    int yOffset = initYOffset + fileOpsLabelHeight;
    Fl_Button* diagramButton = new Fl_Button(x + 20, yOffset + spacingHeight,
		                             opButtonWidth, 30,
		                             "View Diagram @>|");
    diagramButton->callback(DiagramCallback);
    diagramButton->labelcolor(GUI_BTEXT_COLOR);
    diagramButton->labelfont(FL_HELVETICA);

    Fl_Button* statsButton = new Fl_Button(x + 20 + opButtonWidth + 
		             spacingHeight, yOffset + spacingHeight, 
	                     opButtonWidth, 30,
		             "Statistics @>|");
    statsButton->callback(StatsCallback);
    statsButton->labelcolor(GUI_BTEXT_COLOR);
    statsButton->labelfont(FL_HELVETICA);

    const char *fileInstText = "@filenew Files.\nClick on the file buttons to view\nCT file contents in new window.";
    fileLabel = new Fl_Box(x - 2 + NAVBUTTONS_SPACING, y + yOffset + spacingHeight, 
			   fileOpsLabelWidth, fileOpsLabelHeight, fileInstText);
    fileLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    fileLabel->color(GUI_BGCOLOR);
    fileLabel->labelcolor(GUI_BTEXT_COLOR);
    fileLabel->labelfont(LOCAL_BFFONT);
    fileLabel->labelsize(LOCAL_TEXT_SIZE);
    fileLabel->box(FL_RSHADOW_BOX);

    folderScroll = new Fl_Scroll(x+10, y + yOffset + fileOpsLabelHeight + 
		                 dividerTextHeight + 3 * spacingHeight, 
			         280, 310 - 2 * fileOpsLabelHeight - dividerTextHeight - 
				 2 * spacingHeight - NAVBUTTONS_BHEIGHT);
    folderScroll->type(Fl_Scroll::VERTICAL_ALWAYS);

    folderPack = new Fl_Pack(x+10, y + yOffset + fileOpsLabelHeight + 
		             dividerTextHeight + 3 * spacingHeight, 260, 
			     290 - 2 * fileOpsLabelHeight - dividerTextHeight - 
			     2 * spacingHeight - NAVBUTTONS_BHEIGHT);
    folderPack->type(Fl_Pack::VERTICAL);
    folderPack->align(FL_ALIGN_TOP);

    folderScroll->color((Fl_Color) GUI_WINDOW_BGCOLOR);
    folderScroll->labelcolor((Fl_Color) GUI_BTEXT_COLOR);

    this->resizable(folderScroll);
    this->color(GUI_WINDOW_BGCOLOR);
    
    SetStructures(folderIndex);

}

FolderWindow::~FolderWindow() {
     delete structureIcon;
     structureIcon = NULL;
}

void FolderWindow::SetStructures(int folderIndex)
{

    StructureManager* structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    Folder* folder = structureManager->GetFolderAt(folderIndex);
    m_folderIndex = folderIndex;
    int shift = 0;
    for( int ui = 0; ui < folder->structCount; ui++)
    {
        if(folder->folderStructs[(ui+shift)] == -1)
        {
            shift++;
        }
        int i = folder->folderStructs[(ui+shift)];
        RNAStructure *strct = structureManager->GetStructure(i);
	AddStructure(strct->GetFilename(), i);
    }
    label(folder->folderName);
    char structLabel[MAX_BUFFER_SIZE];
    snprintf(structLabel, MAX_BUFFER_SIZE - 1, "%s%s", STRUCT_PANE_LABEL_PREFIX, folder->folderName);
    ConfigParser::nullTerminateString(structLabel);
    copy_label(structLabel);
    labelfont(LOCAL_BFFONT);
    labeltype(_FL_SHADOW_LABEL);
    labelsize(1.25 * LOCAL_TEXT_SIZE);
    align(FL_ALIGN_TOP);

}

void FolderWindow::AddStructure(const char* filename, const int index)
{
    
    Fl_Pack* pack = folderPack;
    pack->begin();
    
    int vertPosn = pack->children() * NAVBUTTONS_BHEIGHT; //+ pack->y() + 15;
    
    Fl_Group* group = new Fl_Group(pack->x(), vertPosn, pack->w(), NAVBUTTONS_BHEIGHT);
    group->begin();
    
    Fl_Button* label = new Fl_Button(pack->x() + 10, vertPosn, pack->w() - 40, 30, filename);
    label->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    label->callback(FolderWindow::ShowFileCallback);
    label->user_data((void*)index);
    label->labelcolor(GUI_BTEXT_COLOR);
    label->labelsize(10);
    label->labelfont(FL_HELVETICA_BOLD_ITALIC);
    char labelWithIcon[MAX_BUFFER_SIZE];

    std::string spaceBuffer = string("                                                    ");
    char fileNameNoExt[MAX_BUFFER_SIZE];
    size_t fileNameCopyChars = strlen(filename);
    const char *fileExtPos = strrchr(filename, '.');
    if(fileExtPos != NULL) {
         fileNameCopyChars = (size_t) (fileExtPos - filename);
    }
    fileNameCopyChars = MIN(fileNameCopyChars, MAX_BUFFER_SIZE - 1);
    strncpy(fileNameNoExt, filename, fileNameCopyChars);
    fileNameNoExt[fileNameCopyChars] = '\0';
    
    int curLabelLen = 0;
    char filePrefix[MAX_BUFFER_SIZE];
    size_t fileNameBytes = strlen(filename);
    snprintf(filePrefix, MAX_BUFFER_SIZE, "%-.22s%s", fileNameNoExt, 
	     fileNameBytes > MAX_FOLDER_LABEL_CHARS ? "..." : "");
    snprintf(labelWithIcon, MAX_BUFFER_SIZE - 1, "@filenew   %s%s", 
	     filePrefix, spaceBuffer.substr(0, 
             MAX(0, MAX_FOLDER_LABEL_CHARS - ((int ) strlen(filePrefix)))).c_str());
    label->copy_label(labelWithIcon);
    label->tooltip(filename); 
    
    Fl_Button* removeButton = new Fl_Button(pack->x() + pack->w() - 20, vertPosn + 5, 20, 20);
    removeButton->callback(FolderWindow::RemoveCallback);
    removeButton->user_data((void*)index);
    removeButton->label("@1+");
    removeButton->tooltip("Remove sample from folder");
    removeButton->labelcolor(GUI_BTEXT_COLOR);
    
    group->resizable(label);
    group->end();
    pack->end();
        
    folderScroll->redraw();
}

void FolderWindow::CloseFolderCallback(Fl_Widget* widget, void* userData)
{
    FolderWindow* fwindow = (FolderWindow*)(widget->parent());
    MainWindow::HideFolderByName(fwindow->label());
}

void FolderWindow::ShowFileCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->GetStructureManager()->DisplayFileContents((intptr_t)userData);
}

void FolderWindow::RemoveCallback(Fl_Widget* widget, void* userData)
{
    FolderWindow* fwindow = (FolderWindow*)(widget->parent()->parent()->parent()->parent());
    
    Fl_Pack* pack = fwindow->folderPack;
    for(int i = 0; i < pack->children(); ++i)
    {
        Fl_Group* tempGroup = (Fl_Group*)pack->child(i);
        Fl_Button* childButton = (Fl_Button*)tempGroup->child(1); // <--- here
        
        if (childButton == widget)
        {
            RNAStructViz* appInstance = RNAStructViz::GetInstance();
            const std::vector<DiagramWindow*>& diagrams = appInstance->GetDiagramWindows();
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == fwindow->m_folderIndex)
                {
                    diagrams[ui]->RemoveStructure((intptr_t)userData);
                    break;
                }
            }
            const std::vector<StatsWindow*>& stats = appInstance->GetStatsWindows();
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == fwindow->m_folderIndex)
                {
                    stats[ui]->RemoveStructure((intptr_t)userData);
                    break;
                }
            }
            Fl_Group* toRemove = (Fl_Group*)pack->child(i); // <--- here
	    int toRemoveHeight = toRemove->h();
	    int toRemoveYPos = toRemove->y();
	    for(int j = i + 1; j < pack->children(); j++) {
                 Fl_Group* groupToMove = (Fl_Group*)pack->child(j);
		 groupToMove->resize(groupToMove->x(), groupToMove->y() - toRemoveHeight, 
				     groupToMove->w(), groupToMove->h());
	    }
	    pack->remove(toRemove);
            pack->resize(pack->x(), pack->y(), pack->w(), pack->h() - toRemoveHeight);
	    fwindow->folderScroll->scroll_to(0, 0);
	    fwindow->folderScroll->scroll_to(fwindow->folderScroll->xposition(), 
			                     fwindow->folderScroll->yposition());
	    fwindow->folderScroll->scrollbar.align();
	    fwindow->folderScroll->redraw();
            Fl::delete_widget(toRemove);
            
            appInstance->GetStructureManager()->DecreaseStructCount( 
			                        fwindow->m_folderIndex);
            appInstance->GetStructureManager()->RemoveStructure((intptr_t)userData);
            
            break;
        }
    }
}

void FolderWindow::DiagramCallback(Fl_Widget* widget, void* userData)
{
    Fl_Group* folderGroup = (Fl_Group*)(widget->parent());
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;
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
    int index;
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
