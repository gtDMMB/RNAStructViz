/* RNACUtils.cpp : A quick, header-less homebrew implementation of some fast, 
 *                 but essential misc utility functions that do not fit elsewhere 
 *                 in the source code. Note that this file gets included for 
 *                 convenience by "ConfigOptions.h" -- SO EVERYTHING THAT GETS 
 *                 DEFINED HERE SHOULD BE STATIC AND LINKED INLINE!
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: Assembled from existing source 2019.03.01
 */

#define GetArrayLength(arr)             (arr != NULL ? sizeof(arr) / sizeof(arr[0]) : 0) 

template<typename PT>
static inline void DeletePointerCheck(PT *ptr) {
     if(ptr != NULL) {
          delete ptr;
     }
}

#define DeletePointerNoType(p)          ({ DeletePointerCheck<void>(p); p = NULL;  })
#define Delete(p, ptype)                ({ DeletePointerCheck<ptype>(p); p = NULL; })

static inline void FreePointerCheck(void *ptr) {
     if(ptr != NULL) {
          free(ptr);
     }
}

#define Free(p)                         ({ FreePointerCheck(p); p = NULL; })

typedef bool (*PredicateFunc_t)(char);

static inline const char* RemoveCharsFromStringByPredicate(char *str, PredicateFunc_t predicateFunc) {
     if(str == NULL) {
          return NULL;
     }
     char *chpos, *chwpos;
     for(chpos = str, chwpos = str; *chpos != '\0'; chpos++) {
          if(!predicateFunc(*chpos)) {
               *chwpos++ = *chpos;
      }
     }
     *chwpos = '\0';
     return str;
}

#define SetStringToEmpty(str)                     (str[0] = '\0')

#define StringRemoveWhitespace(s)                 RemoveCharsFromStringByPredicate(s, isspace)

#define StringsEqual(s1, s2)                      (!strcmp(s1, s2))
#define PointersEqual(p1, p2)                     ((p1 == p2) || *p1 == *p2)

template<typename TSpec>
static inline bool IsEqualDefaultCPPCheck(TSpec x, TSpec y) { 
     return (x) == (y);
}

template<typename TSpec>
static inline bool ElementInSet(TSpec universe[], TSpec element, 
                        bool (*compareFunc)(TSpec, TSpec) = 
                IsEqualDefaultCPPCheck) {
     for(int e = 0; e < GetArrayLength(universe); e++) {
          if(compareFunc(universe[e], element)) {
               return true;
      }
     }
     return false;
}

static inline int StringTranslateCharacters(char *str, char existingChar, char replChar) {
     if(str == NULL) {
          return 0;
     }
     int numReplacements = 0;
     for(char *chpos = str; *chpos != '\0'; chpos++) { 
          if(*chpos == existingChar) { 
               *chpos = replChar;
           ++numReplacements;
      }
     }
     return numReplacements;
}

#define RunAndReturn(Runner, Return) ({\
    volatile decltype(Return) returnVar = Return; \
    Runner; \
    returnVar; \
    })

#define StringMapCharacter(s, oldch, newch) RunAndReturn(StringTranslateCharacters(s, oldch, newch), s)

typedef int (*StringTransformFunc_t)(int);

static inline const char* StringTransform(char *str, StringTransformFunc_t transformFunc) {
     if(str == NULL) {
          return 0;
     }
     for(char *chpos = str; *chpos != '\0'; chpos++) {
          *chpos = transformFunc(*chpos);
     }
     return str;
}

#define StringToUppercase(s)                RunAndReturn(StringTransform(s, toupper), s)
#define StringToLowercase(s)                RunAndReturn(StringTransform(s, tolower), s)

static inline const char * GetUserHome() {
     const char *userHomeDir = getenv("HOME");
     if(userHomeDir == NULL) {
          struct passwd *uhdPasswd = getpwuid(getuid());
      if(uhdPasswd) 
           userHomeDir = uhdPasswd->pw_dir;
      else
           userHomeDir = "";
     }
     return userHomeDir;
}

#define WithinError(x, y, eps)              (abs(x - y) <= eps)

typedef struct {
     int x; 
     int y;
} Point_t;

#define MAX_SIZET                           ((size_t) -1)

static inline char * GetSubstringFromRange(const char *baseStr, size_t startPos, size_t endPos) {
     size_t baseStrLen = strlen(baseStr);
     if(endPos == MAX_SIZET) {
          endPos = baseStrLen ? baseStrLen - 1 : 0;
     }
     if(startPos > endPos || startPos >= baseStrLen || endPos >= baseStrLen) {
          return NULL;
     }
     size_t substrBufLen = endPos - startPos + 1;
     char *substrBuf = (char *) malloc((substrBufLen + 1) * sizeof(char));
     strncpy(substrBuf, baseStr + startPos, substrBufLen);
     substrBuf[substrBufLen] = '\0';
     return substrBuf;
}

#include <time.h>

static inline unsigned int GetRandomNaturalNumberInRange(unsigned int upperBound) {
     volatile time_t srandomSeed = time(NULL); 
     srand(srandomSeed); 
     if(upperBound > 0) {
          return rand() % upperBound;
     }
     return rand();
}

static inline char * UnicodeUTF8ToAscii(const char *srcUnicodeStr) {
     if(srcUnicodeStr == NULL) {
          return NULL;
     }
     unsigned int srcLength = strlen(srcUnicodeStr);
     unsigned int maxUnicodeDestSize = 4 * srcLength + 1;
     char *destAsciiStr = (char *) malloc(maxUnicodeDestSize * sizeof(char));
     if(destAsciiStr == NULL) {
          return NULL;
     }
     unsigned int bytesWritten = fl_utf8toa(srcUnicodeStr, srcLength, destAsciiStr, maxUnicodeDestSize);
     destAsciiStr[bytesWritten] = '\0';
     if(bytesWritten < maxUnicodeDestSize - 1) {
          destAsciiStr = (char *) realloc(destAsciiStr, bytesWritten + 1);
     }
     return destAsciiStr;
}

static inline char *getUserNameFromEnv(const char *envVarName, char *unameBuf, unsigned int bufLength) {
     if(envVarName == NULL || unameBuf == NULL) {
          return NULL;
     }
     char *envNameResult = getenv(envVarName);
     int envNameLength = strnlen(envNameResult, bufLength);
     int copyLength = bufLength > envNameLength + 1 ? envNameLength : bufLength;
     strncpy(unameBuf, envNameResult, copyLength);
     unameBuf[copyLength] = '\0';
     return unameBuf;
}

static inline char *getUserNameFromEnv(char *unameBuf, unsigned int bufLength) {
     char *unameQueryResult = getUserNameFromEnv("USER", unameBuf, bufLength);
     if(unameQueryResult != NULL) {
          return unameQueryResult;
     }
     unameQueryResult = getUserNameFromEnv("USERNAME", unameBuf, bufLength);
     if(unameQueryResult != NULL) {
          return unameQueryResult;
     }
     else if(bufLength > 0) {
          unameBuf[0] = '\0';
     }
     return NULL;
}

#include <boost/filesystem.hpp>

static inline bool FileFormatSortCmp(boost::filesystem::path filePathV1, boost::filesystem::path filePathV2) {
     
     std::string fileV1 = std::string(filePathV1.filename().c_str()), fileV2 = std::string(filePathV2.filename().c_str());
     size_t f1Ext = fileV1.rfind(".");
     size_t f2Ext = fileV2.rfind(".");
     
     /* 
      * NopCT files tend to contain more organism / structure naming data we can 
      * use in picking out good folder names than the other supported file formats. 
      * Thus, we prefer to process them first to get the most descriptive possible 
      * folder name suggestions by default. 
      */ 
     bool f1IsNopCT = f1Ext != std::string::npos && strcasecmp(fileV1.substr(f1Ext + 1).c_str(), "nopct");
     bool f2IsNopCT = f2Ext != std::string::npos && strcasecmp(fileV2.substr(f2Ext + 1).c_str(), "nopct");
     if(f1IsNopCT && f2IsNopCT) {
          return fileV1 < fileV2;
     }
     else if(f1IsNopCT && !f2IsNopCT) {
      return false;
     }
     else if(f2IsNopCT) {
      return true;
     }
     return fileV1 < fileV2;

}
