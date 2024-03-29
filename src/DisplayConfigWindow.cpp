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
#include <FL/fl_show_colormap.H>

#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "MainWindow.h"
#include "ThemesConfig.h"
#include "CairoDrawingUtils.h"
#include "RNAStructViz.h"

#include "pixmaps/ConfigPathsIcon.c"
#include "pixmaps/ConfigThemesIcon.c"
#include "pixmaps/PNGNewPathIcon.c"
#include "pixmaps/ConfigDiagramWindow.c"
#include "pixmaps/ConfigCheckBoxParams.c"

/* Setup initial definitions of the extern'ed variables here: */
#ifndef _SETUP_GLOBAL_EXTERNS_
#define _SETUP_GLOBAL_EXTERNS_
     
     volatile char CTFILE_SEARCH_DIRECTORY[MAX_BUFFER_SIZE]; 
     volatile char PNG_OUTPUT_DIRECTORY[MAX_BUFFER_SIZE];
     volatile char PNG_OUTPUT_PATH[MAX_BUFFER_SIZE];
     volatile char PNG_RADIAL_LAYOUT_OUTPUT_PATH[MAX_BUFFER_SIZE];
     volatile char FLTK_THEME[MAX_BUFFER_SIZE]; 
     volatile char LOCAL_THEME_NAME[MAX_BUFFER_SIZE];
     volatile ColorTheme_t *LOCAL_COLOR_THEME = NULL;

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
     volatile Fl_Color GUI_CTFILEVIEW_COLOR;
     volatile Fl_Color STRUCTURE_DIAGRAM_COLORS[3][7];

     bool GUI_USE_DEFAULT_FOLDER_NAMES;
     bool GUI_KEEP_STICKY_FOLDER_NAMES;
     int  DEBUGGING_ON;
     bool DISPLAY_FIRSTRUN_MESSAGE;

#endif

bool DisplayConfigWindow::SetupInitialConfig() { 

     getcwd((char *) CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString((char *) CTFILE_SEARCH_DIRECTORY);
     if(CTFILE_SEARCH_DIRECTORY == NULL) {
          strncpy((char *) CTFILE_SEARCH_DIRECTORY, DEFAULT_CTFILE_SEARCH_DIRECTORY, 
                  MAX_BUFFER_SIZE - 1);
          ConfigParser::nullTerminateString((char *) CTFILE_SEARCH_DIRECTORY);
     } 
     strncpy((char *) PNG_OUTPUT_DIRECTORY, DEFAULT_PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString((char *) PNG_OUTPUT_DIRECTORY);
     strncpy((char *) PNG_OUTPUT_PATH, DEFAULT_PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString((char *) PNG_OUTPUT_PATH);
     strncpy((char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH, DEFAULT_RLAYOUT_PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString((char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH);
     strncpy((char *) FLTK_THEME, DEFAULT_FLTK_THEME, MAX_BUFFER_SIZE - 1); 
     ConfigParser::nullTerminateString((char *) FLTK_THEME); 
     snprintf(LIBFLTK_VERSION_STRING, MAX_BUFFER_SIZE - 1, 
              "%d.%d.%d (%g) - API %g", FL_MAJOR_VERSION, FL_MINOR_VERSION, 
              FL_PATCH_VERSION, FL_VERSION, FL_API_VERSION);
     ConfigParser::nullTerminateString(LIBFLTK_VERSION_STRING);

     if(!UpdateApplicationThemeByName("Structviz default")) {
          GUI_WINDOW_BGCOLOR = LOCAL_WINDOW_BGCOLOR;
          GUI_BGCOLOR = LOCAL_BGCOLOR;
          GUI_BTEXT_COLOR = LOCAL_BUTTON_COLOR;
          GUI_TEXT_COLOR = LOCAL_TEXT_COLOR;
          GUI_CTFILEVIEW_COLOR = Darker(LOCAL_WINDOW_BGCOLOR, 0.385f);
	  strcpy((char *) LOCAL_THEME_NAME, "Structviz default");
	  LOCAL_COLOR_THEME = GetColorThemeRefByName((const char *) LOCAL_THEME_NAME);
     }

     int structDiagramColorCounts[3] = { 1, 3, 7 };
     Fl_Color diagramInitColors[3][7] = {
         {
             FL_BLACK
         },
         {
             FL_BLACK, 
             FL_MAGENTA,
             FL_GREEN,
         },
         {
             FL_BLACK, 
             FL_LOCAL_DEEP_BLUE,
             FL_LOCAL_BRIGHT_YELLOW,
             FL_MAGENTA,
             FL_GREEN,
             FL_RED,
             FL_CYAN
         },
     };
     for(int s = 0; s < 3; s++) {
          for(int c = 0; c < structDiagramColorCounts[s]; c++) {
               STRUCTURE_DIAGRAM_COLORS[s][c] = diagramInitColors[s][c];
      }
     }

     GUI_USE_DEFAULT_FOLDER_NAMES = false;
     GUI_KEEP_STICKY_FOLDER_NAMES = false;
     DEBUGGING_ON = 0;
     DISPLAY_FIRSTRUN_MESSAGE = true;

     ConfigParser cfgParser(USER_CONFIG_PATH, !DEBUGGING_ON);
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
          truncPath[maxlen] = '\0';
     }
     return truncPath;
}

DisplayConfigWindow::DisplayConfigWindow() : 
     Fl_Double_Window(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT),
     finished(false), pngNewPathIcon(NULL), imageData(NULL) { 

     //Fl::visual(FL_RGB | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE | FL_INDEX);
     Fl::visual(FL_RGB);

     //imageStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, CONFIG_WINDOW_WIDTH);
     //imageData = new uchar[imageStride * CONFIG_WINDOW_WIDTH];
     //memset(imageData, 0, imageStride * CONFIG_WINDOW_WIDTH);
     //crSurface = cairo_image_surface_create_for_data( 
     //                 imageData, CAIRO_FORMAT_ARGB32, 
     //                             CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT, 
     //             imageStride
     //            );
     //crDraw = cairo_create(crSurface);   
     //Fl::cairo_cc(crDraw, false);

     label(CONFIG_WINDOW_TITLE);
     color(GUI_WINDOW_BGCOLOR); 
     size_range(CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT);
     set_modal();
     callback(WindowCloseCallback);
     //set_draw_cb(Draw); 
     //Fl::cairo_make_current(this);

     ConstructWindow();

}

DisplayConfigWindow::~DisplayConfigWindow() {
     for(int w = 0; w < windowWidgets.size(); w++) {
          Delete(windowWidgets[w], Fl_Widget);
     }
     Delete(fpathsIcon, Fl_RGB_Image);
     Delete(themesIcon, Fl_RGB_Image);
     Delete(dwinSettingsIcon, Fl_RGB_Image);
     Delete(cfgCheckboxesIcon, Fl_RGB_Image);
     Delete(pngNewPathIcon, Fl_RGB_Image);
     //cairo_surface_destroy(crSurface);
     //cairo_destroy(crDraw);
     Delete(imageData, uchar);
}

void DisplayConfigWindow::ConstructWindow() {

     xclass("RMAStructViz");
	
     // place the widgets in the window:
     int workingYOffset = CFGWIN_WIDGET_OFFSETY;
    
     fpathsIcon = new Fl_RGB_Image(ConfigPathsIcon.pixel_data, 
          ConfigPathsIcon.width, ConfigPathsIcon.height, 
          ConfigPathsIcon.bytes_per_pixel);
     Fl_Box *fpathsIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
                                        fpathsIcon->w(), fpathsIcon->h());
     fpathsIcon->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
     fpathsIconBox->image(fpathsIcon);
     windowWidgets.push_back(fpathsIconBox);
     workingYOffset += fpathsIcon->h() + CFGWIN_SPACING / 2;

     const char *fieldDesc[] = {
         "@->   File Search Directory:", 
         "@->   PNG Output Directory:", 
         "@->   PNG Output File Name:", 
         "@->   Radial Layout PNG Name:"
     };
     volatile char (*fieldUpdateVars[])[MAX_BUFFER_SIZE] = {
         &CTFILE_SEARCH_DIRECTORY, 
         &PNG_OUTPUT_DIRECTORY, 
         &PNG_OUTPUT_PATH, 
         &PNG_RADIAL_LAYOUT_OUTPUT_PATH,
     };
     bool needsDirChooser[] {
          true, 
          true,
          false,
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
                  (int) (2 * CFGWIN_LABEL_WIDTH), CFGWIN_LABEL_HEIGHT, 
                              (char *) *(fieldUpdateVars[f]));
         settingBox->copy_label(TrimFilePathDisplay((char *) *(fieldUpdateVars[f])));
         settingBox->color(GUI_BTEXT_COLOR);
         settingBox->labelcolor(GUI_TEXT_COLOR);
         settingBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
         fpathsSettingBoxes[f] = settingBox;
         fpathsUpdateRefs[f] = (char *) *(fieldUpdateVars[f]);
         windowWidgets.push_back(settingBox);
         offsetX += (int) (2 * CFGWIN_LABEL_WIDTH) + CFGWIN_SPACING / 2;
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
     
     dwinSettingsIcon = new Fl_RGB_Image(ConfigDiagramWindow.pixel_data, 
                                 ConfigDiagramWindow.width, ConfigDiagramWindow.height, 
                                 ConfigDiagramWindow.bytes_per_pixel);
     Fl_Box *dwinSettingsIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
                                              dwinSettingsIcon->w(), dwinSettingsIcon->h());
     dwinSettingsIcon->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.75), 0.65);
     dwinSettingsIconBox->image(dwinSettingsIcon);
     windowWidgets.push_back(dwinSettingsIconBox);
     workingYOffset += dwinSettingsIcon->h() + CFGWIN_SPACING / 2;

     int structColorCounts[3] = { 1, 3, 7 };
     const char *arcColorRowDesc[3] = {
          "@->   Arc Colors for One Structure:", 
          "@->   Arc Colors for Two Structures:", 
          "@->   Arc Colors for Three Structures:" 
     };
     volatile Fl_Color *structColorVarRefs[3][7] = {
         { 
             &STRUCTURE_DIAGRAM_COLORS[0][0]
         },
         {
             &STRUCTURE_DIAGRAM_COLORS[1][0], 
             &STRUCTURE_DIAGRAM_COLORS[1][1], 
             &STRUCTURE_DIAGRAM_COLORS[1][2]
         },
         {
             &STRUCTURE_DIAGRAM_COLORS[2][0], 
             &STRUCTURE_DIAGRAM_COLORS[2][1], 
             &STRUCTURE_DIAGRAM_COLORS[2][2], 
             &STRUCTURE_DIAGRAM_COLORS[2][3], 
             &STRUCTURE_DIAGRAM_COLORS[2][4], 
             &STRUCTURE_DIAGRAM_COLORS[2][5], 
             &STRUCTURE_DIAGRAM_COLORS[2][6]
         }
     };
     int offsetX = CFGWIN_WIDGET_OFFSETX + 2 * CFGWIN_SPACING;
     int offsetXInit = offsetX;
     for(int s = 0; s < 3; s++) {
          Fl_Box *structColorsDescBox = new Fl_Box(offsetX, workingYOffset, 
                                       1.5 * CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
                                                   arcColorRowDesc[s]);
          structColorsDescBox->labelcolor(GUI_TEXT_COLOR);
          structColorsDescBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
          windowWidgets.push_back(structColorsDescBox);
          offsetX += 1.5 * CFGWIN_LABEL_WIDTH + CFGWIN_COLOR_WIDTH + 2 * CFGWIN_SPACING;
          for(int c = 0; c < structColorCounts[s]; c++) { 
           Fl_Button *colorButton = new Fl_Button(offsetX, workingYOffset, 
                                          CFGWIN_COLOR_WIDTH, CFGWIN_LABEL_HEIGHT, "[+]");
           Fl_Color initDefaultButtonColor = *(structColorVarRefs[s][c]);
           colorButton->color(initDefaultButtonColor);
           Fl_Color labelTextColor = initDefaultButtonColor == FL_BLACK ? 
                                 Lighter(initDefaultButtonColor, 0.5) :
                     Darker(initDefaultButtonColor, 0.5);
           colorButton->labelcolor(labelTextColor);
           int indexUserData = (s & 0x0000ffff) | ((c << 16) & 0xffff0000);
           colorButton->user_data((void *) indexUserData);
           colorButton->callback(ChangeDiagramWindowArcColorCallback);
           dwinArcColorChangeRefs[s][c] = structColorVarRefs[s][c];
           windowWidgets.push_back(colorButton);
           offsetX += CFGWIN_COLOR_WIDTH + CFGWIN_SPACING;
      }
      offsetX = offsetXInit;
          workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;
     }

     // now handle user customizable selections of the FLTK schemes:
     themesIcon = new Fl_RGB_Image(ConfigThemesIcon.pixel_data, 
          ConfigThemesIcon.width, ConfigThemesIcon.height, 
          ConfigThemesIcon.bytes_per_pixel);
     Fl_Box *themesIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
                                        themesIcon->w(), themesIcon->h());
     themesIcon->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
     themesIconBox->image(themesIcon);
     windowWidgets.push_back(themesIconBox);
     workingYOffset += themesIcon->h() + CFGWIN_SPACING / 2;

     Fl_Box *themeDescBox = new Fl_Box(offsetX, workingYOffset, 
                            CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
                            "@->   Global Preset Themes: ");
     themeDescBox->labelcolor(GUI_TEXT_COLOR);
     themeDescBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     windowWidgets.push_back(themeDescBox);
     offsetX += CFGWIN_LABEL_WIDTH + CFGWIN_COLOR_WIDTH + 2 * CFGWIN_SPACING;
     Fl_Choice *fltkThemeChoiceMenu = new Fl_Choice(offsetX, workingYOffset, 
                              CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT);
     int choiceActiveIdx = 0;
     for(int t = 0; t < FLTK_THEME_COUNT; t++) {
           if(!strcmp(ALL_FLTK_THEMES[t], (char *) FLTK_THEME)) {
                choiceActiveIdx = t;
           }
           fltkThemeChoiceMenu->add(ALL_FLTK_THEMES[t], 0, 
                                    FLTKThemeChoiceMenuCallback, 
                                    (void *) ((long int) t), 0);
           Fl_Menu_Item *nextMenuEntry = (Fl_Menu_Item *) fltkThemeChoiceMenu->find_item(ALL_FLTK_THEMES[t]);
           nextMenuEntry->labelcolor(GUI_BTEXT_COLOR);
     }
     fltkThemeChoiceMenu->value(choiceActiveIdx);
     fltkThemeChoiceMenu->labelcolor(GUI_BTEXT_COLOR);
     windowWidgets.push_back(fltkThemeChoiceMenu);
     
     offsetX += CFGWIN_BUTTON_WIDTH + 2 * CFGWIN_SPACING;
     Fl_Choice *presetThemesChooser = new Fl_Choice(offsetX, workingYOffset, 
                              CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT);
     int numThemes = sizeof(PRESET_COLOR_THEMES) / sizeof(PRESET_COLOR_THEMES[0]);
     int selectedThemeIdx = 0;
     for(int t = numThemes - 1; t >= 0; t--) {
          presetThemesChooser->add(PRESET_COLOR_THEMES[t].themeName, 0, 
                       PresetThemeChooserMenuCallback, 
                       (void *) ((long int) t), 0);
          Fl_Menu_Item *nextMenuEntry = (Fl_Menu_Item *) 
          presetThemesChooser->find_item(PRESET_COLOR_THEMES[t].themeName);
          nextMenuEntry->labelcolor(GUI_BTEXT_COLOR);
	  if(!strcmp((char *) LOCAL_THEME_NAME, PRESET_COLOR_THEMES[t].themeName)) {
	       selectedThemeIdx = numThemes - 1 - t;
	  }
     }
     presetThemesChooser->labelcolor(GUI_BTEXT_COLOR);
     presetThemesChooser->value(selectedThemeIdx);
     windowWidgets.push_back(presetThemesChooser);
     workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;

     const char *colorFieldDesc[] = {
          "@->   Window Background:", 
          "@->   Widget Color:", 
          "@->   Button Text Color:", 
          "@->   Primary Text Color:"
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
         offsetX += CFGWIN_LABEL_WIDTH + 2 * CFGWIN_SPACING;
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
         offsetX += CFGWIN_BUTTON_WIDTH + CFGWIN_SPACING;
         Fl_Button *selectColormapBtn = new Fl_Button(offsetX, workingYOffset, 
                            CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
                    "Select Colormap @|>");
         selectColormapBtn->user_data((void *) c);
         selectColormapBtn->labelcolor(GUI_BTEXT_COLOR);
         selectColormapBtn->color(GUI_BGCOLOR);
         selectColormapBtn->callback(SelectFromColormapCallback);
         windowWidgets.push_back(selectColormapBtn);
     
         workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;
     
     }

     cfgCheckboxesIcon = new Fl_RGB_Image(ConfigCheckBoxParamsIcon.pixel_data, 
                                  ConfigCheckBoxParamsIcon.width, 
                                  ConfigCheckBoxParamsIcon.height, 
                                  ConfigCheckBoxParamsIcon.bytes_per_pixel);
     Fl_Box *cfgCheckboxesIconBox = new Fl_Box(CFGWIN_WIDGET_OFFSETX, workingYOffset, 
                                               cfgCheckboxesIcon->w(), cfgCheckboxesIcon->h());
     cfgCheckboxesIcon->color_average(Lighter(*(LOCAL_COLOR_THEME->bwImageAvgColor), 0.7), 0.65);
     cfgCheckboxesIconBox->image(cfgCheckboxesIcon);
     windowWidgets.push_back(cfgCheckboxesIconBox);
     workingYOffset += cfgCheckboxesIcon->h() + CFGWIN_SPACING / 2;

     const char *checkBoxLabels[] = {
          "Display first-run message when RNAStructViz starts",
          "Use default folder names in load files dialog (not recommended)", 
          "Remember structure folder names (use sticky folder names)",
     };
     bool includeRHSClearButton[] = { false, false, true };
     volatile bool *checkBoxParamSettings[] = {
          &DISPLAY_FIRSTRUN_MESSAGE,
          &GUI_USE_DEFAULT_FOLDER_NAMES,
          &GUI_KEEP_STICKY_FOLDER_NAMES,
     };
     for(int cbi = 0; cbi < GetArrayLength(checkBoxLabels); cbi++) {
          const int bulletPointBoxWidth = 16;
          int workingOffsetX = CFGWIN_WIDGET_OFFSETX + 2 * CFGWIN_SPACING;
          Fl_Box *bpBox = new Fl_Box(workingOffsetX, workingYOffset, 
                         bulletPointBoxWidth, CFGWIN_LABEL_HEIGHT, 
                         "@->  ");
          bpBox->labelcolor(GUI_TEXT_COLOR);
          bpBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
          windowWidgets.push_back(bpBox);
          workingOffsetX += bulletPointBoxWidth + CFGWIN_SPACING;
          Fl_Check_Button *cbParam = new Fl_Check_Button(workingOffsetX, workingYOffset, 
                                                         2.5 * CFGWIN_LABEL_WIDTH, CFGWIN_LABEL_HEIGHT, 
                                                         checkBoxLabels[cbi]);
          cbParam->type(FL_TOGGLE_BUTTON);
          cbParam->labelcolor(GUI_BTEXT_COLOR);
          cbParam->selection_color(GUI_TEXT_COLOR);
          cbParam->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
          cbParam->value((int) *(checkBoxParamSettings[cbi]));
          cbParam->user_data((void *) checkBoxParamSettings[cbi]);
          cbParam->callback(ToggleCheckBoxParamCallback);
          windowWidgets.push_back(cbParam);
          if(includeRHSClearButton[cbi]) {
               workingOffsetX += 2.5 * CFGWIN_LABEL_WIDTH + 2 * CFGWIN_SPACING;
               Fl_Button *clearStickyNamesBtn = new Fl_Button(workingOffsetX, workingYOffset, 
                                                      1.5 * CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
                                      "@undo  Clear Saved Folder Names");
               clearStickyNamesBtn->labelcolor(GUI_BTEXT_COLOR);
               clearStickyNamesBtn->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
               clearStickyNamesBtn->callback(ClearAllStickyFolderNamesCallback);
               windowWidgets.push_back(clearStickyNamesBtn);
          }
          workingYOffset += CFGWIN_LABEL_HEIGHT + CFGWIN_SPACING;
     }

     // draw bounding box for the two action buttons on the 
     // bottom right of the window:
     int boundingBoxWidth = 3 * CFGWIN_BUTTON_WIDTH + 7 * CFGWIN_SPACING;
     offsetX = CONFIG_WINDOW_WIDTH - boundingBoxWidth;
     int bdBoxHeight = (int) (2.0 * CFGWIN_LABEL_HEIGHT);
     int bdBoxYOffset = workingYOffset + CFGWIN_SPACING;
     Fl_Box *btnBoundingBox = new Fl_Box(offsetX, bdBoxYOffset, 
                      boundingBoxWidth, bdBoxHeight);
     btnBoundingBox->box(FL_RSHADOW_BOX);
     btnBoundingBox->color(GUI_BGCOLOR);
     windowWidgets.push_back(btnBoundingBox);

     offsetX = CONFIG_WINDOW_WIDTH - 
           (CFGWIN_BUTTON_WIDTH + CFGWIN_SPACING);
     int offsetY = bdBoxYOffset + bdBoxHeight / 4;
     Fl_Button *writeConfigBtn = new Fl_Button(offsetX, offsetY, 
                         CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
                 "@filenew   Save Settings");
     writeConfigBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
     writeConfigBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     writeConfigBtn->callback(WriteConfigFileCallback);
     windowWidgets.push_back(writeConfigBtn);
     offsetX -= CFGWIN_BUTTON_WIDTH + 2 * CFGWIN_SPACING;

     Fl_Button *restoreDefaultsBtn = new Fl_Button(offsetX, offsetY, 
                             CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
                     "@redo   Restore Previous");
     restoreDefaultsBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
     restoreDefaultsBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     restoreDefaultsBtn->callback(RestoreDefaultsCallback);
     windowWidgets.push_back(restoreDefaultsBtn);
     offsetX -= CFGWIN_BUTTON_WIDTH + 2 * CFGWIN_SPACING;

     Fl_Button *cancelBtn = new Fl_Button(offsetX, offsetY, 
                    CFGWIN_BUTTON_WIDTH, CFGWIN_LABEL_HEIGHT, 
                "@1+   Cancel");
     cancelBtn->color(Lighter(GUI_BGCOLOR, 0.5f));
     cancelBtn->labelcolor(Darker(GUI_BTEXT_COLOR, 0.5f));
     cancelBtn->callback(WindowCloseCallback);
     windowWidgets.push_back(cancelBtn);

     redraw();
     //Fl::cairo_make_current(this);
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
    cairo_set_source_rgb(cr, 
                         GetRed(GUI_WINDOW_BGCOLOR) / 255.0f,
                         GetGreen(GUI_WINDOW_BGCOLOR) / 255.0f, 
                         GetBlue(GUI_WINDOW_BGCOLOR) / 255.0f);
    cairo_scale(cr, thisWin->w(), thisWin->h());
    cairo_fill(cr);
    thisWin->drawWidgets();

}

bool DisplayConfigWindow::UpdateApplicationThemeByName(const char *themeName) {
     int themeIndex = -1;
     for(int t = 0; t < GetArrayLength(PRESET_COLOR_THEMES); t++) {
          if(!strcmp(themeName, PRESET_COLOR_THEMES[t].themeName)) {
               themeIndex = t;
               break;
          }
     }
     return UpdateApplicationThemeByIndex(themeIndex);
}

bool DisplayConfigWindow::UpdateApplicationThemeByIndex(int themeIndex) {
     if(themeIndex < 0 || themeIndex >= GetArrayLength(PRESET_COLOR_THEMES)) {
          return false;
     }
     ColorTheme_t nextTheme = PRESET_COLOR_THEMES[themeIndex];
     if(!nextTheme.isValid) {
          return false;
     }
     GUI_WINDOW_BGCOLOR = nextTheme.windowBGColor;
     GUI_BGCOLOR = nextTheme.widgetBGColor;
     GUI_BTEXT_COLOR = nextTheme.widgetTextColor;
     GUI_TEXT_COLOR = nextTheme.printTextColor;
     GUI_CTFILEVIEW_COLOR = nextTheme.ctFileDisplayColor;
     strcpy((char *) LOCAL_THEME_NAME, nextTheme.themeName);
     LOCAL_COLOR_THEME = (volatile ColorTheme_t *) &(PRESET_COLOR_THEMES[themeIndex]);
     return true;
}

void DisplayConfigWindow::SelectDirectoryCallback(Fl_Widget *btn, void *udata) { 

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int settingIdx = (long int) ((int *) btn->user_data());
     char *nextDirectory = fl_dir_chooser("Select New Default Directory ...", 
                                          parentWin->fpathsUpdateRefs[settingIdx], 0);
     if(nextDirectory == NULL) {
          return;
     }
     strncpy(parentWin->fpathsUpdateRefs[settingIdx], nextDirectory, MAX_BUFFER_SIZE);
     parentWin->fpathsUpdateRefs[settingIdx][MAX_BUFFER_SIZE - 1] = '\0';
     parentWin->fpathsSettingBoxes[settingIdx]->copy_label(nextDirectory);
     Free(nextDirectory);
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
           msgIconBox->color(Lighter(GUI_BGCOLOR, 0.5f));
           msgIconBox->box(FL_NO_BOX);
           msgIconBox->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
           msgIconBox->redraw();
     }

     const char *inputDialogMsg = "Choose the format of the PNG image paths saved by the diagram window. \n" 
	                          "Note that C-style strftime-like modifiers such as %%F, %%H%%M%%S, \nare "
				  "supported in the format description entered below. \nSee \"man strftime\", "
				  "or strftime(3), in your terminal for a complete \ndescription of supported timestamp modifiers.";
     const char *nextPNGPath = fl_input(inputDialogMsg, parentWin->fpathsUpdateRefs[settingIdx]);
     if(nextPNGPath == NULL) {
          return;
     }
     strncpy(parentWin->fpathsUpdateRefs[settingIdx], nextPNGPath, MAX_BUFFER_SIZE);
     parentWin->fpathsUpdateRefs[settingIdx][MAX_BUFFER_SIZE - 1] = '\0';
     parentWin->fpathsSettingBoxes[settingIdx]->copy_label(nextPNGPath);
     parentWin->fpathsSettingBoxes[settingIdx]->redraw();

}

void DisplayConfigWindow::FLTKThemeChoiceMenuCallback(Fl_Widget *widget, void *ud) {
     long int themeIndex = (long int) ud;
     strncpy((char *) FLTK_THEME, ALL_FLTK_THEMES[themeIndex], MAX_BUFFER_SIZE - 1);
     ConfigParser::nullTerminateString((char *) FLTK_THEME);
     fl_alert("The FLTK theme \"%s\" corresponds to: %s.\nNote that this new widget scheme will not take effect until after RNAStructViz is restarted.", FLTK_THEME, 
          FLTK_THEME_HELP[themeIndex]);
}

void DisplayConfigWindow::PresetThemeChooserMenuCallback(Fl_Widget *btn, void *ud) {
     
     DisplayConfigWindow *thisWin = (DisplayConfigWindow *) btn->parent();
     long int themeIndex = (long int) ud;
     UpdateApplicationThemeByIndex(themeIndex);
     for(int c = 0; c < GUICOLORS; c++) {
          thisWin->colorDisplayBoxes[c]->color((Fl_Color) 
                             *(thisWin->colorChangeRefs[c]));
      thisWin->colorDisplayBoxes[c]->redraw();
     }
     fl_alert("The selected theme will be applied the next time you restart the application.");
}

void DisplayConfigWindow::WriteConfigFileCallback(Fl_Widget *btn, void *udata) {
     ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
     DisplayConfigWindow::WindowCloseCallback(btn, NULL);
}

void DisplayConfigWindow::ChangeColorCallback(Fl_Widget *btn, void *udata) {

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int colorIdx = (long int) ((int *) btn->user_data());
     Fl_Color currentColor = *(parentWin->colorChangeRefs[colorIdx]);
     double currentR = GetRed(currentColor) / 255.0;
     double currentG = GetGreen(currentColor) / 255.0;
     double currentB = GetBlue(currentColor) / 255.0;
     
     int colorSelectOK = fl_color_chooser("@search   Select New Theme Color ...", 
                 currentR, currentG, currentB, 2);
     if(colorSelectOK) { 
          int nextR = (int) (currentR * 255.0);
          int nextG = (int) (currentG * 255.0);
          int nextB = (int) (currentB * 255.0); 
          Fl_Color nextBGColor = RGBColor(nextR, nextG, nextB);
          *(parentWin->colorChangeRefs[colorIdx]) = nextBGColor;
          parentWin->colorDisplayBoxes[colorIdx]->labelcolor(nextBGColor);
          parentWin->colorDisplayBoxes[colorIdx]->redraw();
          if(!colorIdx) {
               GUI_CTFILEVIEW_COLOR = Darker(nextBGColor, 0.5f);
          }
     }

}

void DisplayConfigWindow::SelectFromColormapCallback(Fl_Widget *btn, void *udata) {

     DisplayConfigWindow *parentWin = (DisplayConfigWindow *) btn->parent();
     long int colorIdx = (long int) ((int *) btn->user_data());
     Fl_Color currentColor = *(parentWin->colorChangeRefs[colorIdx]);
     Fl_Color nextColor = fl_show_colormap(currentColor);
     *(parentWin->colorChangeRefs[colorIdx]) = Fl::get_color(nextColor);
     parentWin->colorDisplayBoxes[colorIdx]->labelcolor(Fl::get_color(nextColor));
     parentWin->colorDisplayBoxes[colorIdx]->redraw();

}

void DisplayConfigWindow::ChangeDiagramWindowArcColorCallback(Fl_Widget *btn, void *udata) {
     DisplayConfigWindow *displayWin = (DisplayConfigWindow *) btn->parent();
     Fl_Button *buttonRef = (Fl_Button *) btn;
     long int userIndexData = (long int) udata, structIdx, colorIdx;
     structIdx = userIndexData & 0x0000ffff;
     colorIdx = (userIndexData >> 16) & 0x0000ffff;
     Fl_Color currentColor = *(displayWin->dwinArcColorChangeRefs[structIdx][colorIdx]);
     Fl_Color nextColor = fl_show_colormap(currentColor);
     *(displayWin->dwinArcColorChangeRefs[structIdx][colorIdx]) = Fl::get_color(nextColor);
     buttonRef->color(Fl::get_color(nextColor));
     buttonRef->redraw();
}

void DisplayConfigWindow::ToggleCheckBoxParamCallback(Fl_Widget *cbWidget, void *udata) {
     Fl_Check_Button *cb = (Fl_Check_Button *) cbWidget;
     bool *cbSetting = (bool *) cbWidget->user_data();
     *cbSetting = cb->value();
}

void DisplayConfigWindow::ClearAllStickyFolderNamesCallback(Fl_Widget *btn, void *udata) {
     RNAStructViz::BackupAndUnlinkLocalConfigFiles(true);
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
}

void DisplayConfigWindow::RedrawTimerCallback(void *data) {
     DisplayConfigWindow *displayWin = (DisplayConfigWindow *) data;
     displayWin->redraw();
     Fl::remove_timeout(RedrawTimerCallback, data);
     Fl::add_timeout(1.0, RedrawTimerCallback, data);
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
}
