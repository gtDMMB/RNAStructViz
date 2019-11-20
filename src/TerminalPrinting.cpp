/* TerminalPrinting.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.30
 */


#include <wchar.h>
#include <stdarg.h>
#include <locale.h>

#include "ConfigOptions.h"
#include "TerminalPrinting.h"

int PRINT_ANSI_COLOR = true;
int PRINT_TERMINAL_UNICODE = true;
int CFG_QUIET_MODE = false;
int CFG_DEBUG_MODE = DEBUGGING_ON;
int CFG_VERBOSE_MODE = DEBUGGING_ON;

unsigned int TerminalText::SelectUnicodeIconIndex(UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, bool randomize) {
     if(!randomize && iconType >= 0 && iconType < UnicodeTerminalChars::SYMBOLS_ARRAY_LENGTH) {
          return 0;
     }
     return GetRandomNaturalNumberInRange(wcslen(UnicodeTerminalChars::UNICODE_SYMBOL_LOOKUP[iconType]));
}

wchar_t TerminalText::GetUnicodeIconString(UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, bool randomize) {
     return UnicodeTerminalChars::UNICODE_SYMBOL_LOOKUP[iconType][ TerminalText::SelectUnicodeIconIndex(iconType, randomize) ];
}

void TerminalText::PrintNoColor(const char *msgFmt, ...) {
     va_list argLst;
     va_start(argLst, msgFmt);
     vfprintf(PRINTFP, msgFmt, argLst);
     va_end(argLst);
}

void TerminalText::PrintANSITerminalMessage(const char *prefixText, 
		        UnicodeTerminalChars::UnicodeSymbolTypeClass iconType, 
                        ANSIColor::ANSIColorCode prefixColor, 
                        ANSIColor::ANSIColorCode msgTextColor, 
                        bool randomizeUnicodeIdx, const char *msgFmt, 
                        va_list printArgs) {
     if(!PRINT_ANSI_COLOR) {
          TerminalText::PrintNoColor(msgFmt, printArgs);
          return;
     }
     wchar_t unicodeLeadingIcon = PRINT_TERMINAL_UNICODE ? 
                                  TerminalText::GetUnicodeIconString(iconType, randomizeUnicodeIdx) : 0x00;
     setlocale(LC_CTYPE, "UTF-8");
     fprintf(PRINTFP, "%s%s", prefixColor, ANSIColor::BOLD);
     fwprintf(PRINTFP, L" %lc ", unicodeLeadingIcon != 0x00 ? unicodeLeadingIcon : L' ');
     fflush(PRINTFP);
     //freopen(NULL, "w", PRINTFP);
     setlocale(LC_CTYPE, "");
     fprintf(PRINTFP, "%s%s:%s ", ANSIColor::UNDERLINE, prefixText, ANSIColor::END);
     fprintf(PRINTFP, "%s%s", msgTextColor, ANSIColor::ITALIC);
     vfprintf(PRINTFP, msgFmt, printArgs);
     fprintf(PRINTFP, "%s", ANSIColor::END);
}

void TerminalText::PrintError(const char *emsgFmt, ...) {
     va_list argLst;
     va_start(argLst, emsgFmt);
     TerminalText::PrintANSITerminalMessage("ERROR", UnicodeTerminalChars::ERROR_SYMBOLS, ANSIColor::RED, 
                                            ANSIColor::LIGHT_RED, true, emsgFmt, argLst);
     va_end(argLst);
}

void TerminalText::PrintDebug(const char *dmsgFmt, ...) {
     if(CFG_QUIET_MODE || !CFG_DEBUG_MODE) {
          return;
     } 
     va_list argLst;
     va_start(argLst, dmsgFmt);
     TerminalText::PrintANSITerminalMessage("DEBUGGING", UnicodeTerminalChars::INFO_SYMBOLS, ANSIColor::GREEN, 
                                            ANSIColor::LIGHT_WHITE, true, dmsgFmt, argLst);
     va_end(argLst);
}

void TerminalText::PrintWarning(const char *wmsgFmt, ...) {
     if(CFG_QUIET_MODE || (!CFG_DEBUG_MODE && !CFG_VERBOSE_MODE)) {
          return;
     }
     va_list argLst;
     va_start(argLst, wmsgFmt);
     TerminalText::PrintANSITerminalMessage("WARNING", UnicodeTerminalChars::WARNING_SYMBOLS, ANSIColor::PURPLE, 
                                            ANSIColor::LIGHT_CYAN, true, wmsgFmt, argLst);
     va_end(argLst);
}

void TerminalText::PrintInfo(const char *imsgFmt, ...) {
     va_list argLst;
     va_start(argLst, imsgFmt);
     TerminalText::PrintANSITerminalMessage("STATUS", UnicodeTerminalChars::WARNING_SYMBOLS, ANSIColor::YELLOW, 
                                            ANSIColor::LIGHT_GRAY, true, imsgFmt, argLst);
     va_end(argLst);
}

