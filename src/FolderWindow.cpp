#include "FolderWindow.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include "MainWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include <unistd.h>
#include <iostream>
#include <FL/Fl_Button.H>

#include "pixmaps/StructureOperationIcon.c"

void FolderWindow::Construct(int w, int h, int folderIndex) {}

FolderWindow::FolderWindow(int x, int y, int wid, int hgt, const char *label, int folderIndex): 
	      Fl_Group(x, y, wid, hgt, label)
{

    // label configuration:  
    labelcolor(GUI_TEXT_COLOR);
    labelfont(LOCAL_BFFONT);
    labelsize(2 * LOCAL_TEXT_SIZE);    

    // icon configuration:
    structureIcon = new Fl_RGB_Image(StructureOperationIcon.pixel_data, 
		    StructureOperationIcon.width, StructureOperationIcon.height, 
		    StructureOperationIcon.bytes_per_pixel);
    Fl_Box *structIconBox = new Fl_Box(x - 14, y - 50, structureIcon->w(), structureIcon->h());
    structIconBox->image(structureIcon);
    
    const char *dividerText = "----------------------------------------------";
    int dividerTextHeight = 4, spacingHeight = 10;
    Fl_Box *textDivider = new Fl_Box(x + 20, y + 36, 120, dividerTextHeight, 
		                     dividerText); 
    textDivider->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    textDivider->labelcolor(GUI_TEXT_COLOR); 

    int fileOpsLabelHeight = 25, fileOpsLabelWidth = 120;
    const char *fileOpsLabelText = "@reload Structure Operations.\nEach operation opens a new window.";
    Fl_Box *fileOpsLabel = new Fl_Box(x + 20, y + 36 + dividerTextHeight + 
		                              spacingHeight, 
		                              fileOpsLabelWidth, fileOpsLabelHeight, 
				                      fileOpsLabelText);
    fileOpsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);  
    fileOpsLabel->labelcolor(GUI_TEXT_COLOR);

    int opButtonWidth = 110;
    Fl_Button* diagramButton = new Fl_Button(x+20,y+36 + dividerTextHeight + 
		                             fileOpsLabelHeight + 
				            	     2 * spacingHeight,
					                 opButtonWidth, 30,
		                             "@circle   Diagram @>|");
    diagramButton->callback(DiagramCallback);
    diagramButton->labelcolor(GUI_BTEXT_COLOR);

    Fl_Button* statsButton = new Fl_Button(x + 20 + opButtonWidth + 
		                           spacingHeight, y + 36 + 
					               dividerTextHeight + 
		                           fileOpsLabelHeight + 
					               2 * spacingHeight, 
					               opButtonWidth, 30,
		                           "@square   Statistics @>|");
    statsButton->callback(StatsCallback);
    statsButton->labelcolor(GUI_BTEXT_COLOR);

    Fl_Button* closeButton = new Fl_Button(x+wid-18,y - 29,20,20,"");
    closeButton->callback(CloseFolderCallback);
    closeButton->label("@1+");
    closeButton->labelcolor(GUI_TEXT_COLOR);

    int fileLabelHeight = 25, fileLabelWidth = 120; 
    const char *fileInstText = "@filenew Files.\nClick on the file buttons to view\nCT file contents in new window.";
    Fl_Box* fileLabel = new Fl_Box(x+20,y+55 + 26 + dividerTextHeight + 
		                   fileOpsLabelHeight + 2 * spacingHeight, 
				           fileLabelWidth, fileLabelHeight, 
		                   fileInstText);
    fileLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    fileLabel->labelcolor(GUI_TEXT_COLOR);

    folderScroll = new Fl_Scroll(x+10, y+70 + 26 + fileLabelHeight + 
		                         dividerTextHeight + fileOpsLabelHeight + 
			                	 2 * spacingHeight, 
			                	 280, 310 - fileLabelHeight - 
				                 dividerTextHeight - fileOpsLabelHeight - 
				                 2 * spacingHeight);
    folderScroll->type(Fl_Scroll::VERTICAL_ALWAYS);

    folderPack = new Fl_Pack(x+10, y+70 + 26 + fileLabelHeight + 
		                     dividerTextHeight + fileOpsLabelHeight + 
			                 2 * spacingHeight, 260, 
			                 310 - fileLabelHeight - dividerTextHeight - 
			                 fileOpsLabelHeight - 2 * spacingHeight);
    folderPack->type(Fl_Pack::VERTICAL);
    
    folderScroll->color(GUI_WINDOW_BGCOLOR);
    folderPack->color(GUI_WINDOW_BGCOLOR);

    this->resizable(folderScroll);
    this->color(GUI_WINDOW_BGCOLOR);
    //this->selection_color(GUI_BTEXT_COLOR);
    
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
    this->label(folder->folderName);
    char structLabel[MAX_BUFFER_SIZE];
    snprintf(structLabel, MAX_BUFFER_SIZE - 1, "Structure:\n%s", folder->folderName);
    ConfigParser::nullTerminateString(structLabel);
    copy_label(structLabel);
    this->labelsize(LOCAL_TEXT_SIZE);
    
}

void FolderWindow::AddStructure(const char* filename, const int index)
{
    
    Fl_Pack* pack = folderPack;
    pack->begin();
    
    int vertPosn = pack->children() * 30 + pack->y() + 120;
    
    Fl_Group* group = new Fl_Group(pack->x(), vertPosn, pack->w(), 30);
    group->begin();
    
    Fl_Button* label = new Fl_Button(pack->x() + 10, vertPosn, pack->w() - 40, 30, filename);
    label->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    label->callback(FolderWindow::ShowFileCallback);
    label->user_data((void*)index);
    label->labelcolor(GUI_BTEXT_COLOR);
    char labelWithIcon[MAX_BUFFER_SIZE];

    std::string spaceBuffer = string("                                                    ");
    int curLabelLen = 0;
    char filePrefix[MAX_BUFFER_SIZE];
    size_t numChars = snprintf(filePrefix, (int) (strchr(filename, '_') - filename + 1), 
		               "%s", filename);
    filePrefix[numChars] = '\0';
    snprintf(labelWithIcon, MAX_BUFFER_SIZE - 1, "@filenew   %s%s@|>", 
	     filePrefix, spaceBuffer.substr(0, 28 - strlen(filePrefix)).c_str());
    label->copy_label(labelWithIcon); 
    
    Fl_Button* removeButton = new Fl_Button(pack->x() + pack->w() - 20, vertPosn + 5, 20, 20);
    removeButton->callback(FolderWindow::RemoveCallback);
    removeButton->user_data((void*)index);
    removeButton->label("@1+");
    removeButton->labelcolor(GUI_TEXT_COLOR);
    
    group->resizable(label);
    group->end();
    pack->end();
        
    folderScroll->redraw();
}

void FolderWindow::CloseFolderCallback(Fl_Widget* widget, void* userData)
{
    FolderWindow* fwindow = (FolderWindow*)(widget->parent());
    MainWindow::HideFolderByName(fwindow->label());
    fprintf(stderr, "%s\n", fwindow->label());
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
            pack->remove(toRemove);
            fwindow->folderScroll->redraw();
            Fl::delete_widget(toRemove);
            
            appInstance->GetStructureManager()->RemoveStructure((intptr_t)userData);
            appInstance->GetStructureManager()->DecreaseStructCount(fwindow->m_folderIndex);
            
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
        if (!strcmp(folders[index]->folderName, 
		    folderGroup->label() + strlen("Structure:\n")))
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
        if (!strcmp(folders[index]->folderName, 
		    folderGroup->label() + strlen("Structure:\n")))
            break;
    }
    RNAStructViz::GetInstance()->AddStatsWindow(index);
}
