/* CommonDialogs.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.20
 */

#include <iterator>

#include "CommonDialogs.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "RNAStructViz.h"
#include "ConfigExterns.h"

std::string CommonDialogs::GetHelpInstructionsMessageString() {

     const char *instTextBuffer[] = {
          "Welcome to RNAStructViz!\n\n",
          "► A detailed user manual and how-to guide is available on the project WIKI:\n",
          "« https://github.com/gtDMMB/RNAStructViz/wiki »\n",
          "► Instructions to help new users first running the application\n",
          "is found online at:\n",
          "« https://github.com/gtDMMB/RNAStructViz/wiki/FirstRunInstructions »\n",
          "► If you need help while running RNAStructViz, click on the help button\n",
          "(circled question mark) at the top-right corner of the left main window pane.\n",
          "► Sample structures are available for you to use with the application. You can\n",
          "copy them into your home directory by clicking on the middle button below, or \n", 
          "directly from the install directory using the first-run instructions link above.\n",
          "► Users may view a summary tour of RNAStructViz features by clicking on the\nhelp ",
          "button in the upper right left pane navigation bar and then on the left-most button.\n\n",
          "Thank you for using our application! ", 
          "Please feel free to send us questions, comments, \n",
          "and/or leave us general feedback on your user experience by \n",
          "by posting a new \nGitHub issue thread using the instructions at the following link:\n",
          "« https://github.com/gtDMMB/RNAStructViz/wiki/BugReportingAndErrors »", 
     };
     std::string fullInstText;
     for(int bufline = 0; bufline < GetArrayLength(instTextBuffer); bufline++) {
          fullInstText += std::string(instTextBuffer[bufline]);
     }
     return fullInstText;

}

void CommonDialogs::DisplayFirstRunInstructions() {
     
     std::string fullInstText = CommonDialogs::GetHelpInstructionsMessageString();    
     fl_message_title("╠  ---  Welcome to your first install of RNAStructViz  ---  ╣");
     fl_message_icon()->image(CommonDialogs::welcomeIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     fl_message_font(FL_COURIER_BOLD, 11);
     int userHelpSelection = fl_choice("%s",
                                       "Copy WIKI link to clipboard", 
                                       "Copy sample structures to your home directory", 
                                       "Do not show again at start-up", fullInstText.c_str());
     switch(userHelpSelection) {
          case 0: {
               const char *firstTimeRunLink =
                           "https://github.com/gtDMMB/RNAStructViz/wiki";
               Fl::copy(firstTimeRunLink, strlen(firstTimeRunLink), 1, Fl::clipboard_plain_text);
               break;
          }
          case 1: 
           RNAStructViz::CopySampleStructures();
           break;
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
     
     std::string fullInstText = CommonDialogs::GetHelpInstructionsMessageString();
     fl_message_title("╠  ---  Welcome to your first install of RNAStructViz  ---  ╣");
     fl_message_icon()->image(CommonDialogs::helpIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     fl_message_font(FL_COURIER_BOLD, 11);
     int userHelpSelection = fl_choice("%s",
                                       "Copy WIKI link to clipboard", 
                                       "Copy sample structures to your home directory", 
                                       "Display a tour of RNAStructViz", fullInstText.c_str());
     switch(userHelpSelection) {
          case 0: {
               const char *firstTimeRunLink =
                           "https://github.com/gtDMMB/RNAStructViz/wiki";
               Fl::copy(firstTimeRunLink, strlen(firstTimeRunLink), 1, Fl::clipboard_plain_text);
               break;
          }
      case 1: 
           RNAStructViz::CopySampleStructures();
           break;
      case 2: {
           CommonDialogs::DisplayTourDialog();
           break;
      }
      default:
           break;
     }
}

void CommonDialogs::DisplayTourDialog() {

     const char *tourTextBuffer[] = {
          "What does RNAStructViz do?\n\n",
          "► It loads sample structures in CT, NOPCT, DOT and several other standard formats.\n",
          "The loaded samples are then grouped into folders in the left-hand-side pane according to\n", 
          "their common base nucleotide sequences.\n", 
          "► After selecting an organism by clicking on its folder, the right-hand-side window pane is\n", 
          "populated with a set of operations that can compare the different loaded samples.\n\n", 
          "Users can then compare the samples for the selected organism by doing the following:\n\n", 
          "► Generate graphical arc diagrams to compare the overlap in pairings between up to three\n", 
          "samples at once. The arc diagram window also lets users zoom and select consecutive\n", 
          "sequences of bases to view in a radial layout viewer based on ViennaRNA.\n", 
          "► Compute comparitive statistics among all samples for the given organism given a \n", 
          "reference structure.\n", 
          "► Directly view the secondary structure base pairings in our color-coded integrated\n", 
          "CT-file-style file viewer (lower right-hand-side buttons).\n", 
          "► Export loaded samples to CT and DOT Bracket file formats starting from any input\n", 
          "sequence format RNAStructViz supports.\n", 
          "► Export statistical data and the visual arc diagram representations for the samples,\n", 
          "respectively, to the standard text-based CSV and PNG image formats.\n\n", 
          "The RNAStructViz WIKI page on GitHub is an excellent reference to get new users started with the\n", 
          "functionality built into the application. It can be found online by visiting the following link:\n", 
          "« https://github.com/gtDMMB/RNAStructViz/wiki »",
     };
     char fullTourText[4 * MAX_BUFFER_SIZE];
     fullTourText[0] = '\0';
     for(int bufline = 0; bufline < GetArrayLength(tourTextBuffer); bufline++) {
          strcat(fullTourText, tourTextBuffer[bufline]);
     }
     fl_message_title("╠  ---  Tour an overview of the features of RNAStructViz  ---  ╣");
     fl_message_icon()->image(CommonDialogs::tourIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     fl_message_font(FL_COURIER_BOLD, 11);
     fl_message("%s", fullTourText);

}

std::string CommonDialogs::GetInfoAboutMessageString() {
     
     string infoTableOrigData[] = {
	   ApplicationBuildInfo::GitReleaseType(),
	   ApplicationBuildInfo::GitRevisionInfo(),
           ApplicationBuildInfo::GitRevisionDate(),
           ApplicationBuildInfo::FLTKVersionString(),
           ApplicationBuildInfo::BuildFLTKConfig(),
           ApplicationBuildInfo::CairoVersionString(),
           ApplicationBuildInfo::BuildPlatform(),
           ApplicationBuildInfo::LocalBuildDateTime(),
           //"",
           (string("RNAStructViz Launch Path Command: ") + string(rnaStructVizExecPath)), 
           (string("Current Working Directory (CWD): ") + string(runtimeCWDPath)), 
           (string("Active System User (From ENV): ") + string(activeSystemUserFromEnv)),
     };
     string spaces = string("................................................................................");
     int tableHeaderWidth = 42;
     string infoMsg;
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
     return infoMsg;

}

void CommonDialogs::DisplayInfoAboutDialog() {

     string infoWelcomeMsg = string("The next table summarizes the compile and current runtime time stats ") + 
                         string("associated with this build of RNAStructViz. A copy \nof the build-time ") + 
                 string("parameters on your system can be obtained by running the next ") + 
                 string("command in a terminal:\n") + 
                 string("$ RNAStructViz --about\n\n");
     std::string infoMsg = infoWelcomeMsg + CommonDialogs::GetInfoAboutMessageString();

     fl_message_title("About the Application : RNAStructViz Build and Current Runtime Information");
     fl_message_icon()->image(CommonDialogs::infoIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(Lighter(GUI_BGCOLOR, 0.5f));
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     fl_message_font(FL_COURIER_BOLD, 12);
     int infoChoiceStatus = fl_choice("%s", "Copy about information to clipboard", 
                              "Close dialog", "Copy bug reporting WIKI link", 
                      infoMsg.c_str());
     switch(infoChoiceStatus) {
      case 0: {
               Fl::copy(infoMsg.c_str(), infoMsg.length(), 1, Fl::clipboard_plain_text);
               break;
          }
      case 2: {
               const char *bugReportingWIKILink =
                           "https://github.com/gtDMMB/RNAStructViz/wiki/BugReportingAndErrors";
               Fl::copy(bugReportingWIKILink, strlen(bugReportingWIKILink), 1, Fl::clipboard_plain_text);
               break;
          }
      default:
           break;
     }

}

void CommonDialogs::DisplayKeyboardShortcutsDialog() {}
