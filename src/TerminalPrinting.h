/* TerminalPrinting.h : Unicodes emojis, ANSI color definitions, and special print functions 
 *                      to handle special printing options to the terminal (e.g., 
 *                      error messages, extra status messages in verbose mode, and/or 
 *                      debugging messages for developers and testers;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.20
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef __TERMINAL_PRINTING_FUNCS_H__
#define __TERMINAL_PRINTING_FUNCS_H__

#define PRINTFP           (stderr)

typedef enum {
     ERROR_SYMBOLS        = 0,
     WARNING_SYMBOLS      = 1,
     INFO_SYMBOLS         = 2,
     UITYPE_SYMBOLS       = 3,
     HELP_SYMBOLS         = 4,
     RUNTIMEOS_SYMBOLS    = 5,
     NUMERAL_SYMBOLS      = 6, 
     MISC_SYMBOLS         = 7,
     SYMBOLS_ARRAY_LENGTH = 8,
} UnicodeSymbolTypeClass;

static const wchar_t *UNICODE_SYMBOL_LOOKUP[SYMBOLS_ARRAY_LENGTH] {

     /* ERROR SYMBOLS: */
     L"🛑‼️💥⛔🤬",

     /* WARNING SYMBOLS: */
     L"⚠️,", 

     /* INFO SYMBOLS: */
     L"ℹ️,💬🌟",

     /* UI-LIKE SYMBOLS: */
     L"☑️,✔️❎⌨️📁📂⚙️,🧰📋🔅🔆🆗✉️,", 

     /* HELP SYMBOLS: */
     L"❓",  

     /* RUNTIME-OS SYMBOLS: */
     L"🍎🐧🐡😈📎🗔,💻🖥️",

     /* NUMERAL SYMBOLS: */
     L"0️⃣ 1️⃣ 2️⃣ 3️⃣ 4️⃣ 5️⃣ 6️⃣ 7️⃣ 8️⃣ 9️⃣ 🔟#️⃣,",

     /* MISC SYMBOLS: */
     L"📈📊🧬🔬🧮,♾️🐞🐛👀👣" 

};

/* ANSI color codes: */
namespace ANSIColor {
    
     typedef const char * ANSIColorCode;
     
     static ANSIColorCode BLACK           = "\033[0;30m";
     static ANSIColorCode RED             = "\033[0;31m";
     static ANSIColorCode GREEN           = "\033[0;32m";
     static ANSIColorCode BROWN           = "\033[0;33m";
     static ANSIColorCode BLUE            = "\033[0;34m";
     static ANSIColorCode PURPLE          = "\033[0;35m";
     static ANSIColorCode CYAN            = "\033[0;36m";
     static ANSIColorCode LIGHT_GRAY      = "\033[0;37m";
     static ANSIColorCode DARK_GRAY       = "\033[1;30m";
     static ANSIColorCode LIGHT_RED       = "\033[1;31m";
     static ANSIColorCode LIGHT_GREEN     = "\033[1;32m";
     static ANSIColorCode YELLOW          = "\033[1;33m";
     static ANSIColorCode LIGHT_BLUE      = "\033[1;34m";
     static ANSIColorCode LIGHT_PURPLE    = "\033[1;35m";
     static ANSIColorCode LIGHT_CYAN      = "\033[1;36m";
     static ANSIColorCode LIGHT_WHITE     = "\033[1;37m";
     static ANSIColorCode BOLD            = "\033[1m";
     static ANSIColorCode FAINT           = "\033[2m";
     static ANSIColorCode ITALIC          = "\033[3m";
     static ANSIColorCode UNDERLINE       = "\033[4m";
     static ANSIColorCode BLINK           = "\033[5m";
     static ANSIColorCode NEGATIVE        = "\033[7m";
     static ANSIColorCode CROSSED         = "\033[9m";
     static ANSIColorCode END             = "\033[0m";

     void PrintColorTextString(const char *textMsg, ANSIColorCode textFGColor, ANSIColorCode textStyle = NULL);

};

extern int PRINT_ANSI_COLOR;
extern int PRINT_TERMINAL_UNICODE;
extern int CFG_QUIET_MODE;
extern int CFG_DEBUG_MODE;
extern int CFG_VERBOSE_MODE;

namespace TerminalText {

     unsigned int SelectUnicodeIconIndex(UnicodeSymbolTypeClass iconType, bool randomize);
     wchar_t GetUnicodeIconString(UnicodeSymbolTypeClass iconType, bool randomize = true);

     void PrintNoColor(const char *msgFmt, ...);
     void PrintANSITerminalMessage(const char *prefixText, UnicodeSymbolTypeClass iconType, 
		                   ANSIColor::ANSIColorCode prefixColor, ANSIColor::ANSIColorCode msgTextColor, 
				   bool randomizeUnicodeIdx, const char *msgFmt, va_list printArgs);
     void PrintError(const char *emsgFmt, ...);
     void PrintDebug(const char *dmsgFmt, ...);
     void PrintWarning(const char *wmsgFmt, ...);
     void PrintInfo(const char *imsgFmt, ...);

};

#endif
