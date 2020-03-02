/* MacSystem.h : Define specific actions and settings for the Mac OSX platform;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2020.03.02
 */

#ifndef __MAC_SYSTEM_H__
#define __MAC_SYSTEM_H__

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/x.H>

#include "ConfigOptions.h"
#include "CommonDialogs.h"
#include "RNAStructViz.h"
#include "StructureManager.h"
#include "MainWindow.h"

namespace MacSystem {

     static inline void HandleAboutApplicationMenuCallback(Fl_Widget *flwid, void *udata) {
          CommonDialogs::DisplayInfoAboutDialog();
     }

     static inline void HandleFileDropInApplicationCallback(const char *filePath) {
          StructureManager *structMgr = RNAStructViz::GetInstance()->GetStructureManager();
          structMgr->AddFile(filePath, true, false);
     }

     const int NUM_CUSTOM_MENU_ITEMS = 6;

     typedef enum {
          MAC_MENU_HELP      = 1, 
          MAC_MENU_INFO      = 2, 
          MAC_MENU_FIRSTRUN  = 3,
          MAC_MENU_TOUR      = 4,
          MAC_MENU_OPENFILES = 5, 
          MAC_MENU_CONFIG    = 6, 
     };
    
     static inline void HandleMacApplicationMenu(Fl_Widget *flwid, void *udata) {
          long int menuOption = (long int) udata;
          switch(menuOption) {
               case MAC_MENU_HELP:
                    CommonDialogs::DisplayHelpDialog();
                    break;
               case MAC_MENU_INFO:
                    CommonDialogs::DisplayInfoAboutDialog();
                    break;
               case MAC_MENU_FIRSTRUN:
                    CommonDialogs::DisplayFirstRunInstructions();
                    break;
               case MAC_MENU_TOUR:
                    CommonDialogs::DisplayTourDialog();
                    break;
               case MAC_MENU_OPENFILES:
                    MainWindow::OpenFileCallback();
                    break;
               case MAC_MENU_CONFIG:
                    MainWindow::ConfigOptionsCallback();
                    break;
               default:
                    break;
          }
     }

     static inline bool RegisterMacSystemCallbacks() {
          #ifdef __APPLE__
          fl_mac_set_about((Fl_Callback *) MacSystem::HandleAboutApplicationMenuCallback, NULL, 0);
          fl_open_callback(MacSystem::HandleFileDropInApplicationCallback);
          Fl_Mac_App_Menu::print = "Print a screenshot ...";
          Fl_Menu_Item *appMenuItems = (Fl_Menu_Item *) calloc(sizeof(Fl_Menu_Item), NUM_CUSTOM_MENU_ITEMS + 1);
          appMenuItems[0].add("Display help dialog ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_HELP, 0);
          appMenuItems[1].add("Display info dialog ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_INFO, 0);
          appMenuItems[2].add("Display first run instructions ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_FIRSTRUN, 0);
          appMenuItems[3].add("Display features tour dialog ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_TOUR, 0);
          appMenuItems[4].add("Open new structure files ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_OPENFILES, 0);
          appMenuItems[5].add("Show user config panel ...", 0, (Fl_Callback *) MacSystem::HandleMacApplicationMenu, (void *) MAC_MENU_CONFIG, 0);
          Fl_Mac_App_Menu::custom_application_menu_items(appMenuItems);
          return true;
          #endif
          return false;
     }

}

#endif
