/* AutoloadIndicatorButton.cpp : Implementation of the simple button wrapper class;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.28
 */

#include "ConfigOptions.h"
#include "AutoloadIndicatorButton.h"
#include "RNAStructViz.h"
#include "TerminalPrinting.h"

AutoloadIndicatorButton::AutoloadIndicatorButton(std::string diskPath, std::string baseNamePath, bool initState) : 
     Fl_Button(0, 0, AUTOLOAD_BUTTON_SIZE, AUTOLOAD_BUTTON_SIZE, ""), 
     filePathOnDisk(diskPath), filePathBaseName(baseNamePath), 
     isAutoloadedState(initState) {
     
     AutoloadIndicatorButton::InitStaticData();
     if(initState) {
	  ResetButton(1);
     }
     else {
	  ResetButton(0);
     }
     labeltype(_FL_ICON_LABEL);
     box(FL_NO_BOX);
     align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     callback(AutoloadIndicatorButton::ToggleAutoloadStatusCallback);

}

void AutoloadIndicatorButton::ResetButton(int stateValue) {
     if(stateValue) {
          image(LINK_SET_ICON);
	  deimage(LINK_SET_ICON);
          value(1);
          set();
	  isAutoloadedState = true;
	  tooltip(AutoloadIndicatorButton::linkSetTooltipText);
     }
     else {
          image(LINK_UNSET_ICON);
	  deimage(LINK_UNSET_ICON);
          value(0);
	  clear();
	  isAutoloadedState = false;
          tooltip(AutoloadIndicatorButton::linkUnsetTooltipText);
     }
     if(visible()) {
          redraw();
     }
}

void AutoloadIndicatorButton::ToggleAutoloadStatusCallback(Fl_Widget *wbtn, void *udata) {
     AutoloadIndicatorButton *aliBtn = (AutoloadIndicatorButton *) wbtn;
     bool opState;
     fprintf(stderr, "Copy/RemoveStructure: %s, %s\n", aliBtn->filePathBaseName.c_str(), aliBtn->filePathOnDisk.c_str());
     std::string autoLoadDir = std::string(USER_AUTOLOAD_PATH);
     if(aliBtn->filePathOnDisk.length() >= autoLoadDir.length() && aliBtn->filePathOnDisk.substr(0, autoLoadDir.length()) == autoLoadDir) {
	  TerminalText::PrintWarning("Unable to unlink non-symlinked file from initial autoload directory!");
	  return;
     }
     else if(!aliBtn->isAutoloadedState) {
	  opState = RNAStructViz::CopyStructureFileToAutoloadDirectory(aliBtn->filePathBaseName.c_str(), aliBtn->filePathOnDisk.c_str());
     }
     else {
	  RNAStructViz::RemoveStructureFileFromAutoloadDirectory(aliBtn->filePathBaseName.c_str());
	  opState = false;
     }
     aliBtn->ResetButton(opState ? 1 : 0);
}
