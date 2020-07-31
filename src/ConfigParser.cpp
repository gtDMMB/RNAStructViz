/* ConfigParser.h : Implements the user configuration file parser; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <string>

#include <FL/Fl.H>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "TerminalPrinting.h"
#include "ConfigExterns.h"
#include "ThemesConfig.h"
#include "RNAStructViz.h"

ConfigParser::ConfigParser() { 
     setDefaults(); 
}

ConfigParser::ConfigParser(const char *userCfgFile, 
                   bool silenceErrors = false) { 
     setDefaults(); 
     parseFile(userCfgFile, silenceErrors); 
} 

int ConfigParser::parseFile(const char *userCfgFile, bool silenceErrors) { 

     if(userCfgFile == NULL) {
          return -1;
     }

     FILE *fpCfgFile = fopen(userCfgFile, "r+");
     if(fpCfgFile == NULL && !silenceErrors) {
         TerminalText::PrintError("Unable to open file \"%s\": %s\n", 
                                  userCfgFile, strerror(errno));
      return errno;
     }
     else if(fpCfgFile == NULL) {
          return errno;
     }

     char nextLine[MAX_BUFFER_SIZE];
     ParsedConfigOption_t parsedLine;
     while(!feof(fpCfgFile)) { 
          if(!fgets(nextLine, MAX_BUFFER_SIZE - 1, fpCfgFile)) {
               if(!silenceErrors) 
                perror("Unable to read in config file line");
           fclose(fpCfgFile);
           return errno;
      }
      else if(errno = parseConfigLine(nextLine, &parsedLine)) {
           perror("Error reading config file line");
           fclose(fpCfgFile);
           return errno;
      }
      else if(!strcmp(parsedLine.cfgOption, "CTFILE_SEARCH_DIR")) { 
           if(strlen(parsedLine.cfgValue) > 0 && directoryExists(parsedLine.cfgValue)) {
                strncpy(ctFileSearchDirectory, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
            nullTerminateString(ctFileSearchDirectory);
           }
           else {
               TerminalText::PrintError("No such directory \"%s\" ... skipping the init.\n", 
                                        parsedLine.cfgValue);
           }
      }
      else if(!strcmp(parsedLine.cfgOption, "PNGOUT_DIR")) {
           if(strlen(parsedLine.cfgValue) > 0 && directoryExists(parsedLine.cfgValue)) {
                    strncpy(pngOutputDirectory, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
            nullTerminateString(pngOutputDirectory);
           }
           else {
               TerminalText::PrintError("Unknown PNG output dir \"%s\" ... skipping.\n", 
                                        parsedLine.cfgValue);
           }
      }
      else if(!strcmp(parsedLine.cfgOption, "PNGOUT_PATH")) {
           if(strlen(parsedLine.cfgValue) > 0) {
                strncpy(pngOutputPath, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
            nullTerminateString(pngOutputPath);
           }
      }
      else if(!strcmp(parsedLine.cfgOption, "PNGOUT_RLAYOUT_PATH")) {
               if(strlen(parsedLine.cfgValue) > 0) {
                strncpy(pngRadialLayoutOutputPath, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
            nullTerminateString(pngRadialLayoutOutputPath);
           }
      }
      else if(!strcmp(parsedLine.cfgOption, "FLTK_THEME")) {
           strncpy(fltkTheme, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
           nullTerminateString(fltkTheme);
      }
      else if(!strcmp(parsedLine.cfgOption, "LOCAL_THEME")) {
	   strncpy(localTheme, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
	   nullTerminateString(localTheme);
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_WINDOW_BGCOLOR")) { 
               guiWindowBGColor = strtol(parsedLine.cfgValue, NULL, 16);
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_BGCOLOR")) { 
               guiBGColor = strtol(parsedLine.cfgValue, NULL, 16);
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_BTEXT_COLOR")) { 
               guiBTextColor = strtol(parsedLine.cfgValue, NULL, 16);
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_TEXT_COLOR")) { 
               guiTextColor = strtol(parsedLine.cfgValue, NULL, 16);
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_CTFILEVIEW_COLOR")) { 
               guiCTFileViewColor = strtol(parsedLine.cfgValue, NULL, 16);
      }
      else if(!strcmp(parsedLine.cfgOption, "DISPLAY_FIRSTRUN_MESSAGE")) {
               guiDisplayFirstRunMessage = !strcasecmp(parsedLine.cfgValue, "true");
      }
      else if(!strcmp(parsedLine.cfgOption, "GUI_KEEP_STICKY_FOLDER_NAMES")) {
           guiKeepStickyFolderNames = !strcasecmp(parsedLine.cfgValue, "true") ? true : false;
      }
      else if(!strncmp(parsedLine.cfgOption, "DWIN_COLORS_STRUCT", 18) && 
              strlen(parsedLine.cfgOption) == 19) {
               int structIndex = atoi(parsedLine.cfgOption + 18) - 1;
           if(structIndex < 0 || structIndex >= 3) {
               TerminalText::PrintError("Unknown structure index \"%s\" ... skipping\n", 
                                    parsedLine.cfgOption + 18);
                    continue;
           }
           char *commaDelimPos = strchrnul(parsedLine.cfgValue, ',');
           char *curStrStartPos = parsedLine.cfgValue;
           bool reachedLastColor = false;
           int colorIdx = 0;
           while(!reachedLastColor) {
                 char colorStr[MAX_BUFFER_SIZE];
                 int colorStrLen = commaDelimPos - curStrStartPos;
                 strncpy(colorStr, curStrStartPos, colorStrLen);
                 colorStr[colorStrLen] = '\0';
                 guiStructureDiagramColors[structIndex][colorIdx] = strtol(colorStr, NULL, 16);
                 curStrStartPos = commaDelimPos + 1;
                 if(*commaDelimPos == '\0') {
                      reachedLastColor = true;
                 }
                 else {
                      commaDelimPos = strchrnul(curStrStartPos, ',');
                 }
                 colorIdx++;
           }
           guiStructureDiagramColorsCount[structIndex] = colorIdx;
      }
      else {
          TerminalText::PrintError("Unknown config option \"%s\" ... skipping.\n", 
                                       parsedLine.cfgOption);
      }
     }
     fclose(fpCfgFile);
     return 0;

} 

int ConfigParser::writeFile(const char *userCfgFile, bool silenceErrors) const { 

     if(userCfgFile == NULL) { 
          return -1;
     }
     
     int NUM_OPTIONS = 4;
     const char *cfgOptions[] = { 
          "CTFILE_SEARCH_DIR", 
          "PNGOUT_DIR", 
          "PNGOUT_PATH", 
          "PNGOUT_RLAYOUT_PATH",
          "FLTK_THEME",
	  "LOCAL_THEME",
     };
     const char *cfgValues[] = { 
          ctFileSearchDirectory, 
          pngOutputDirectory, 
          pngOutputPath,
          pngRadialLayoutOutputPath,
          fltkTheme,
	  localTheme,
     }; 
     const char *cfgColorOptions[] = {
          "GUI_WINDOW_BGCOLOR", 
          "GUI_BGCOLOR", 
          "GUI_BTEXT_COLOR", 
          "GUI_TEXT_COLOR", 
          "GUI_CTFILEVIEW_COLOR",
     };
     const Fl_Color cfgColorValues[] = {
          guiWindowBGColor, 
          guiBGColor, 
          guiBTextColor, 
          guiTextColor,
          guiCTFileViewColor,
     };

     FILE *fpCfgFile = fopen(userCfgFile, "w+"); 
     if(fpCfgFile == NULL && !silenceErrors) { 
           TerminalText::PrintError("Unable to open config file \"%s\" for writing: ",
                                    userCfgFile);
           TerminalText::PrintError("%s\n", strerror(errno)); 
           return errno;
     }
     else if(fpCfgFile == NULL) {
          return errno;
     }
     
     for(int line = 0; line < NUM_OPTIONS; line++) {
           char nextOutputLine[MAX_BUFFER_SIZE];
           int lineLength = snprintf(nextOutputLine, MAX_BUFFER_SIZE - 1, "%s=%s\n", 
                                     cfgOptions[line], cfgValues[line]); 
           nullTerminateString(nextOutputLine, MAX_BUFFER_SIZE - 1); 
           if(!fwrite(nextOutputLine, sizeof(char), lineLength, fpCfgFile)) { 
                TerminalText::PrintError("Error writing line #%d to file: %s\n", 
                                         line + 1, strerror(errno));
                fclose(fpCfgFile); 
                return errno;
           }
     }
     NUM_OPTIONS = 5;
     for(int line = 0; line < NUM_OPTIONS; line++) {
          char nextOutputLine[MAX_BUFFER_SIZE];
          int lineLength = snprintf(nextOutputLine, MAX_BUFFER_SIZE - 1, "%s=0x%08x\n", 
                                    cfgColorOptions[line], cfgColorValues[line]); 
          nullTerminateString(nextOutputLine, MAX_BUFFER_SIZE - 1); 
          if(!fwrite(nextOutputLine, sizeof(char), lineLength, fpCfgFile)) { 
                TerminalText::PrintError("Error writing line #%d to file: %s\n", 
                                         line + sizeof(cfgValues) + 1, strerror(errno));
                fclose(fpCfgFile); 
                return errno;
           }
     }
     int curLineNum = sizeof(cfgValues) + NUM_OPTIONS + 1;

     for(int s = 0; s < 3; s++) {
          char colorListLine[MAX_BUFFER_SIZE], curColorStr[MAX_BUFFER_SIZE];
           snprintf(colorListLine, MAX_BUFFER_SIZE, "DWIN_COLORS_STRUCT%d=\0", s + 1);
           for(int c = 0; c < guiStructureDiagramColorsCount[s]; c++) {
                snprintf(curColorStr, MAX_BUFFER_SIZE, "0x%08x\0", guiStructureDiagramColors[s][c]);
                strcat(colorListLine, curColorStr);
                if(c + 1 < guiStructureDiagramColorsCount[s]) {
                     strcat(colorListLine, ",");
                }
           }
           strcat(colorListLine, "\n");
           curLineNum++;
           if(!fwrite(colorListLine, sizeof(char), strlen(colorListLine), fpCfgFile)) {
                TerminalText::PrintError("Error writing line #%d to file: %s\n", curLineNum, strerror(errno));
                fclose(fpCfgFile);
                return errno;
           }
     }

     const char *BOOLEAN_VALUED_CFGOPTS[] = {
          "DISPLAY_FIRSTRUN_MESSAGE",
          "GUI_KEEP_STICKY_FOLDER_NAMES",
     };
     bool BOOLEAN_VALUED_CFGOPTS_VALUES[] = {
           guiDisplayFirstRunMessage,
           guiKeepStickyFolderNames,
     };
     int lineLen;
     char lastOutputLine[MAX_BUFFER_SIZE];
     for(int bi = 0; bi < GetArrayLength(BOOLEAN_VALUED_CFGOPTS); bi++) {
          const char *newlineStr = bi + 1 < GetArrayLength(BOOLEAN_VALUED_CFGOPTS) ? 
                                   "\n" : "";
           lineLen = snprintf(lastOutputLine, MAX_BUFFER_SIZE, "%s=%s%s", 
                              BOOLEAN_VALUED_CFGOPTS[bi], 
                              BOOLEAN_VALUED_CFGOPTS_VALUES[bi] ? "true" : "false", 
                              newlineStr);
           nullTerminateString(lastOutputLine, MAX_BUFFER_SIZE - 1);
           if(!fwrite(lastOutputLine, sizeof(char), lineLen, fpCfgFile)) {
                TerminalText::PrintError("Error writing line #%d to file: %s\n", 
                                         curLineNum, strerror(errno));
                fclose(fpCfgFile);
                return errno;
           }
     }

     fclose(fpCfgFile); 
     return 0;

}

void ConfigParser::storeVariables() const {

     strncpy((char *) CTFILE_SEARCH_DIRECTORY, ctFileSearchDirectory, MAX_BUFFER_SIZE - 1); 
     nullTerminateString((char *) CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     strncpy((char *) PNG_OUTPUT_DIRECTORY, pngOutputDirectory, MAX_BUFFER_SIZE - 1); 
     nullTerminateString((char *) PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     strncpy((char *) PNG_OUTPUT_PATH, pngOutputPath, MAX_BUFFER_SIZE - 1);
     nullTerminateString((char *) PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);  
     strncpy((char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH, pngRadialLayoutOutputPath, MAX_BUFFER_SIZE - 1);
     nullTerminateString((char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);  
     strncpy((char *) FLTK_THEME, fltkTheme, MAX_BUFFER_SIZE - 1); 
     nullTerminateString((char *) FLTK_THEME, MAX_BUFFER_SIZE - 1); 
     strcpy((char *) LOCAL_THEME_NAME, localTheme);
     nullTerminateString((char *) LOCAL_THEME_NAME, MAX_BUFFER_SIZE - 1);
     LOCAL_COLOR_THEME = GetColorThemeRefByName((const char *) LOCAL_THEME_NAME);

     GUI_WINDOW_BGCOLOR = guiWindowBGColor;
     GUI_BGCOLOR = guiBGColor;
     GUI_BTEXT_COLOR = guiBTextColor;
     GUI_TEXT_COLOR = guiTextColor;
     GUI_CTFILEVIEW_COLOR = guiCTFileViewColor;

     for(int s = 0; s < 3; s++) {
          for(int c = 0; c < guiStructureDiagramColorsCount[s]; c++) {
               STRUCTURE_DIAGRAM_COLORS[s][c] = guiStructureDiagramColors[s][c];
          }
     }

     DISPLAY_FIRSTRUN_MESSAGE = guiDisplayFirstRunMessage;
     GUI_KEEP_STICKY_FOLDER_NAMES = guiKeepStickyFolderNames;

} 

void ConfigParser::nullTerminateString(char *str, int nullCharPos) { 
     if(str == NULL || nullCharPos < 0) {
          return;
     }
     str[nullCharPos] = '\0';
}

bool ConfigParser::fileExists(const char *filePath, bool regularFile) { 
     if(filePath == NULL) { 
          return false;
     }
     struct stat fileInfo;
     return stat(filePath, &fileInfo) == 0 && (!regularFile || IS_FILE(fileInfo.st_mode));
}

bool ConfigParser::directoryExists(const char *dirPath) { 
     if(dirPath == NULL) {
          return false;
     }
     struct stat dirInfo;
     return stat(dirPath, &dirInfo) == 0 && IS_DIR(dirInfo.st_mode);
}

void ConfigParser::WriteUserConfigFile(const char *fpath) {
     bool writeCfgFile = true;
     if(!ConfigParser::directoryExists(USER_CONFIG_DIR)) {
	  try {
	       fs::path dirPath(USER_CONFIG_DIR);
               if(!fs::create_directories(dirPath)) { 
                    TerminalText::PrintError("Unable to create directory \"%s\" ... Aborting\n", 
                                             USER_CONFIG_DIR);
                    perror("Directory Creation Error");
                    writeCfgFile = false;
               }
	  } catch(fs::filesystem_error fse) {
	       TerminalText::PrintError("Unable to create directory \"%s\": %s\n", USER_CONFIG_DIR, fse.what());
	       writeCfgFile = false;
	  }
     }
     if(writeCfgFile) {
          ConfigParser cfgParser;
          cfgParser.setDefaults();
          cfgParser.writeFile(fpath, !DEBUGGING_ON);
     }
}

bool ConfigParser::ParseAutoloadStructuresDirectory(const char *autoloadDirPath) {
     if(!ConfigParser::directoryExists(autoloadDirPath)) {
	  try {
	       fs::path dirPath(autoloadDirPath);
	       if(!fs::create_directories(dirPath)) {
	            TerminalText::PrintError("Unable to create directory \"%s\" ... Skipping autoloading of structure files\n", 
			                     autoloadDirPath);
	            perror("Local autoload directory creation error");
	            return false;
	       }
	  } catch(fs::filesystem_error fse) {
	       TerminalText::PrintError("Unable to create directory \"%s\": %s ... Skipping autoloading of structure files\n", 
			                autoloadDirPath, fse.what());
	       return false;
	  }
     }
     std::string curFilePath;
     try {
        fs::path dirPath(autoloadDirPath);
        fs::directory_iterator cwdDirIter(dirPath);
        std::vector<boost::filesystem::path> sortedFilePaths(cwdDirIter, boost::filesystem::directory_iterator());
        std::sort(sortedFilePaths.begin(), sortedFilePaths.end(), FileFormatSortCmp);
        for(int sfileIdx = 0; sfileIdx < sortedFilePaths.size(); sfileIdx++) {
         curFilePath = sortedFilePaths[sfileIdx].filename().string();
	     bool isSymlink = fs::symlink_status(sortedFilePaths[sfileIdx]).type() == fs::symlink_file;
	     fs::path curStructFilePathCanon = fs::canonical( isSymlink ? fs::read_symlink(sortedFilePaths[sfileIdx]) : sortedFilePaths[sfileIdx] );
	     fs::path curStructFilePath = fs::absolute(curStructFilePathCanon, curStructFilePathCanon.parent_path());
	     //fprintf(stderr, " --> %s [%s]\n", curStructFilePath.string().c_str(), curFilePath.c_str());
         //if(curFilePath == "." || curFilePath.equals("..")) {
         //         continue;
         //}
	     if(fs::is_directory(curStructFilePath)) {
	          continue;
	     }
	     std::string parentDir = curStructFilePath.parent_path().string();
	     std::string fullStructFilePath = parentDir + "/" + curStructFilePath.filename().string();
	     TerminalText::PrintDebug("(Auto)Loading structure file at path \"%s\" ... \n", fullStructFilePath.c_str());
	     RNAStructViz::GetInstance()->GetStructureManager()->AddFile(fullStructFilePath.c_str(), false, true);
        }
     } catch(fs::filesystem_error fse) {
          TerminalText::PrintWarning("Unable to copy autoloaded file \"%s\": %s\n (ABORTING ENTIRE OPERATION)", curFilePath.c_str(), fse.what());
     }
     return true;
}

void ConfigParser::setDefaults() { 
     
     strncpy(ctFileSearchDirectory, (char *) CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(ctFileSearchDirectory);
     strncpy(pngOutputDirectory, (char *) PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputDirectory);
     strncpy(pngOutputPath, (char *) PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputPath);
     strncpy(pngRadialLayoutOutputPath, (char *) PNG_RADIAL_LAYOUT_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngRadialLayoutOutputPath);
     strncpy(fltkTheme, (char *) FLTK_THEME, MAX_BUFFER_SIZE - 1);
     nullTerminateString(fltkTheme);
     strncpy(localTheme, (char *) LOCAL_THEME_NAME, MAX_BUFFER_SIZE - 1);
     nullTerminateString(localTheme);

     guiWindowBGColor = GUI_WINDOW_BGCOLOR;
     guiBGColor = GUI_BGCOLOR;
     guiBTextColor = GUI_BTEXT_COLOR;
     guiTextColor = GUI_TEXT_COLOR;
     guiCTFileViewColor = GUI_CTFILEVIEW_COLOR;

     guiStructureDiagramColorsCount[0] = 1;
     guiStructureDiagramColorsCount[1] = 3;
     guiStructureDiagramColorsCount[2] = 7;
     for(int s = 0; s < 3; s++) {
          for(int c = 0; c < guiStructureDiagramColorsCount[s]; c++) {
               guiStructureDiagramColors[s][c] = STRUCTURE_DIAGRAM_COLORS[s][c];
          }
     }
     
     guiDisplayFirstRunMessage = DISPLAY_FIRSTRUN_MESSAGE;
     guiKeepStickyFolderNames = GUI_KEEP_STICKY_FOLDER_NAMES;

}

int ConfigParser::parseConfigLine(const char *configLine, ParsedConfigOption_t *result) const { 
     
     if(configLine == NULL || result == NULL) { 
          return -1;
     }

     int lineLength = strlen(configLine);
     if(configLine[lineLength - 1] == '\n') { 
      --lineLength;
     }

     int delimiterPos = -1;     
     for(int chpos = 0; chpos < lineLength; chpos++) { 
          if(configLine[chpos] == '=') { 
               delimiterPos = chpos; 
           break;
      }
     }
     if(delimiterPos < 0) {
          return -2;
     }
     strncpy(result->cfgOption, configLine, delimiterPos);
     nullTerminateString(result->cfgOption, delimiterPos);
     strncpy(result->cfgValue, configLine + delimiterPos + 1, lineLength - delimiterPos - 1);
     nullTerminateString(result->cfgValue, lineLength - delimiterPos - 1);

     return 0;

}
