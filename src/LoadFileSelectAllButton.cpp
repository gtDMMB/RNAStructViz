/* LoadFileSelectAllButton.cpp : Implementation of the select all files button;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.15
 */

#include "LoadFileSelectAllButton.h"
#include "ConfigOptions.h"
#include "ConfigExterns.h"

SelectAllButton::SelectAllButton(Fl_File_Chooser *flFileChooser, int width, int height) : 
     Fl_Pack(0, 0, width, height), flFileChooserRef(flFileChooser), selectedFileFilter(NULL), 
     recursiveDirs(false), searchInHiddenDirs(false), followSymlinks(false), 
     avoidDuplicateStructs(true), selectAll(false), selectAllInHome(false) {

     color(Darker(GUI_WINDOW_BGCOLOR, 0.7f));
     labelcolor(GUI_BTEXT_COLOR);
     box(SelectAllButton::containerWidgetBorderStyle);
     labeltype(FL_ENGRAVED_LABEL);
     begin();
     saSubWidgetGroupContainer = new Fl_Group(x(), y(), w(), h(), "");
     saSubWidgetGroupContainer->begin();
     
     int offsetX = x() + SAWIDGET_PADDING, offsetY = y() + SAWIDGET_PADDING;
     saWidgetInstBox = new Fl_Box(offsetX, offsetY, 
		                  w() - 2 * SAWIDGET_PADDING, 
				  2 * NAVBUTTONS_BHEIGHT, 
				  SelectAllButton::selectAllWidgetInstsLabel);
     saWidgetInstBox->color(Darker(GUI_WINDOW_BGCOLOR, 0.75f));
     saWidgetInstBox->labelcolor(GUI_BTEXT_COLOR);
     saWidgetInstBox->labelsize(11);
     saWidgetInstBox->labelfont(FL_HELVETICA_ITALIC);
     saWidgetInstBox->box(SelectAllButton::instLabelStyle);
     offsetY += 2 * NAVBUTTONS_BHEIGHT + SAWIDGET_PADDING;
     offsetX += 2 * SAWIDGET_PADDING;

     selectAllBtn = new Fl_Button(offsetX, offsetY, NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
		                  SelectAllButton::selectAllBtnLabel);
     selectAllBtn->color(Lighter(GUI_WINDOW_BGCOLOR, 0.5f));
     selectAllBtn->labelcolor(GUI_BTEXT_COLOR);
     selectAllBtn->labelsize(12);
     selectAllBtn->labelfont(FL_HELVETICA_BOLD_ITALIC);
     selectAllBtn->callback(SelectAllButton::FileChooserSelectAllCallback);
     selectAllBtn->user_data((void *) this);
     selectAllBtn->box(SelectAllButton::buttonBoxStyle);
     selectAllBtn->labeltype(SelectAllButton::buttonLabelStyle);
     offsetX += NAVBUTTONS_BWIDTH + SAWIDGET_PADDING;

     selectAllHomeBtn = new Fl_Button(offsetX, offsetY, 1.75 * NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
		                      SelectAllButton::selectAllHomeBtnLabel);
     selectAllHomeBtn->color(Lighter(GUI_WINDOW_BGCOLOR, 0.5f));
     selectAllHomeBtn->labelcolor(GUI_BTEXT_COLOR);
     selectAllHomeBtn->labelsize(12);
     selectAllHomeBtn->labelfont(FL_HELVETICA_BOLD_ITALIC);
     selectAllHomeBtn->callback(SelectAllButton::FileChooserSelectAllInHomeCallback);
     selectAllHomeBtn->user_data((void *) this);
     selectAllHomeBtn->box(SelectAllButton::buttonBoxStyle);
     selectAllHomeBtn->labeltype(SelectAllButton::buttonLabelStyle);
     offsetY += NAVBUTTONS_BHEIGHT + SAWIDGET_PADDING;
     offsetX = x() + 4.5 * SAWIDGET_PADDING;

     Fl_Check_Button **saWidgetCheckButtons[] = {
          &cbRecDirSearch, 
	  &cbHiddenDirSearch,
	  &cbFollowSymlinks,
	  &cbAvoidStructDuplicates,
     };
     bool *saWidgetCBValues[] = {
	  &recursiveDirs,
	  &searchInHiddenDirs,
	  &followSymlinks,
	  &avoidDuplicateStructs,
     };
     const char *saWidgetCBLabels[] = {
          "Recursively search subdirctories",
	  "Search for files in hidden directories",
	  "Allow following of symlinked directories",
	  "Avoid loading duplicate structures for a sequence",
     };
     bool saWidgetCBEnabled[] = {
	  true,
	  true,
	  false,
	  true,
     };
     unsigned int numCheckBoxes = 4;
     for(int cbnum = 0; cbnum < numCheckBoxes; cbnum++) {
          *(saWidgetCheckButtons[cbnum]) = new Fl_Check_Button(offsetX, offsetY, w() - 6.5 * SAWIDGET_PADDING, 
			                                       0.75 * NAVBUTTONS_BHEIGHT, saWidgetCBLabels[cbnum]);
          //(*(saWidgetCheckButtons[cbnum]))->label(saWidgetCBLabels[cbnum]);
          saWidgetCBEnabled[cbnum] ? (*(saWidgetCheckButtons[cbnum]))->activate() : (*(saWidgetCheckButtons[cbnum]))->deactivate();
          *(saWidgetCBValues[cbnum]) ? (*(saWidgetCheckButtons[cbnum]))->set() : (*(saWidgetCheckButtons[cbnum]))->clear();
          (*(saWidgetCheckButtons[cbnum]))->color(Darker(GUI_WINDOW_BGCOLOR, 0.8f));
          (*(saWidgetCheckButtons[cbnum]))->labelcolor(GUI_BTEXT_COLOR);
          (*(saWidgetCheckButtons[cbnum]))->labelsize(11);
          (*(saWidgetCheckButtons[cbnum]))->labelfont(FL_HELVETICA);
          (*(saWidgetCheckButtons[cbnum]))->box(FL_RFLAT_BOX);
	  offsetY += 0.75 * NAVBUTTONS_BHEIGHT + SAWIDGET_PADDING / 2;
     }

     saSubWidgetGroupContainer->end();
     end();
}

SelectAllButton::~SelectAllButton() {
     saSubWidgetGroupContainer->hide();
     while(saSubWidgetGroupContainer->visible()) { Fl::wait(1.0); }
     Delete(cbRecDirSearch, Fl_Check_Button);
     Delete(cbHiddenDirSearch, Fl_Check_Button);
     Delete(cbFollowSymlinks, Fl_Check_Button);
     Delete(cbAvoidStructDuplicates, Fl_Check_Button);
     Delete(selectAllBtn, Fl_Button);
     Delete(selectAllHomeBtn, Fl_Button);
     Delete(saWidgetInstBox, Fl_Box);
     remove(saSubWidgetGroupContainer);
     Delete(saSubWidgetGroupContainer, Fl_Group);
     Free(selectedFileFilter);
}

bool SelectAllButton::SelectAllFilesActivated() const {
     return selectAll;
}

bool SelectAllButton::SelectAllFilesInHomeActivated() const {
     return selectAllInHome;
}

const char * SelectAllButton::GetSelectedFileFilter() const {
     return selectedFileFilter;
}

bool SelectAllButton::RecursiveDirectorySearch() const {
     return recursiveDirs;
}

bool SelectAllButton::HiddenDirectorySearch() const {
     return searchInHiddenDirs;
}

bool SelectAllButton::FollowSymlinks() const {
     return followSymlinks;
}

bool SelectAllButton::AvoidDuplicateStructures() const {
     return avoidDuplicateStructs;
}

void SelectAllButton::SetWidgetState() {
     selectedFileFilter = strdup(flFileChooserRef->filter());
     recursiveDirs = cbRecDirSearch->value() != 0;
     searchInHiddenDirs = cbHiddenDirSearch->value() != 0;
     followSymlinks = cbFollowSymlinks->value() != 0;
     avoidDuplicateStructs = cbAvoidStructDuplicates->value() != 0;
}

void SelectAllButton::FileChooserSelectAllCallback(Fl_Widget *wbtn, void *udata) {
     SelectAllButton *saBtn = (SelectAllButton *) wbtn->user_data();
     saBtn->selectAll = true;
     saBtn->SetWidgetState();
     saBtn->flFileChooserRef->hide();
}

void SelectAllButton::FileChooserSelectAllInHomeCallback(Fl_Widget *wbtn, void *udata) {
     SelectAllButton *saBtn = (SelectAllButton *) wbtn->user_data();
     saBtn->selectAllInHome = true;
     saBtn->SetWidgetState();
     saBtn->flFileChooserRef->hide();
}

#include "RNAStructViz.h"
#include "TerminalPrinting.h"

bool SelectAllButton::LoadAllFilesFromDirectory(std::string dirPathStr, const char *fileFilter, 
		                                bool recursive, bool hidden, 
		                                bool followSymlinks, bool avoidDuplicates) {
     
     bool returnStatus = true;
     const char *fileChooserFilter = fileFilter;
     try {
        fs::path dirPath(dirPathStr.c_str());
        fs::directory_iterator cwdDirIter(dirPath);
        std::vector<boost::filesystem::path> sortedFilePaths(cwdDirIter, boost::filesystem::directory_iterator());
        std::sort(sortedFilePaths.begin(), sortedFilePaths.end(), FileFormatSortCmp);
        for(int fi = 0; fi < sortedFilePaths.size(); fi++) {
             fs::path curFileEntryPath = fs::canonical(sortedFilePaths[fi], dirPath);
	     if(curFileEntryPath.filename().string() == "." || 
	        curFileEntryPath.filename().string() == "..") {
                  continue;
	     }
	     else if(curFileEntryPath.filename().string().length() > 0 && 
	      	     curFileEntryPath.filename().string().at(0) == '.' && !hidden) {
	          continue;
	     }
	     else if(fs::symlink_status(curFileEntryPath).type() == fs::symlink_file && !followSymlinks) {
	          continue;
	     }
	     else if(fs::is_directory(curFileEntryPath) && !recursive) {
	          continue;
	     }
	     else if(fs::is_directory(curFileEntryPath)) {
	          returnStatus = returnStatus && SelectAllButton::LoadAllFilesFromDirectory(dirPathStr + "/" + curFileEntryPath.filename().string(), 
			                                                                    fileChooserFilter, 
			                                                                    recursive, hidden, followSymlinks, 
                                                                                            avoidDuplicates);
	          continue;
	     }
	     else if(fl_filename_match(curFileEntryPath.filename().c_str(), fileChooserFilter)) {
                     char nextFilePath[MAX_BUFFER_SIZE];
		     char *nextWorkingDir = strdup(dirPathStr.c_str());
                     snprintf(nextFilePath, MAX_BUFFER_SIZE, "%s%s%s\0", nextWorkingDir,
                              nextWorkingDir[strlen(nextWorkingDir) - 1] == '/' ? "" : "/",
                              curFileEntryPath.filename().c_str());
                     TerminalText::PrintDebug("Adding structure with path \"%s\" [%s]\n", nextFilePath, nextWorkingDir);
                     RNAStructViz::GetInstance()->GetStructureManager()->AddFile(nextFilePath, avoidDuplicates, true);
		     Free(nextWorkingDir);
             }
        }
     } catch(fs::filesystem_error fse) {
          TerminalText::PrintError("Received filesystem_error (aborting from here): %s\n", fse.what());
	  return false;
     }
     return returnStatus;

}

bool SelectAllButton::LoadAllFilesFromDirectory(std::string dirPath, const SelectAllButton &saBtnRef) {
     return SelectAllButton::LoadAllFilesFromDirectory(dirPath, saBtnRef.GetSelectedFileFilter(),
		                                       saBtnRef.RecursiveDirectorySearch(), 
		                                       saBtnRef.HiddenDirectorySearch(), 
						       saBtnRef.FollowSymlinks(), saBtnRef.AvoidDuplicateStructures());
}
