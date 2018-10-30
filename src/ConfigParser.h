/* ConfigParser.h : Header defines the user configuration file parser; 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.21
 */

#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

class ConfigParser {

	public:
		char ctFileSearchDirectory[MAX_BUFFER_SIZE];
                char pngOutputDirectory[MAX_BUFFER_SIZE];
		char pngOutputPath[MAX_BUFFER_SIZE];
		char fltkTheme[MAX_BUFFER_SIZE];
		Fl_Color guiWindowBGColor;
		Fl_Color guiBGColor;
		Fl_Color guiBTextColor;
		Fl_Color guiTextColor;

	public:
		ConfigParser(); 
                ConfigParser(const char *userCfgFile); 

		int parseFile(const char *userCfgFile);
		int writeFile(const char *userCfgFile) const;
		void storeVariables() const; 

		static void nullTerminateString(char *str, 
				                int nullCharPos = MAX_BUFFER_SIZE - 1); 
	        static bool fileExists(const char *filePath);
		static bool directoryExists(const char *dirPath); 

	private:
		void setDefaults(); 
		
		typedef struct {
			char cfgOption[MAX_BUFFER_SIZE];
			char cfgValue[MAX_BUFFER_SIZE];
		} ParsedConfigOption_t;
		int parseConfigLine(const char *configLine, ParsedConfigOption_t *result) const;
    
};



#endif
