/* LoadFileSelectAllButton.h : Subclasses the Fl_Button widget so that 
 *                             it can handle selecting all files when 
 *                             we add this custom widget to the 
 *                             Fl_File_Chooser using add_extra(...);
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.15
 */

#ifndef __LOAD_FILE_SELECT_ALL_BUTTON_H__
#define __LOAD_FILE_SELECT_ALL_BUTTON_H__

#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>

#ifndef NAVBUTTONS_BWIDTH
     #define NAVBUTTONS_BWIDTH    (135)
     #define NAVBUTTONS_BHEIGHT   (30)
#endif

class SelectAllButton : public Fl_Button {

     public:
          SelectAllButton(Fl_File_Chooser *flFileChooserRef, 
			  const char *btnLabel = "@filenew  Select All Files");

	  bool SelectAllFilesActivated() const;
	  const char * GetSelectedFileFilter() const;

     protected:
          Fl_File_Chooser *flFileChooserRef;
	  const char *selectedFileFilter;

	  static void FileChooserSelectAllCallback(Fl_Widget *saBtn, void *udata);

	  int handle(int flEvent);

};

#endif
