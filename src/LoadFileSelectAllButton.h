/* LoadFileSelectAllButton.h : Subclasses the Fl_Button widget so that 
 *                             it can handle selecting all files when 
 *                             we add this custom widget to the 
 *                             Fl_File_Chooser using add_extra(...);
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.15
 */

#ifndef __LOAD_FILE_SELECT_ALL_BUTTON_H__
#define __LOAD_FILE_SELECT_ALL_BUTTON_H__

#include <string>
#include <vector>
using std::string;
using std::vector;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "Fl_File_Chooser.H"
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>

#ifndef NAVBUTTONS_BWIDTH
     #define NAVBUTTONS_BWIDTH    (135)
     #define NAVBUTTONS_BHEIGHT   (30)
#endif

#define DEFAULT_SAWIDGET_WIDTH    (460)
#define DEFAULT_SAWIDGET_HEIGHT   (235)
#define SAWIDGET_PADDING          (12)

class SelectAllButton : public Fl_Pack {

     public:
          SelectAllButton(Fl_File_Chooser_Custom *flFileChooserRef, 
			  int w = DEFAULT_SAWIDGET_WIDTH, 
			  int h = DEFAULT_SAWIDGET_HEIGHT);
          ~SelectAllButton();

          void redraw();

          void SetWidgetState();
	  bool SelectAllFilesActivated() const;
	  bool SelectAllFilesInHomeActivated() const;
	  const char * GetSelectedFileFilter() const;
	  bool RecursiveDirectorySearch() const;
	  bool HiddenDirectorySearch() const;
	  bool FollowSymlinks() const;
	  bool AvoidDuplicateStructures() const;

     protected:
          Fl_File_Chooser_Custom *flFileChooserRef;
	  char *selectedFileFilter;
          bool recursiveDirs, searchInHiddenDirs;
	  bool followSymlinks, avoidDuplicateStructs;
	  bool selectAll, selectAllInHome;

	  Fl_Group        *saSubWidgetGroupContainer;
	  Fl_Box          *saWidgetInstBox;
	  Fl_Button       *selectAllBtn, *selectAllHomeBtn;
	  Fl_Button       *rescanDirBtn;
          Fl_Check_Button *cbRecDirSearch, *cbHiddenDirSearch;
	  Fl_Check_Button *cbFollowSymlinks, *cbAvoidStructDuplicates;

	  static void FileChooserSelectAllCallback(Fl_Widget *saBtn, void *udata);
	  static void FileChooserSelectAllInHomeCallback(Fl_Widget *saBtn, void *udata);
          static void FileChooserRescanDirectoryCallback(Fl_Widget *rescanBtn, void *udata);

	  static inline Fl_Boxtype containerWidgetBorderStyle = FL_ROUNDED_FRAME;
	  static inline Fl_Boxtype instLabelStyle             = FL_RSHADOW_BOX;
	  static inline Fl_Boxtype buttonBoxStyle             = FL_PLASTIC_UP_BOX;
	  static inline Fl_Labeltype buttonLabelStyle         = FL_SHADOW_LABEL;

	  static inline const char *selectAllBtnLabel         = "@filenew  Select All Files";
	  static inline const char *selectAllHomeBtnLabel     = "@search   Search All In User Home";
          static inline const char *rescanDirBtnLabel         = "@reload";
	  static inline const char *selectAllWidgetInstsLabel = "Click on the buttons below to select all files whose\n"
		                                                "extensions match the file filter settings in the\n"
								"drop down menu selection at the top of this dialog.";
     
     /* Separate a slightly more generally useful procedure for loading files 
      * recursively from a starting point directory on disk: 
      */
     public:
	  static bool LoadAllFilesFromDirectory(std::string dirPath, const char *fileFilter, 
			                        bool recursive, bool hidden, 
			                        bool followSymlinks, bool avoidDuplicates);
	  static bool LoadAllFilesFromDirectory(std::string dirPath, const SelectAllButton &settingsWidgetRef);

};

#endif
