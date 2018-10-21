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

#include "ConfigOptions.h"

#define CONFIG_WINDOW_WIDTH    (500)
#define CONFIG_WINDOW_HEIGHT   (500)

class DisplayConfigWindow : public Fl_Cairo_Window { 

	public:
		static bool SetupInitialConfig(); 
	
	public:
		DisplayConfigWindow(); 
		~DisplayConfigWindow();

		bool ApplyConfigOptions();
		bool CloseWindow();

	protected:
		bool ChangeConfigWindowTheme(const char *themeName);

};

#endif
