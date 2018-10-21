/* DisplayConfigWindow.h : Window to display and change user-configurable 
 *                         options within the running program; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#ifndef _DISPLAY_CONFIG_WINDOW_H_
#define _DISPLAY_CONFIG_WINDOW_H_

#include <cairo.h>
#include <FL/Fl_Cairo.H>
#include <FL/Fl_Cairo_Window.H>
#include <FL/Fl_Button.H>

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
		bool CloseWindow();

	protected:
                static void ThemeButtonCallback(Fl_Button *rb, void *userData); 
};

#endif
