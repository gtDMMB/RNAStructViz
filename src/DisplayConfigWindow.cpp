/* DisplayConfigWindow.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Color_Chooser.H>

#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "MainWindow.h"

#include "pixmaps/ConfigPathsIcon.c"
#include "pixmaps/ConfigThemesIcon.c"
#include "pixmaps/PNGNewPathIcon.c"
#include "pixmaps/ConfigWindowIcon.xbm"

/* Setup initial definitions of the extern'ed variables here: */
#ifndef _SETUP_GLOBAL_EXTERNS_
#define _SETUP_GLOBAL_EXTERNS_
     
     char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE]; 
     char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
     char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
     char FLTK_THEME[MAX_BUFFER_SIZE]; 

     const char *ALL_FLTK_THEMES[FLTK_THEME_COUNT] = { 
     	     "local",
	     "base", 
	     "plastic", 
	     "gtk+", 
	     "gleam",
	     "custom"
     }; 

     const char *FLTK_THEME_HELP[FLTK_THEME_COUNT] = {
             "A sane choice for the local RNAStructViz application", 
	     "Default old Windows (95/98/Me/NT/2000), old GTK/KDE", 
	     "Aqua theme inspired controls like in Max OSX", 
	     "RedHat Bluecurve like theme", 
	     "Scheme inspired by the Clearlooks Glossy theme",
	     "Something else (user defined)"
     };

     char LIBFLTK_VERSION_STRING[MAX_BUFFER_SIZE];

     volatile Fl_Color GUI_WINDOW_BGCOLOR;
     volatile Fl_Color GUI_BGCOLOR;
     volatile Fl_Color GUI_BTEXT_COLOR;
     volatile Fl_Color GUI_TEXT_COLOR;

     bool GUI_USE_DEFAULT_FOLDER_NAMES;

#endif

bool DisplayConfigWindow::SetupInitialConfig() { 

     strncpy(CTFILE_SEARCH_DIRECTORY, DEFAULT_CTFILE_SEARCH_DIRECTORY, 
             MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(CTFILE_SEARCH_DIRECTORY); 
     strncpy(PNG_OUTPUT_DIRECTORY, DEFAULT_PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString(PNG_OUTPUT_DIRECTORY);
     strncpy(PNG_OUTPUT_PATH, DEFAULT_PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(PNG_OUTPUT_PATH);
     strncpy(FLTK_THEME, DEFAULT_FLTK_THEME, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString(FLTK_THEME); 
     snprintf(LIBFLTK_VERSION_STRING, MAX_BUFFER_SIZE - 1, 
              "%d.%d.%d (%g) - API %g", FL_MAJOR_VERSION, FL_MINOR_VERSION, 
	      FL_PATCH_VERSION, FL_VERSION, FL_API_VERSION);
     ConfigParser::nullTerminateString(LIBFLTK_VERSION_STRING); 

     GUI_WINDOW_BGCOLOR = LOCAL_WINDOW_BGCOLOR;
     GUI_BGCOLOR = LOCAL_BGCOLOR;
     GUI_BTEXT_COLOR = LOCAL_BUTTON_COLOR;
     GUI_TEXT_COLOR = LOCAL_TEXT_COLOR;
     GUI_USE_DEFAULT_FOLDER_NAMES = false;

     ConfigParser cfgParser(USER_CONFIG_PATH, true);
     cfgParser.storeVariables();      
     
     return true;

}

char * DisplayConfigWindow::TrimFilePathDisplay(const char *path, int maxlen = CFGWIN_MAX_FPATH_LENGTH) { 
     char *truncPath = (char *) malloc((maxlen + 1) * sizeof(char));
     if(strlen(path) < maxlen) {
          strncpy(truncPath, path, strlen(path) + 1);
     }
     else {
          truncPath[0] = truncPath[1] = truncPath[2] = '.';
	  strncpy(truncPath + 3, path + strlen(path) - maxlen, maxlen);
	  truncPath[maxlen + 2] = '\0';
     }
     return truncPath;
}

DisplayConfigWindow::DisplayConfigWindow() : 
     Fl_Cairo_Window(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT),
     finished(false), pngNewPathIcon(NULL), imageData(NULL) { 

     imageStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, CONFIG_WINDOW_WIDTH);
     imageData = new uchar[imageStride * CONFIG_WINDOW_WIDTH];
     memset(imageData, 0, imageStride * CONFIG_WINDOW_WIDTH);
     cairo_surface_t *crSurface = cairo_image_surface_create_for_data( 
			          imageData, CAIRO_FORMAT_ARGB32, 
                                  CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT, 
				  imageStride);
     crDraw = cairo_create(crSurface);   
     Fl::cairo_cc(crDraw, false);

     label(CONFIG_WINDOW_TITLE);
     color(GUI_WINDOW_BGCOLOR); 
     size_range(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT);
     set_modal();
     set_draw_cb(Draw); 
     callback(WindowCloseCallback);

     //#ifndef __APPLE__
     //fl_open_display();
     //Pixmap iconPixmap = XCreateBitmapFromData(fl_display, 
     // 		         DefaultRootWindow(fl_display),
     //                    ConfigWindowIcon_bits, ConfigWindowIcon_width, 
     //    	         ConfigWindowIcon_height);
     //this->icon((const void *) iconPixmap);
     //#endif

     ConstructWindow();

}

DisplayConfigWindow::~DisplayConfigWindow() {
     for(int w = 0; w < windowWidgets.size(); w++) {
          delete windowWidgets[w];
	  windowWidgets[w] = NULL;
     }
     delete fpathsIcon;
     delete themesIcon;
     if(pngNewPathIcon != NULL) {
          delete pngNewPathIcon;
     }
     //cairo_destroy(crDraw);
     //cairo_surface_destroy(crSurface);
     delete imageData;
     imageData = NULL;
}

void DisplayConfigWindow::ConstructWindow() {

     // place the widgets in the window:
     int workingYOffset = CFGWIN_WIDGET_OFFSETY + CFGWIN_SPACING;
    
     fpathsIcon = new Fl_RGB_Image(ConfigPathsIcon.pixel_data, 
		  ConfigPathsIcon.width, ConfigPathsIcon.height, 
		  ConfigPathsIcon.bytes_per_pixel);
     Fl_Box *fpathsIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
		             fpathsIcon->w(), fpathsIcon->h());
     fpathsIconBox->image(fpathsIcon);
     windowWidgets.push_back(fpathsIconBox);
     workingYOffset += fpathsIcon->h() + CFGWIN_SPACING;

     const char *fieldDesc[] = {
	"@->   Structure Search Directory:", 
	"@->   PNG Output Directory:", 
	"@->   PNG Output File Name:"
     };
     char (*fieldUpdateVars[])[MAX_BUFFER_SIZE] = {
         &CTFILE_SEARCH_DIRECTORY, 
	 &PNG_OUTPUT_DIRECTORY, 
	 &PNG_OUTPUT_PATH
     };
     bool needsDirChooser[] {
          true, 
          true,
	  false
     };
     for(int f = 0; f < NUMSETTINGS; f++) {
         int offsetX = CFGWIN_WIDGET_OFFSETX + 2 * CFGWIN_SPACING;
         Fl_Box *descBox = new Fl_Box(offsetX, workingYOffset, 
	 		   CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
	 		   fieldDesc[f]);
	 descBox->labelcolor(GUI_TEXT_COLOR);
	 descBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	 windowWidgets.push_back(descBox);
	 offsetX += CFGWIN_LABEL_WIDTH + CFGWIN_SPACING;
         Fl_Box *settingBox = new Fl_Box(offsetX, workingYOffset, 
			      (int) (1.5 * CFGWIN_LABEL_WIDTH), CFGWIN_LABEL_HEIGHT, 
                              *(fieldUpdateVars[f]));
	 settingBox->copy_label(TrimFilePathDisplay(*(fieldUpdateVars[f])));
	 settingBox->color(GUI_BTEXT_COLOR);
	 settingBox->labelcolor(GUI_TEXT_COLOR);
	 settingBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
         fpathsSettingBoxes[f] = settingBox;
	 fpathsUpdateRefs[f] = *(fieldUpdateVars[f]);
	 windowWidgets.push_back(settingBox);
	 offsetX += (int) (1.5 * CFGWIN_LABEL_WIDTH) + CFGWIN_SPACING;
	 if(needsDirChooser[f]) { 
              Fl_Button *chooseDirBtn = new Fl_Button(offsetX, workingYOffset, 
			CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
			"Select @|>");
	      chooseDirBtn->color(GUI_BGCOLOR);
	      chooseDirBtn->labelcolor(GUI_BTEXT_COLOR);
	      chooseDirBtn->user_data((void *) f);
	      chooseDirBtn->callback(SelectDirectoryCallback);
	      windowWidgets.push_back(chooseDirBtn);
	 }
	 else {
              Fl_Button *updatePathBtn = new Fl_Button(offsetX, workingYOffset, 
			CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
			"Update @filesaveas");
	      updatePathBtn->color(GUI_BGCOLOR);
	      updatePathBtn->labelcolor(GUI_BTEXT_COLOR);
	      updatePathBtn->user_data((void *) f);
	      updatePathBtn->callback(UpdatePNGPathCallback);
	      windowWidgets.push_back(updatePathBtn);
	 }
	 workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;
     }
     workingYOffset += CFGWIN_SPACING;
     
     themesIcon = new Fl_RGB_Image(ConfigThemesIcon.pixel_data, 
		  ConfigThemesIcon.width, ConfigThemesIcon.height, 
		  ConfigThemesIcon.bytes_per_pixel);
     Fl_Box *themesIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
		             themesIcon->w(), themesIcon->h());
     themesIconBox->image(themesIcon);
     windowWidgets.push_back(themesIconBox);
     workingYOffset += themesIcon->h() + CFGWIN_SPACING;

     // now handle user customizable selections of the FLTK schemes:
     int offsetX = CFGWIN_WIDGET_OFFSETX + 2 * CFGWIN_SPACING;
     Fl_Box *themeDescBox = new Fl_Box(offsetX, workingYOffset, 
	 	        	       CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
	 		               "@->   Global FLTK Theme: ");
     themeDescBox->labelcolor(GUI_TEXT_COLOR);
     themeDescBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     windowWidgets.push_back(themeDescBox);
     offsetX += CFGWIN_LABEL_WIDTH + CFGWIN_COLOR_WIDTH + 2 * CFGWIN_SPACING;
     Fl_Choice *fltkThemeChoiceMenu = new Fl_Choice(offsetX, workingYOffset, 
		                      CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT);
     int choiceActiveIdx = 0;
     for(int t = 0; t < FLTK_THEME_COUNT; t++) {
	  if(!strcmp(ALL_FLTK_THEMES[t], FLTK_THEME)) {
               choiceActiveIdx = t;
	  }
	  fltkThemeChoiceMenu->add(ALL_FLTK_THEMES[t], 0, 
			           ThemeChoiceMenuCallback, 
			           (void *) ((long int) t), 0);
	  Fl_Menu_Item *nextMenuEntry = (Fl_Menu_Item *) 
		                        fltkThemeChoiceMenu->find_item(ALL_FLTK_THEMES[t]);
	  nextMenuEntry->labelcolor(GUI_BTEXT_COLOR);
     }
     fltkThemeChoiceMenu->value(choiceActiveIdx);
     fltkThemeChoiceMenu->labelcolor(GUI_BTEXT_COLOR);
     windowWidgets.push_back(fltkThemeChoiceMenu);
     workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;

     const char *colorFieldDesc[] = {
          "@->   GUI Window Background Color:", 
	  "@->   GUI Widget Color:", 
	  "@->   GUI Button Text Color:", 
	  "@->   GUI Primary (Dark) Text Color:"
     };
     volatile Fl_Color *colorVarRefs[] = {
          &GUI_WINDOW_BGCOLOR, 
	  &GUI_BGCOLOR, 
	  &GUI_BTEXT_COLOR, 
	  &GUI_TEXT_COLOR
     };
     for(int c = 0; c < GUICOLORS; c++) { 
	 offsetX = CFGWIN_WIDGET_OFFSETX + 2 * CFGWIN_SPACING;
         Fl_Box *descBox = new Fl_Box(offsetX, workingYOffset, 
			   CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
			   colorFieldDesc[c]);
	 descBox->labelcolor(GUI_TEXT_COLOR);
	 descBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	 windowWidgets.push_back(descBox);
	 offsetX += CFGWIN_LABEL_WIDTH + CFGWIN_SPACING;
	 Fl_Box *colorBox = new Fl_Box(offsetX, workingYOffset, 
			    CFGWIN_COLOR_WIDTH, CFGWIN_LABEL_HEIGHT, "@square");
	 colorBox->labelcolor(*(colorVarRefs[c]));
	 colorDisplayBoxes[c] = colorBox;
	 colorChangeRefs[c] = colorVarRefs[c];
	 windowWidgets.push_back(colorBox);
	 offsetX += CFGWIN_COLOR_WIDTH + CFGWIN_SPACING;
	 Fl_Button *selectColorButton = new Fl_Button(offsetX, workingYOffset, 
			                CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
					"Choose Color @|>");
	 selectColorButton->user_data((void *) c);
	 selectColorButton->labelcolor(GUI_BTEXT_COLOR); 
	 selectColorButton->color(GUI_BGCOLOR);
	 selectColorButton->callback(ChangeColorCallback); 
	 windowWidgets.push_back(selectColorButton);
         workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;
     }

     workingYOffset += CFGWIN_SPACING;

     // draw bounding box for the two action buttons on the 
     // bottom right of the window:
     int boundingBoxWidth = 3 * CFGWIN_BUTTON_WIDTH + 4 * CFGWIN_SPACING;
     offsetX = CONFIG_WINDOW_WIDTH - boundingBoxWidth - CFGWIN_SPACING / 2;
     int bdBoxHeight = (int) 1.5 * (CONFIG_WINDOW_HEIGHT - workingYOffset);
     int bdBoxYOffset = CONFIG_WINDOW_HEIGHT - bdBoxHeight - CFGWIN_SPACING;
     Fl_Box *btnBoundingBox = new Fl_Box(offsetX, bdBoxYOffset, 
		              boundingBoxWidth, bdBoxHeight);
     btnBoundingBox->box(FL_RSHADOW_BOX);
     btnBoundingBox->color(GUI_BGCOLOR);
     windowWidgets.push_back(btnBoundingBox);

     offsetX = CONFIG_WINDOW_WIDTH - 
	       (CFGWIN_BUTTON_WIDTH + 2 * CFGWIN_SPACING);
     Fl_Button *writeConfigBtn = new Fl_Button(offsetX, workingYOffset, 
		                 CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
				 "@filenew   Save Settings");
     writeConfigBtn->color(FL_LIGHT2);
     writeConfigBtn->labelcolor(FL_DARK2);
     writeConfigBtn->callback(WriteConfigFileCallback);
     windowWidgets.push_back(writeConfigBtn);
     offsetX -= CFGWIN_BUTTON_WIDTH + CFGWIN_SPACING;

     Fl_Button *restoreDefaultsBtn = new Fl_Button(offsetX, workingYOffset, 
		                     CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
				     "@redo   Restore Previous");
     restoreDefaultsBtn->color(FL_LIGHT2);
     restoreDefaultsBtn->labelcolor(FL_DARK2);
     restoreDefaultsBtn->callback(RestoreDefaultsCallback);
     windowWidgets.push_back(restoreDefaultsBtn);
     offsetX -= CFGWIN_BUTTON_WIDTH + CFGWIN_SPACING;

     Fl_Button *cancelBtn = new Fl_Button(offsetX, workingYOffset, 
		            CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
			    "@1+   Cancel");
     cancelBtn->color(FL_LIGHT2);
     cancelBtn->labelcolor(FL_DARK2);
     cancelBtn->callback(WindowCloseCallback);
     windowWidgets.push_back(cancelBtn);

} 

bool DisplayConfigWindow::isDone() const {
     return finished;
}

void DisplayConfigWindow::drawWidgets() {
     for(int w = 0; w < windowWidgets.size(); w++) {
          windowWidgets[w]->redraw();
     }
}

void DisplayConfigWindow::Draw(Fl_Cairo_Window *crWin, cairo_t *cr) {

    DisplayConfigWindow *thisWin = (DisplayConfigWindow *) crWin;
    cairo_set_source_rgb(thisWin->crDraw, 
		         GetRed(GUI_WINDOW_BGCOLOR) / 255.0f,
		         GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
			 GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
    cairo_scale(thisWin->crDraw, thisWin->w(), thisWin->h());
    cairo_fill(thisWin->crDraw);
    thisWin->drawWidgets();

}

void DisplayConfigWindow::SelectDirectoryCallback(Fl_Widget *btn, void *udata) { 

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int settingIdx = (long int) ((int *) btn->user_data());
     char *nextDirectory = fl_dir_chooser("Select New Default Directory ...", 
		                          parentWin->fpathsUpdateRefs[settingIdx], 0);
     if(nextDirectory == NULL) {
          return;
     }
     strncpy(parentWin->fpathsUpdateRefs[settingIdx], nextDirectory, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(parentWin->fpathsUpdateRefs[settingIdx]);
     parentWin->fpathsSettingBoxes[settingIdx]->copy_label(nextDirectory);
     parentWin->fpathsSettingBoxes[settingIdx]->redraw();

}

void DisplayConfigWindow::UpdatePNGPathCallback(Fl_Widget *btn, void *udata) {

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int settingIdx = (long int) ((int *) btn->user_data());
     
     fl_message_title("Choose New Format String for Saved PNG Output Paths ...");
     Fl_Box *msgIconBox = (Fl_Box *) fl_message_icon();
     if(parentWin->pngNewPathIcon == NULL) { 
          parentWin->pngNewPathIcon = new Fl_RGB_Image(PNGNewPathIcon.pixel_data, 
	     	                      PNGNewPathIcon.width, PNGNewPathIcon.height, 
		                      PNGNewPathIcon.bytes_per_pixel);
     }
     if(msgIconBox != NULL) { 
          msgIconBox->image(parentWin->pngNewPathIcon);
          msgIconBox->label("");
	  msgIconBox->type(FL_NO_BOX);
	  msgIconBox->color(FL_LIGHT2);
          msgIconBox->box(FL_NO_BOX);
	  msgIconBox->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | 
			    FL_ALIGN_CENTER);
	  msgIconBox->redraw();
     }

     const char *inputDialogMsg = "Choose the format of the PNG image paths saved by the diagram window. \nNote that C-style strftime-like modifiers such as %%F, %%H%%M%%S, \nare supported in the format description entered below. \nSee \"man strftime\", or strftime(3), in your terminal for a complete \ndescription of supported timestamp modifiers.";
     const char *nextPNGPath = fl_input(inputDialogMsg, parentWin->fpathsUpdateRefs[settingIdx]);
     if(nextPNGPath == NULL) {
          return;
     }
     strncpy(parentWin->fpathsUpdateRefs[settingIdx], nextPNGPath, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(parentWin->fpathsUpdateRefs[settingIdx]);
     parentWin->fpathsSettingBoxes[settingIdx]->copy_label(nextPNGPath);
     parentWin->fpathsSettingBoxes[settingIdx]->redraw();

}

void DisplayConfigWindow::ThemeChoiceMenuCallback(Fl_Widget *widget, void *udata) {
     long int themeIndex = (long int) udata;
     strncpy(FLTK_THEME, ALL_FLTK_THEMES[themeIndex], MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString(FLTK_THEME);
     fl_alert("The FLTK theme \"%s\" corresponds to: %s.\nNote that this new widget scheme will not take effect until after RNAStructViz is restarted.", FLTK_THEME, 
	      FLTK_THEME_HELP[themeIndex]);
}

void DisplayConfigWindow::WriteConfigFileCallback(Fl_Widget *btn, void *udata) {
     btn->parent()->hide();
     ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
     MainWindow::RethemeMainWindow();
}

void DisplayConfigWindow::ChangeColorCallback(Fl_Widget *btn, void *udata) {

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int colorIdx = (long int) ((int *) btn->user_data());
     Fl_Color currentColor = *(parentWin->colorChangeRefs[colorIdx]);
     double currentR = GetRed(currentColor) / 255.0;
     double currentG = GetGreen(currentColor) / 255.0;
     double currentB = GetBlue(currentColor) / 255.0;
     
     int colorSelectOK = fl_color_chooser("@search   Select New Theme Color ...", 
		         currentR, currentG, currentB, 0);
     if(colorSelectOK) { 
          int nextR = (int) (currentR * 255.0);
          int nextG = (int) (currentG * 255.0);
          int nextB = (int) (currentB * 255.0);
          *(parentWin->colorChangeRefs[colorIdx]) = RGBColor(nextR, nextG, nextB);
          parentWin->colorDisplayBoxes[colorIdx]->labelcolor(RGBColor(nextR, nextG, nextB));
          parentWin->colorDisplayBoxes[colorIdx]->redraw();
     }

}

void DisplayConfigWindow::RestoreDefaultsCallback(Fl_Widget *btn, void *udata) {
     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     DisplayConfigWindow::SetupInitialConfig();
     for(int fp = 0; fp < NUMSETTINGS; fp++) {
          parentWin->fpathsSettingBoxes[fp]->copy_label(parentWin->fpathsUpdateRefs[fp]);
          parentWin->fpathsSettingBoxes[fp]->redraw();
     }
     for(int c = 0; c < GUICOLORS; c++) {
          parentWin->colorDisplayBoxes[c]->labelcolor(*(parentWin->colorChangeRefs[c]));
	  parentWin->colorDisplayBoxes[c]->redraw();
     }
     MainWindow::RethemeMainWindow();
}

void DisplayConfigWindow::WindowCloseCallback(Fl_Widget *win, void *udata) {
     DisplayConfigWindow *thisWin; 
     if(win->as_window()) {
          thisWin = (DisplayConfigWindow *) win;
     }
     else {
          thisWin = (DisplayConfigWindow *) win->parent();
     }
     thisWin->hide();
     thisWin->finished = true;
     MainWindow::ms_instance->m_mainWindow->redraw();
     //MainWindow::RethemeMainWindow();
}
