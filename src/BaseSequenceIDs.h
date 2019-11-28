/* BaseSequenceIDs.h : Functionality to implement "sticky" folder names already used / typed 
 *                     by users for certain base (nucleotide) sequences. Also includes 
 *                     functions to try to automatically lookup some of this information 
 *                     on-the-fly via the NCBI databases;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.20
 */

#ifndef __BASE_SEQUENCE_IDS_H__
#define __BASE_SEQUENCE_IDS_H__

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <string>

#include "ConfigOptions.h"
#include "RNAStructVizTypes.h"

class RNAStructure;

#define FOLDER_NAME_DIVIDER                     (" -- ")
#define DEFAULT_STICKY_FOLDERNAME_CFGFILE       ("sequence-folder-names.dat")
#define GetStickyFolderConfigPath(cfgFile)      (std::string(USER_CONFIG_DIR) + std::string(cfgFile))

#define BSHASH_BYTES                            (48)
#define MAX_BYTES_TO_HASH                       (2048)
std::string HashBaseSequence(const char *baseSeq); 

char * LookupStickyFolderNameForSequence(const char *cfgFilePath, const char *baseSeqSpec);
char * LookupStickyFolderNameForSequence(const char *cfgFilePath, RNAStructure *rnaStructSpec);
char * LookupStickyFolderNameForSequence(const char *cfgFilePath, off_t fnameFileOffset);

#define LSEEK_NOT_FOUND                         ((off_t) -1)

off_t FolderNameForSequenceExists(const char *cfgFilePath, const char *baseSeqSpec);
off_t FolderNameForSequenceExists(const char *cfgFilePath, RNAStructure *rnaStructSpec);

int SaveStickyFolderNameToFirstConfigFile(const char *cfgFilePath, 
		                          std::string baseSeq, std::string folderName);
int SaveStickyFolderNameToConfigFile(const char *cfgFilePath, 
		                     std::string baseSeq, std::string folderName, 
		                     off_t replacePos = LSEEK_NOT_FOUND);

InputFileTypeSpec ClassifyInputFileTypeByExtension(const char *fileExt);
InputFileTypeSpec ClassifyInputFileType(const char *inputFilePath);
std::string GetSequenceFileHeaderLines(const char *filePath, InputFileTypeSpec fileType);

#endif
