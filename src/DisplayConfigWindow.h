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

#define CONFIG_WINDOW_WIDTH    (500)
#define CONFIG_WINDOW_HEIGHT   (500)
#define CONFIG_WINDOW_TITLE    ("RNAStructViz Configuration Settings")

#define CFGWIN_WIDGET_OFFSETX  (10)
#define CFGWIN_WIDGET_OFFSETY  (10)
#define CFGWIN_LABEL_HEIGHT    (15)
#define CFGWIN_LABEL_WIDTH     (400)
#define CFGWIN_SPACING         (15)

class DisplayConfigWindow : public Fl_Cairo_Window { 

	public:
		static bool SetupInitialConfig(); 
	
	public:
		DisplayConfigWindow(); 
		~DisplayConfigWindow();

		bool ApplyConfigOptions();
		bool isDone() const;

	protected:
                void drawWidgets();        
		static void Draw(Fl_Cairo_Window *crWin, cairo_t *cr);
		
		static void ThemeButtonCallback(Fl_Button *rb, void *userData); 
                static void WindowCloseCallback(Fl_Widget *win, void *udata);

		bool finished; 

		cairo_surface_t *crSurface;
		cairo_t *crDraw;
		int imageStride;
		uchar *imageData;

		Fl_RGB_Image *themesIcon, *pathsIcon;
		std::vector<Fl_Widget *> windowWidgets;
};

#endif
