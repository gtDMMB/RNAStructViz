/* StructureType.cpp : Separating the nested class definitions requires taking a few functions 
 *                     out of the inline definitions in the header file;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.28
 */

#include "StructureType.h"
#include "RNAStructViz.h"
#include "FolderWindow.h"
#include "RNAStructure.h"
#include "ConfigParser.h"

void StructureData::SetTooltipText() {
     std::string tooltipText = structFileBaseName;
     if(structure != NULL) {
	  std::string baseSeqStart = std::string(structure->GetSequenceString()).substr(0, TOOLTIP_PREVIEW_CHARS_LENGTH);
	  std::string dotPairSeqStart = std::string(structure->GetDotBracketSequenceString()).substr(0, TOOLTIP_PREVIEW_CHARS_LENGTH);
	  baseSeqStart = StringInsertSpacing(baseSeqStart, TOOLTIP_DATA_SPACING_INTERVAL, " ");
	  dotPairSeqStart = StringInsertSpacing(dotPairSeqStart, TOOLTIP_DATA_SPACING_INTERVAL, " ");
          baseSeqStart += " ///";
          dotPairSeqStart += " ///";
          tooltipText += std::string("\n") + baseSeqStart + std::string("\n") + dotPairSeqStart;
     }
     mainViewerDisplayBtn->copy_tooltip(tooltipText.c_str());
     mainButtonTooltip = tooltipText;
}

void StructureData::CreateGUIElementsDisplay(Fl_Pack *pack) {
               
     if(pack == NULL) {
          return;
     }	       
     DeleteGUIWidgets();	       
     guiPackingContainerRef = pack;

     pack->begin();
     int offsetY = STRUCTURE_WIDGET_HEIGHT * pack->children();
     guiPackingGroup = new Fl_Group(pack->x() + 10, offsetY, pack->w(), STRUCTURE_WIDGET_HEIGHT, "");
     guiPackingGroup->begin();
               
     mainViewerDisplayBtn = new Fl_Button(pack->x() + 10, offsetY, pack->w() - 50, STRUCTURE_WIDGET_HEIGHT, "");
     mainViewerDisplayBtn->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     mainViewerDisplayBtn->callback(FolderWindow::ShowFileCallback);
     mainViewerDisplayBtn->user_data((void *) index);
     mainViewerDisplayBtn->labelsize(10);
     mainViewerDisplayBtn->labelcolor(GUI_BTEXT_COLOR);
     mainViewerDisplayBtn->labelfont(FL_HELVETICA_BOLD_ITALIC);
     SetLabelText();
     SetTooltipText();

     navCloseBtn = new Fl_Button(pack->x() + pack->w() - 8, offsetY + 5, STRUCTURE_NAVBTN_SIZE, STRUCTURE_NAVBTN_SIZE);
     navCloseBtn->callback(FolderWindow::RemoveCallback);
     navCloseBtn->user_data((void *) index);
     navCloseBtn->label(StructureData::navCloseBtnLabel);
     navCloseBtn->tooltip("Remove sample from folder");
     navCloseBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     navCloseBtn->box(FL_PLASTIC_UP_BOX);

     bool isAutoloaded = ConfigParser::fileExists((std::string(USER_AUTOLOAD_PATH) + structFileBaseName).c_str());
     autoloadToggleBtn = new AutoloadIndicatorButton(structFileBaseDir + "/" + structFileBaseName, 
		                                     structFileBaseName, isAutoloaded);
     autoloadToggleBtn->position(pack->x() + pack->w() - 42, offsetY + 3);
     
     // TODO: Add other image status buttons

     guiPackingGroup->resizable(mainViewerDisplayBtn);
     guiPackingGroup->end();
     pack->end();
     if(parentMainFolderWin != NULL) {
          parentMainFolderWin->folderScroll->redraw();
     }

}

StructureData* StructureData::AddStructureFromData(FolderWindow *fwinRef, const char *fileName, const int index) {
     StructureData *nextStructData = new StructureData();
     nextStructData->parentMainFolderWin = fwinRef;
     nextStructData->structure = RNAStructViz::GetInstance()->GetStructureManager()->GetStructure(index);
     nextStructData->origFolderWinLabel = std::string(fileName);
     size_t dirPrefixLen = ((strrchr(fileName, '/') ? strrchr(fileName, '/') : fileName) - fileName);
     nextStructData->structFileBaseDir = std::string(fileName).substr(0, dirPrefixLen + 1);
     nextStructData->structFileBaseName = std::string(fileName + dirPrefixLen + 1);
     nextStructData->index = index;
     nextStructData->CreateGUIElementsDisplay(fwinRef->folderPack);
     return nextStructData;
}
