/* LoadFileSelectAllButton.cpp : Implementation of the select all files button;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.15
 */

#include "LoadFileSelectAllButton.h"
#include "ConfigOptions.h"

SelectAllButton::SelectAllButton(Fl_File_Chooser *flFileChooser, const char *buttonLabel) : 
     Fl_Button(0, 0, NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, buttonLabel), 
     flFileChooserRef(flFileChooser), selectedFileFilter("") {

     color(GUI_WINDOW_BGCOLOR);
     labelcolor(GUI_BTEXT_COLOR);
     box(FL_SHADOW_BOX);
     labeltype(FL_SHADOW_LABEL);
     user_data((void *) flFileChooser);
     callback(SelectAllButton::FileChooserSelectAllCallback);
     activate();

}

bool SelectAllButton::SelectAllFilesActivated() const {
     return user_data() == (void *) flFileChooserRef;
}

const char * SelectAllButton::GetSelectedFileFilter() const {
     return selectedFileFilter;
}

void SelectAllButton::FileChooserSelectAllCallback(Fl_Widget *wbtn, void *udata) {
     SelectAllButton *saBtn = (SelectAllButton *) wbtn;
     const char *curFileFilter = saBtn->flFileChooserRef->filter();
     saBtn->user_data((void *) curFileFilter);
     saBtn->selectedFileFilter = curFileFilter;
     saBtn->flFileChooserRef->hide();
}

int SelectAllButton::handle(int flEvent) {
     if(flEvent == FL_KEYDOWN) {
          if(Fl::event_ctrl() && (Fl::get_key((int) 'a') || Fl::get_key((int) 'A'))) {
	       FileChooserSelectAllCallback(this, NULL);
	  }
          return 1;
     }
     return Fl_Button::handle(flEvent);
}
