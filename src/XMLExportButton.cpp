/* XMLExportButton.cpp : Implementation of the custom XML export buttons;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.12.03
 */

#include "XMLExportButton.h"
#include "RNAStructViz.h"
#include "ConfigParser.h"

XMLExportButton::XMLExportButton(unsigned int sindex) : 
	Fl_Button(0, 0, XMLEXPORT_BUTTON_SIZE, XMLEXPORT_BUTTON_SIZE, ""),
        structure(NULL) {
     RNAStructure *rnaStructInst = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(sindex);
     InitializeButtonData(rnaStructInst);
}

XMLExportButton::XMLExportButton(StructureData *stdata) : 
	Fl_Button(0, 0, XMLEXPORT_BUTTON_SIZE, XMLEXPORT_BUTTON_SIZE, ""), 
        structure(NULL) {
     if(stdata == NULL) {
          return;
     }
     InitializeButtonData(stdata->structure);
}

void XMLExportButton::InitializeButtonData(RNAStructure *rnaStruct) {
     
     structure = rnaStruct;
     user_data((void *) structure);
     callback(XMLExportButton::WriteStructureToXMLFileCallback);
     
     tooltip(XMLExportButton::ExportButtonTooltipText);
     labeltype(_FL_ICON_LABEL);
     box(FL_NO_BOX);
     align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     
     image(XMLExportButton::XMLEXPORT_BUTTON_IMAGE);
     deimage(XMLExportButton::XMLEXPORT_BUTTON_IMAGE);

}

std::string XMLExportButton::GetXMLExportPathFromUser(unsigned int options) {
     return "";
}

