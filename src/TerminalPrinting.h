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

namespace UnicodeTerminalChars {

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
          L"‼️💥⛔🤬",

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

}

/* ANSI color codes: */
namespace ANSIColor {
    
     typedef const char * ANSIColorCode;
     
     const ANSIColorCode BLACK           = "\x1b[0;30m";
     const ANSIColorCode RED             = "\x1b[0;31m";
     const ANSIColorCode GREEN           = "\x1b[0;32m";
     const ANSIColorCode BROWN           = "\x1b[0;33m";
     const ANSIColorCode BLUE            = "\x1b[0;34m";
     const ANSIColorCode PURPLE          = "\x1b[0;35m";
     const ANSIColorCode CYAN            = "\x1b[0;36m";
     const ANSIColorCode LIGHT_GRAY      = "\x1b[0;37m";
     const ANSIColorCode DARK_GRAY       = "\x1b[1;30m";
     const ANSIColorCode LIGHT_RED       = "\x1b[1;31m";
     const ANSIColorCode LIGHT_GREEN     = "\x1b[1;32m";
     const ANSIColorCode YELLOW          = "\x1b[1;33m";
     const ANSIColorCode LIGHT_BLUE      = "\x1b[1;34m";
     const ANSIColorCode LIGHT_PURPLE    = "\x1b[1;35m";
     const ANSIColorCode LIGHT_CYAN      = "\x1b[1;36m";
     const ANSIColorCode LIGHT_WHITE     = "\x1b[1;37m";
     const ANSIColorCode BOLD            = "\x1b[1m";
     const ANSIColorCode FAINT           = "\x1b[2m";
     const ANSIColorCode ITALIC          = "\x1b[3m";
     const ANSIColorCode UNDERLINE       = "\x1b[4m";
     const ANSIColorCode BLINK           = "\x1b[5m";
     const ANSIColorCode NEGATIVE        = "\x1b[7m";
     const ANSIColorCode CROSSED         = "\x1b[9m";
     const ANSIColorCode END             = "\x1b[0m";

     void PrintColorTextString(const char *textMsg, ANSIColorCode textFGColor, ANSIColorCode textStyle = NULL);

}

extern int PRINT_ANSI_COLOR;
extern int PRINT_TERMINAL_UNICODE;
extern int CFG_QUIET_MODE;
extern int CFG_DEBUG_MODE;
extern int CFG_VERBOSE_MODE;

namespace TerminalText {

     unsigned int SelectUnicodeIconIndex(UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, bool randomize);
     wchar_t GetUnicodeIconString(UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, bool randomize = true);

     void PrintNoColor(const char *msgFmt, ...);
     void PrintANSITerminalMessage(const char *prefixText, UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, 
		                   ANSIColor::ANSIColorCode prefixColor, ANSIColor::ANSIColorCode msgTextColor, 
				   bool randomizeUnicodeIdx, const char *msgFmt, va_list printArgs);
     void PrintError(const char *emsgFmt, ...);
     void PrintDebug(const char *dmsgFmt, ...);
     void PrintWarning(const char *wmsgFmt, ...);
     void PrintInfo(const char *imsgFmt, ...);

}

#endif
