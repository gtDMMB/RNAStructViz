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

#include "ConfigOptions.h"
#include "RNAStructure.h"

#define MAX_WEBDB_LOOKUPS                       (1)
#define DEFAULT_STICKY_FOLDERNAME_CFGFILE       ("sequence-folder-names.dat")

char * LookupStickyFolderNameForSequence(const char *baseSeqSpec);
char * LookupStickyFolderNameForSequence(const RNAStructure *rnaStructSpec);

#define LSEEK_NOT_FOUND                         ((off_t) -1)

off_t FolderNameForSequenceExists(const char *baseSeqSpec);
off_t FolderNameForSequenceExists(const RNAStructure *rnaStructSpec);
int SaveStickyFolderNameToConfigFile(const char *cfgFilePath, off_t replacePos = LSEEK_NOT_FOUND);

static const char *NCBI_ETOOL_UTIL_URLBASE[] = {
     "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi",
     "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/espell.fcgi",
};

static const char *NCBI_ETOOL_LOOKUP_PARAMS[] = {
     "nuccore", // TODO
     "text",
     "gtDMMB-RNAStructViz",
     "gtdmmb@gatech.edu",
     "<API-KEY-TODO>",
};
static const char *NCBI_ETOOL_LOOKUP_URLSPEC = "$1?db=$2&format=$3&tool=$4&email=$5&term=%%s&usehistory=y";

typedef enum {
     LOOKUP_BY_ACCNO,
     LOOKUP_BY_NAMESTR,
     LOOKUP_BY_BASESEQ,
} NCBIDatabaseLookupMethod;

char * CreateNCBISearchURLFromParams(const char *searchFieldDesc, 
		                     NCBIDatabaseLookupMethod searchMethod = LOOKUP_BY_BASESEQ);

typedef struct {
     char *baseSequenceData;
     char *seqAccessionNo;
     char *seqLatinName;
     char *seqExtraData;
} DBLookupResult_t;

void FreeDBLookupResultStruct(DBLookupResult_t * &lres);

/* https://stackoverflow.com/questions/11208299/how-to-make-an-http-get-request-in-c-without-libcurl */
char * PerformWebSearchLookup(const char *webURL, unsigned int maxDataBufLength = INT_MAX * sizeof(char) - 1);
char * SpacedSearchTermToWebQuery(const char *queryText); // replaces spaces, whitespace with +  
DBLookupResult_t * LookupBaseSequenceNCBI(const char *baseSeq);
DBLookupResult_t * LookupBaseSequenceNCBI(const RNAStructure *rnaStructObj);

/* Returns an array of two character strings, the first is the suggested folder name, the 
 * second is the tooltip text to use to display the full folder name data if it is too long to 
 * fit in the alotted button text length size. The pointer, and both of the contained strings need to 
 * be freed by the caller once the data is expired;
 */
char ** SuggestedNCBISearchFolderName(const char *prefixStr, DBLookupResult_t *orgData, 
                                      unsigned int maxStrLength = MAX_BUFFER_SIZE);

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

char ** SuggestedFolderNameFromFileComments(const char *seqDataFilePath, InputFileTypeSpec inputFileType);

#endif
