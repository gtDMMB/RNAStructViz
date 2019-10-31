/* CommonDialogs.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.20
 */

#include <iterator>

#include "CommonDialogs.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

void CommonDialogs::DisplayFirstRunInstructions(Fl_RGB_Image *dialogIcon) {

     const char *instTextBuffer[] = {
          "Welcome to RNAStructViz!\n\n",
          "► A detailed user manual and how-to guide is available on the project WIKI:\n",
          "« https://github.com/gtDMMB/RNAStructViz/wiki »\n\n",
          "► Instructions to help new users first running the application\n",
          "is found online at:\n",
          "« https://github.com/gtDMMB/RNAStructViz/wiki/FirstRunInstructions »\n\n",
          "► If you need help while running RNAStructViz, click on the help button\n",
          "(circled question mark) at the top-right corner of the left main window pane.\n\n",
          "► Sample structures are available for you to use with the application. You can\n",
          "copy them into your home directory from the install directory using the first-run\n",
          "instructions link above, or by importing them from the \"User Config\" window button.\n\n",
          "► Users may view a summary tour of RNAStructViz features by clicking on the\npath ",
          "button in the upper right left pane navigation bar\n\n",
          "Thank you for using our application!", 
     };
     char fullInstText[4 * MAX_BUFFER_SIZE];
     fullInstText[0] = '\0';
     for(int bufline = 0; bufline < GetArrayLength(instTextBuffer); bufline++) {
          strcat(fullInstText, instTextBuffer[bufline]);
     }
     fl_message_title("╠ ---  Welcome to your first install of RNAStructViz  --- ╣");
     fl_message_icon()->image(dialogIcon);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     int userHelpSelection = fl_choice(fullInstText,
                                       "Copy WIKI link to clipboard", "Dismiss", 
                                       "Do not show again at start-up");
     switch(userHelpSelection) {
          case 0: {
               const char *firstTimeRunLink =
                           "https://github.com/gtDMMB/RNAStructViz/wiki";
               Fl::copy(firstTimeRunLink, strlen(firstTimeRunLink), 1, Fl::clipboard_plain_text);
               break;
          }
          case 2: {
               DISPLAY_FIRSTRUN_MESSAGE = false;
               ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
               break;
          }
          default:
               break;
     }

}

void CommonDialogs::DisplayHelpDialog() {
     CommonDialogs::DisplayFirstRunInstructions(CommonDialogs::helpIconImage);
}

void CommonDialogs::DisplayTourDialog() {


//"\n\nPlease feel free to send us questions, comments, \n",
//          "and/or leave us general feedback on your user experience by writing to gtdmmb@gatech.edu, \n",
//          "or by posting a new GitHub issue thread using the instructions at the following link:\n",
//          "https://github.com/gtDMMB/RNAStructViz/wiki/BugReportingAndErrors."



}

void CommonDialogs::DisplayInfoAboutDialog() {

     string infoWelcomeMsg = string("The next table summarizes the compile and current runtime time stats ") + 
	                     string("associated with this build of RNAStructViz. A copy \nof the build-time ") + 
			     string("parameters on your system can be obtained by running the next ") + 
			     string("command in a terminal:\n") + 
			     string("$ RNAStructViz --about\n\n");
     string infoTableOrigData[] = {
           ApplicationBuildInfo::GitRevisionInfo(),
           ApplicationBuildInfo::GitRevisionDate(),
           ApplicationBuildInfo::FLTKVersionString(),
           ApplicationBuildInfo::BuildFLTKConfig(),
           ApplicationBuildInfo::CairoVersionString(),
           ApplicationBuildInfo::BuildPlatform(),
           ApplicationBuildInfo::LocalBuildDateTime(),
	   "",
	   (string("RNAStructViz Launch Path Command: ") + string(rnaStructVizExecPath)), 
	   (string("Current Working Directory (CWD): ") + string(runtimeCWDPath)), 
	   (string("Active System User (From ENV): ") + string(activeSystemUserFromEnv)),
     };
     string spaces = string("................................................................................");
     int tableHeaderWidth = 42;
     string infoMsg = infoWelcomeMsg;
     for(int data = 0; data < GetArrayLength(infoTableOrigData); data++) {
          string curInfoStr = infoTableOrigData[data];
	  if(!strcmp(curInfoStr.c_str(), "")) {
	       infoMsg += string("\n");
	       continue;
	  }
	  size_t headerPos = curInfoStr.find_first_of(":");
          string headerPrefix = curInfoStr.substr(0, headerPos);
          string tableData = curInfoStr.substr(headerPos + 2);
          string headerSpacing = spaces.substr(0, MAX(0, tableHeaderWidth - headerPrefix.length()));
          char fullLineData[MAX_BUFFER_SIZE];
          snprintf(fullLineData, MAX_BUFFER_SIZE - 1, " >> %s %s %s\n", headerPrefix.c_str(), 
		   headerSpacing.c_str(), tableData.c_str());
          fullLineData[MAX_BUFFER_SIZE - 1] = '\0';
          infoMsg += string(fullLineData);
     }

     fl_message_title("About the Application : RNAStructViz Build and Current Runtime Information");
     fl_message_icon()->image(CommonDialogs::infoIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     fl_message_font(FL_COURIER_BOLD, 12);
     fl_message(infoMsg.c_str());

}

void CommonDialogs::DisplayContactDialog() {}

void CommonDialogs::DisplayKeyboardShortcutsDialog() {}
