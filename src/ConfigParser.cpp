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

#ifndef IS_DIR
     #define IS_DIR(mode)     (mode == S_IFDIR)
#endif

ConfigParser::ConfigParser() { 
     setDefaults(); 
}

ConfigParser::ConfigParser(const char *userCfgFile) { 
     setDefaults(); 
     parseFile(userCfgFile); 
} 

int ConfigParser::parseFile(const char *userCfgFile) { 

     if(userCfgFile == NULL) {
          return -1;
     }

     FILE *fpCfgFile = fopen(userCfgFile, "r+");
     if(fpCfgFile == NULL) {
          fprintf(stderr, "Unable to open file \"%s\": %s\n", userCfgFile, strerror(errno));
	  return errno;
     }

     char nextLine[MAX_BUFFER_SIZE];
     ParsedConfigOption_t parsedLine;
     while(!feof(fpCfgFile)) { 
          if(!fgets(nextLine, MAX_BUFFER_SIZE - 1, fpCfgFile)) {
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
	            fprintf(stderr, "No such directory \"%s\" ... skipping this init.\n", 
		            parsedLine.cfgValue);
	       }
	  }
	  else if(!strcmp(parsedLine.cfgOption, "PNGOUT_DIR")) {
	       if(strlen(parsedLine.cfgValue) > 0 && directoryExists(parsedLine.cfgValue)) {
                    strncpy(pngOutputDirectory, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
		    nullTerminateString(pngOutputDirectory);
	       }
	       else {
	            fprintf(stderr, "Unknown PNG output dir \"%s\" ... skipping.\n", 
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
               if(Fl::is_scheme(parsedLine.cfgValue)) { 
	            strncpy(fltkTheme, parsedLine.cfgValue, MAX_BUFFER_SIZE - 1);
		    nullTerminateString(fltkTheme);
		}
		else {
		     fprintf(stderr, "No such FLTK theme \"%s\" ... skipping this.\b", 
		             parsedLine.cfgValue);
		}
          }
	  else {
	       fprintf(stderr, "Unknown config option \"%s\" ... skipping.\n", 
	               parsedLine.cfgOption);
	  }
     }
     fclose(fpCfgFile);
     return 0;

} 

int ConfigParser::writeFile(const char *userCfgFile) const { 

     if(userCfgFile == NULL) { 
          return -1;
     }
     
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
     
     FILE *fpCfgFile = fopen(userCfgFile, "r+"); 
     if(fpCfgFile == NULL) { 
          fprintf(stderr, "Unable to open user config file \"%s\" for writing: %s\n", 
	          userCfgFile, strerror(errno)); 
	  return errno;
     }
     for(int line = 0; line < sizeof(cfgValues); line++) {
	  char nextOutputLine[MAX_BUFFER_SIZE];
	  snprintf(nextOutputLine, MAX_BUFFER_SIZE - 1, "%s=%s\n", 
	           cfgOptions[line], cfgValues[line]); 
	  nullTerminateString(nextOutputLine, MAX_BUFFER_SIZE - 1); 
          if(!fwrite(cfgValues[line], sizeof(char), strlen(nextOutputLine), fpCfgFile)) { 
               fprintf(stderr, "Error writing line #%d to file: %s\n", 
	               line + 1, strerror(errno));
	       fclose(fpCfgFile); 
	       return errno;
	  }
     }
     fclose(fpCfgFile); 
     return 0;

}

void ConfigParser::storeVariables() const {

     strncpy(CTFILE_SEARCH_DIRECTORY, ctFileSearchDirectory, MAX_BUFFER_SIZE - 1); 
     nullTerminateString(CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     strncpy(PNG_OUTPUT_DIRECTORY, pngOutputDirectory, MAX_BUFFER_SIZE - 1); 
     nullTerminateString(PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1); 
     strncpy(PNG_OUTPUT_PATH, pngOutputPath, MAX_BUFFER_SIZE - 1);
     nullTerminateString(PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);  
     strncpy(FLTK_THEME, fltkTheme, MAX_BUFFER_SIZE - 1); 
     nullTerminateString(FLTK_THEME, MAX_BUFFER_SIZE - 1); 

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
     return stat(filePath, &fileInfo) == 0 && !IS_DIR(fileInfo.st_mode);
}

bool ConfigParser::directoryExists(const char *dirPath) { 
     if(dirPath == NULL) {
          return false;
     }
     struct stat dirInfo;
     return stat(dirPath, &dirInfo) == 0 && IS_DIR(dirInfo.st_mode);
}

void ConfigParser::setDefaults() { 
     
     strncpy(ctFileSearchDirectory, CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(ctFileSearchDirectory);
     strncpy(pngOutputDirectory, PNG_OUTPUT_DIRECTORY, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputDirectory);
     strncpy(pngOutputPath, PNG_OUTPUT_PATH, MAX_BUFFER_SIZE - 1);
     nullTerminateString(pngOutputPath);
     strncpy(fltkTheme, FLTK_THEME, MAX_BUFFER_SIZE - 1);
     nullTerminateString(fltkTheme);

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
     strncpy(result->cfgValue, configLine + delimiterPos, lineLength - delimiterPos - 1);
     
     return 0;

}
