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
#include "RNAStructure.h"

#define FOLDER_NAME_DIVIDER                     (" -- ")
#define DEFAULT_STICKY_FOLDERNAME_CFGFILE       ("sequence-folder-names.dat")

char * LookupStickyFolderNameForSequence(const char *cfgFilePath, const char *baseSeqSpec);
char * LookupStickyFolderNameForSequence(const char *cfgFilePath, const RNAStructure *rnaStructSpec);
char * LookupStickyFolderNameForSequence(const char *cfgFilePath, off_t fnameFileOffset);

#define LSEEK_NOT_FOUND                         ((off_t) -1)

off_t FolderNameForSequenceExists(const char *baseSeqSpec);
off_t FolderNameForSequenceExists(const RNAStructure *rnaStructSpec);

std::string ExtractSequenceNameFromButtonLabel(const char *buttonLabel);
int SaveStickyFolderNameToConfigFile(const char *cfgFilePath, std::string folderName, 
		                     off_t replacePos = LSEEK_NOT_FOUND);

typedef enum {
     FILETYPE_CT, 
     FILETYPE_NOPCT,
     FILETYPE_DOTBRACKET, 
     FILETYPE_BPSEQ,
     FILETYPE_GTB,
     FILETYPE_HLXTRIPLE,
     FILETYPE_FASTA,
} InputFileTypeSpec;

InputFileTypeSpec ClassifyInputFileTypeByExtension(const char *fileExt);
InputFileTypeSpec ClassifyInputFileType(const char *inputFilePath);
std::string GetSequenceFileHeaderLines(const char *filePath, InputFileTypeSpec fileType);

#endif
