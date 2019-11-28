/* ConfigParser.h : Header defines the user configuration file parser; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#ifndef IS_DIR
     #define IS_DIR(mode)     ((mode & S_IFMT) == S_IFDIR)
#endif
#ifndef IS_FILE
     #define IS_FILE(mode)    ((mode & S_IFMT) == S_IFREG)
#endif

class ConfigParser {

	public:
		char ctFileSearchDirectory[MAX_BUFFER_SIZE];
                char pngOutputDirectory[MAX_BUFFER_SIZE];
		char pngOutputPath[MAX_BUFFER_SIZE];
		char pngRadialLayoutOutputPath[MAX_BUFFER_SIZE];
		char fltkTheme[MAX_BUFFER_SIZE];
		char localTheme[MAX_BUFFER_SIZE];
		Fl_Color guiWindowBGColor;
		Fl_Color guiBGColor;
		Fl_Color guiBTextColor;
		Fl_Color guiTextColor;
		Fl_Color guiCTFileViewColor;
		Fl_Color guiStructureDiagramColors[3][7];
                int guiStructureDiagramColorsCount[3];
		bool guiDisplayFirstRunMessage;
		bool guiKeepStickyFolderNames;

	public:
		ConfigParser(); 
                ConfigParser(const char *userCfgFile, bool silenceErrors); 

		int parseFile(const char *userCfgFile, 
			      bool silenceErrors = false);
		int writeFile(const char *userCfgFile, 
			      bool silenceErrors = false) const;
		void storeVariables() const; 

		static void nullTerminateString(char *str, 
		            int nullCharPos = MAX_BUFFER_SIZE - 1); 
		
		static bool fileExists(const char *filePath);
		static bool directoryExists(const char *dirPath); 

		static void WriteUserConfigFile(const char *fpath = USER_CONFIG_PATH);
		static bool ParseAutoloadStructuresDirectory(const char *autoloadDirPath = USER_AUTOLOAD_PATH);

	private:
		void setDefaults(); 
		
		typedef struct {
			char cfgOption[MAX_BUFFER_SIZE];
			char cfgValue[MAX_BUFFER_SIZE];
		} ParsedConfigOption_t;
		int parseConfigLine(const char *configLine, ParsedConfigOption_t *result) const;
    
};



#endif
