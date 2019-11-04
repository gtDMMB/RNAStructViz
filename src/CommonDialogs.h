/* CommonDialogs.h : Defines for common text buffers displayed in help and information 
 *                   dialogs in the application, as well as commands to display common 
 *                   (FLTK) help dialogs;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.30
 */

#ifndef __COMMON_DIALOGS_H__
#define __COMMON_DIALOGS_H__

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_PNG_Image.H>

#include "pixmaps/WelcomeButton.c"
#include "pixmaps/HelpIcon.c"
#include "pixmaps/InfoButton.c"
#include "pixmaps/TourIcon.c"

namespace CommonDialogs {

     static Fl_RGB_Image *welcomeIconImage = new Fl_RGB_Image( 
	       WelcomeButton.pixel_data, 
	       WelcomeButton.width, 
	       WelcomeButton.height, 
	       WelcomeButton.bytes_per_pixel
     );

     static Fl_RGB_Image *helpIconImage = new Fl_RGB_Image( 
	       HelpIcon.pixel_data, 
	       HelpIcon.width, 
	       HelpIcon.height, 
	       HelpIcon.bytes_per_pixel
     );

     static Fl_RGB_Image *infoIconImage = new Fl_RGB_Image( 
	       InfoButton.pixel_data, 
	       InfoButton.width, 
	       InfoButton.height, 
	       InfoButton.bytes_per_pixel
     );

     static Fl_RGB_Image *tourIconImage = new Fl_RGB_Image( 
	       TourIcon.pixel_data, 
	       TourIcon.width, 
	       TourIcon.height, 
	       TourIcon.bytes_per_pixel
     );

     std::string GetHelpInstructionsMessageString();
     void DisplayFirstRunInstructions();
     void DisplayHelpDialog();
     
     void DisplayTourDialog();
     
     std::string GetInfoAboutMessageString();
     void DisplayInfoAboutDialog();
     
     void DisplayKeyboardShortcutsDialog();

};

#endif
