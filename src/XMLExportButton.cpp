/* XMLExportButton.cpp : Implementation of the custom XML export buttons;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.12.03
 */

#include "XMLExportButton.h"
#include "RNAStructViz.h"
#include "ConfigParser.h"

XMLExportButton::XMLExportButton(unsigned int sindex) : 
	Fl_Button(0, 0, XMLEXPORT_BUTTON_SIZE, XMLEXPORT_BUTTON_SIZE, "") {
     XMLExportButton::InitStaticData();
     RNAStructure *rnaStructInst = RNAStructViz::GetInstance()->GetStructureManager()->GetStructure(sindex);
     InitializeButtonData(rnaStructInst);
}

XMLExportButton::XMLExportButton(StructureData *stdata) : 
	Fl_Button(0, 0, XMLEXPORT_BUTTON_SIZE, XMLEXPORT_BUTTON_SIZE, "") {
     if(stdata == NULL) {
          return;
     }
     InitializeButtonData(stdata->structure);
}

void XMLExportButton::InitializeButtonData(RNAStructure *rnaStruct) {
     
     user_data((void *) rnaStruct);
     callback(XMLExportButton::WriteStructureToXMLFileCallback);
     
     tooltip(XMLExportButton::exportButtonTooltipText);
     labeltype(_FL_ICON_LABEL);
     box(FL_NO_BOX);
     align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     
     image(XMLExportButton::XMLEXPORT_BUTTON_IMAGE);
     deimage(XMLExportButton::XMLEXPORT_BUTTON_IMAGE);

}

std::string XMLExportButton::GetXMLExportPathFromUser(unsigned int options) {
    const char *chooserMsg = "Choose a file name for your output structure XML file ...";
    const char *fileExtMask = "*.xml";
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    char defaultFilePath[MAX_BUFFER_SIZE];
    defaultFilePath[0] = '\0';
    strftime(defaultFilePath + strlen(defaultFilePath), MAX_BUFFER_SIZE - 1, (char *) PNG_OUTPUT_PATH,
             tmCurrentTime);
    Fl_Native_File_Chooser fileChooser;
    fileChooser.directory((char *) PNG_OUTPUT_DIRECTORY);
    fileChooser.title(chooserMsg);
    fileChooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fileChooser.options(Fl_Native_File_Chooser::NEW_FOLDER |
                Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    fileChooser.preset_file(defaultFilePath);
    switch(fileChooser.show()) {
      case -1: // ERROR
             fl_alert("Error selecting file path to save XML file: \"%s\".\n"
                      "If you are receiving a permissions error trying to save "
                      "the image into the directory you have chosen, try again by "
                      "saving the PNG image into a path in your user home directory.",
                      fileChooser.errmsg());
         return string("");
      case 1: // CANCEL
         return string("");
      default:
         const char *outFileFullPath = fileChooser.filename();
         const char *dirMarkerPos = strrchr(outFileFullPath, '/');
         if(dirMarkerPos != NULL || (dirMarkerPos = strrchr(fileChooser.directory(), '/')) != NULL) {
              unsigned int charsToCopy = (unsigned int) (dirMarkerPos - outFileFullPath + 1);
              strncpy((char *) PNG_OUTPUT_DIRECTORY, outFileFullPath, charsToCopy);
              PNG_OUTPUT_DIRECTORY[charsToCopy] = '\0';
              ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
         }
         return std::string(outFileFullPath);
    }
}

void XMLExportButton::WriteStructureToXMLFileCallback(Fl_Widget *w, void *udata) {
     XMLExportButton *xmlBtn = (XMLExportButton *) w;
     RNAStructure *rnaStructInst = (RNAStructure *) xmlBtn->user_data();
     fl_alert("The export structure to XML feature is disabled in this testing release.");
}

