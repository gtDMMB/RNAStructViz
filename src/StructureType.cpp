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
          baseSeqStart += " /// ->";
          dotPairSeqStart += " /// ->";
          tooltipText += std::string("\n") + baseSeqStart + std::string("\n") + dotPairSeqStart;
     }
     mainButtonTooltip = tooltipText;
     mainViewerDisplayBtn->copy_tooltip(mainButtonTooltip.c_str());
}

void StructureData::CreateGUIElementsDisplay(Fl_Pack *pack) {
               
     if(pack == NULL) {
          return;
     }	       
     DeleteGUIWidgets();	       
     guiPackingContainerRef = pack;

     pack->begin();
     int offsetY = STRUCTURE_WIDGET_HEIGHT * pack->children();
     guiPackingGroup = new Fl_Group(pack->x() + 2, offsetY, pack->w() + 8, STRUCTURE_WIDGET_HEIGHT, "");
     guiPackingGroup->begin();
               
     mainViewerDisplayBtn = new Fl_Button(pack->x() + 2, offsetY, pack->w() - 60, STRUCTURE_WIDGET_HEIGHT, "");
     mainViewerDisplayBtn->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     mainViewerDisplayBtn->callback(FolderWindow::ShowFileCallback);
     mainViewerDisplayBtn->user_data((void *) index);
     mainViewerDisplayBtn->labelsize(10);
     mainViewerDisplayBtn->labelcolor(GUI_BTEXT_COLOR);
     mainViewerDisplayBtn->labelfont(FL_HELVETICA_BOLD_ITALIC);
     SetLabelText();
     SetTooltipText();

     navCloseBtn = new Fl_Button(pack->x() + pack->w() - 8, offsetY + 5, STRUCTURE_NAVBTN_SIZE, 
		                 STRUCTURE_NAVBTN_SIZE, "");
     navCloseBtn->callback(FolderWindow::RemoveCallback);
     navCloseBtn->user_data((void *) index);
     navCloseBtn->label(StructureData::navCloseBtnLabel);
     navCloseBtn->tooltip("Remove sample from folder");
     navCloseBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     navCloseBtn->box(FL_PLASTIC_UP_BOX);

     char exactBaseName[MAX_BUFFER_SIZE];
     const char *exactPathName = structure->GetFilename(true);
     const char *lastSlashPos = strrchr(exactPathName, '/');
     if(lastSlashPos == NULL) {
          strcpy(exactBaseName, exactPathName);
     }
     else {
	  strcpy(exactBaseName, lastSlashPos + 1);
     }
     const char *autoLoadBaseName = (const char *) exactBaseName;
     bool isAutoloaded = ConfigParser::fileExists((std::string(USER_AUTOLOAD_PATH) + autoLoadBaseName).c_str());
     autoloadToggleBtn = new AutoloadIndicatorButton(structFileBaseDir + autoLoadBaseName, 
		                                     autoLoadBaseName, isAutoloaded);
     autoloadToggleBtn->position(pack->x() + pack->w() - 32, offsetY + 2);
     
     xmlExportBtn = new XMLExportButton(index);
     xmlExportBtn->position(pack->x() + pack->w() - 52, offsetY + 5);

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
     std::string fileNameStr = std::string(fileName);
     nextStructData->structFileBaseDir = fileNameStr.substr(0, dirPrefixLen + 1);
     nextStructData->structFileBaseName = fileNameStr.substr(dirPrefixLen + 1);
     nextStructData->index = index;
     nextStructData->CreateGUIElementsDisplay(fwinRef->folderPack);
     return nextStructData;
}
