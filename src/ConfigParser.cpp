/* ConfigParser.h : Implements the user configuration file parser; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <FL/Fl.H>

#include "ConfigOptions.h"
#include "ConfigParser.h"
#include "TerminalPrinting.h"

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
	  else if(!strcmp(parsedLine.cfgOption, "FLTK_THEME")) {
	       strncpy(fltkTheme, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
	       nullTerminateString(fltkTheme);
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
	  "FLTK_THEME"
     };
     const char *cfgValues[] = { 
          ctFileSearchDirectory, 
	  pngOutputDirectory, 
	  pngOutputPath,
	  fltkTheme
     }; 
     const char *cfgColorOptions[] = {
          "GUI_WINDOW_BGCOLOR", 
	  "GUI_BGCOLOR", 
	  "GUI_BTEXT_COLOR", 
	  "GUI_TEXT_COLOR", 
	  "GUI_CTFILEVIEW_COLOR"
     };
     const Fl_Color cfgColorValues[] = {
          guiWindowBGColor, 
	  guiBGColor, 
	  guiBTextColor, 
          guiTextColor,
	  guiCTFileViewColor
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

     char lastOutputLine[MAX_BUFFER_SIZE];
     int lineLen = snprintf(lastOutputLine, MAX_BUFFER_SIZE, "DISPLAY_FIRSTRUN_MESSAGE=%s", 
	                    guiDisplayFirstRunMessage ? "true" : "false");
     nullTerminateString(lastOutputLine, MAX_BUFFER_SIZE - 1);
     if(!fwrite(lastOutputLine, sizeof(char), lineLen, fpCfgFile)) {
	  TerminalText::PrintError("Error writing line #%d to file: %s\n", 
	                           curLineNum, strerror(errno));
	  fclose(fpCfgFile);
	  return errno;
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
     strncpy((char *) FLTK_THEME, fltkTheme, MAX_BUFFER_SIZE - 1); 
     nullTerminateString((char *) FLTK_THEME, MAX_BUFFER_SIZE - 1); 

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

} 

void ConfigParser::nullTerminateString(char *str, int nullCharPos) { 
     if(str == NULL || nullCharPos < 0) {
          return;
     }
     str[nullCharPos] = '\0';
}

bool ConfigParser::fileExists(const char *filePath) { 
     if(filePath == NULL) { 
          return false;
     }
     struct stat fileInfo;
     return stat(filePath, &fileInfo) == 0 && IS_FILE(fileInfo.st_mode);
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
          int dirCreateErr = mkdir(USER_CONFIG_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	  if(dirCreateErr == -1) { 
	       TerminalText::PrintError("Unable to create directory \"%s\" ... Aborting\n", 
		                        USER_CONFIG_DIR);
	       perror("Directory Creation Error");
	       writeCfgFile = false;
	  }
     }
     if(writeCfgFile) {
          ConfigParser cfgParser;
          cfgParser.setDefaults();
	  cfgParser.writeFile(fpath, !DEBUGGING_ON);
     }
}

void ConfigParser::setDefaults() { 
     
     strncpy(ctFileSearchDirectory, (char *) CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(ctFileSearchDirectory);
     strncpy(pngOutputDirectory, (char *) PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputDirectory);
     strncpy(pngOutputPath, (char *) PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputPath);
     strncpy(fltkTheme, (char *) FLTK_THEME, MAX_BUFFER_SIZE - 1);
     nullTerminateString(fltkTheme);
     
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
