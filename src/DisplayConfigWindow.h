/* DisplayConfigWindow.h : Window to display and change user-configurable 
 *                         options within the running program; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#ifndef _DISPLAY_CONFIG_WINDOW_H_
#define _DISPLAY_CONFIG_WINDOW_H_

#include <cairo.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Cairo.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Button.H>

#include <vector>

#include "ConfigOptions.h"
#include "ConfigExterns.h"

#define CONFIG_WINDOW_WIDTH         (810)
#define CONFIG_WINDOW_HEIGHT        (665)
#define CONFIG_WINDOW_TITLE         ("RNAStructViz Configuration Options and Settings")

#define CFGWIN_WIDGET_OFFSETX       (10)
#define CFGWIN_WIDGET_OFFSETY       (10)
#define CFGWIN_LABEL_HEIGHT         (18)
#define CFGWIN_LABEL_WIDTH          (205)
#define CFGWIN_BUTTON_WIDTH         (150)
#define CFGWIN_COLOR_WIDTH          (32)
#define CFGWIN_SPACING              (7)
#define CFGWIN_CDIALOG_X            (100)
#define CFGWIN_CDIALOG_Y            (100)
#define CFGWIN_CDIALOG_WIDTH        (250)
#define CFGWIN_CDIALOG_HEIGHT       (250)
#define CFGWIN_MAX_FPATH_LENGTH     (256)

class DisplayConfigWindow : public Fl_Double_Window { 

	public:
		static bool SetupInitialConfig(); 
	        static char * TrimFilePathDisplay(const char *path, int maxlen);
	
	public:
		DisplayConfigWindow();
		~DisplayConfigWindow();

		void ConstructWindow();
		bool isDone() const;

	protected:
                void drawWidgets();        
		static void Draw(Fl_Cairo_Window *crWin, cairo_t *cr);

		static bool UpdateApplicationThemeByName(const char *themeName);
		static bool UpdateApplicationThemeByIndex(int themeIdx);

	        static void SelectDirectoryCallback(Fl_Widget *btn, void *ud);	
		static void UpdatePNGPathCallback(Fl_Widget *btn, void *udata);
		static void FLTKThemeChoiceMenuCallback(Fl_Widget *menu, void *);
		static void PresetThemeChooserMenuCallback(Fl_Widget *, void *);
		static void ChangeColorCallback(Fl_Widget *btn, void *udata); 
                static void SelectFromColormapCallback(Fl_Widget *btn, void *);
		static void ChangeDiagramWindowArcColorCallback(Fl_Widget *btn, void *udata);
		static void ToggleCheckBoxParamCallback(Fl_Widget *cb, void *udata);
		static void ClearAllStickyFolderNamesCallback(Fl_Widget *btn, void *udata);
		static void WriteConfigFileCallback(Fl_Widget *btn, void *);
		static void RestoreDefaultsCallback(Fl_Widget *btn, void *);
		static void RedrawTimerCallback(void *data);
		static void WindowCloseCallback(Fl_Widget *win, void *udata);

		bool finished;

		cairo_surface_t *crSurface;
		cairo_t *crDraw;
		int imageStride;
		uchar *imageData;

                #define NUMSETTINGS      (4)
                #define GUICOLORS        (4)

		Fl_RGB_Image *fpathsIcon, *themesIcon;
		Fl_RGB_Image *dwinSettingsIcon, *pngNewPathIcon;
	        Fl_RGB_Image *cfgCheckboxesIcon;
		Fl_Box *fpathsSettingBoxes[NUMSETTINGS];
	        char *fpathsUpdateRefs[NUMSETTINGS];	
		Fl_Box *colorDisplayBoxes[GUICOLORS];
		volatile Fl_Color *colorChangeRefs[GUICOLORS];
		volatile Fl_Color *dwinArcColorChangeRefs[3][7];
		std::vector<Fl_Widget *> windowWidgets;
};

#endif
