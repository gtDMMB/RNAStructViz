/* DisplayConfigWindow.h : Window to display and change user-configurable 
 *                         options within the running program; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#ifndef _DISPLAY_CONFIG_WINDOW_H_
#define _DISPLAY_CONFIG_WINDOW_H_

#include <cairo.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Cairo.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Button.H>

#include <vector>

#include "ConfigOptions.h"

#define CONFIG_WINDOW_WIDTH         (750)
#define CONFIG_WINDOW_HEIGHT        (450)
#define CONFIG_WINDOW_TITLE         ("RNAStructViz Configuration Settings")

#define CFGWIN_WIDGET_OFFSETX       (10)
#define CFGWIN_WIDGET_OFFSETY       (10)
#define CFGWIN_LABEL_HEIGHT         (20)
#define CFGWIN_LABEL_WIDTH          (200)
#define CFGWIN_BUTTON_WIDTH         (200)
#define CFGWIN_COLOR_WIDTH          (25)
#define CFGWIN_SPACING              (15)
#define CFGWIN_CDIALOG_X            (100)
#define CFGWIN_CDIALOG_Y            (100)
#define CFGWIN_CDIALOG_WIDTH        (250)
#define CFGWIN_CDIALOG_HEIGHT       (250)
#define CFGWIN_MAX_FPATH_LENGTH     (48)

class DisplayConfigWindow : public Fl_Cairo_Window { 

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
	
	        static void SelectDirectoryCallback(Fl_Widget *btn, void *ud);	
		static void UpdatePNGPathCallback(Fl_Widget *btn, void *udata);
		static void ChangeColorCallback(Fl_Widget *btn, void *udata); 
                static void WriteConfigFileCallback(Fl_Widget *btn, void *);
		static void WindowCloseCallback(Fl_Widget *win, void *udata);

		bool finished; 

		cairo_surface_t *crSurface;
		cairo_t *crDraw;
		int imageStride;
		uchar *imageData;

                #define NUMSETTINGS      (3)
                #define GUICOLORS        (4)

		Fl_RGB_Image *fpathsIcon, *themesIcon, *pngNewPathIcon;
	        Fl_Box *fpathsSettingBoxes[NUMSETTINGS];
	        char *fpathsUpdateRefs[NUMSETTINGS];	
		Fl_Box *colorDisplayBoxes[GUICOLORS];
		Fl_Color *colorChangeRefs[GUICOLORS];
		std::vector<Fl_Widget *> windowWidgets;
};

#endif
