/* RNACUtils.cpp : A quick, header-less homebrew implementation of some fast, 
 *                 but essential misc utility functions that do not fit elsewhere 
 *                 in the source code. Note that this file gets included for 
 *                 convenience by "ConfigOptions.h" -- SO EVERYTHING THAT GETS 
 *                 DEFINED HERE SHOULD BE STATIC AND LINKED INLINE -- OR ELSE!!
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: Assembled from existing source 2019.03.01
 */

#define GetArrayLength(arr)             (arr != NULL ? sizeof(arr) / sizeof(arr[0]) : 0) 

static inline void DeletePointerCheck(void *ptr) {
     if(ptr != NULL) {
          delete ptr;
     }
}

#define Delete(p)                       ({ DeletePointerCheck(p); p = NULL; })

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

