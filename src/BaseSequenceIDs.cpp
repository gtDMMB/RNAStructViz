/* BaseSequenceIDs.cpp :
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.30
 */

#include <openssl/sha.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "BaseSequenceIDs.h"
#include "TerminalPrinting.h"
#include "ConfigParser.h"
#include "RNAStructure.h"

/* Hash Scheme: [FIRST 8 BYTES OF BSEQ + LAST 8 BYTES OF BSEQ + 256/8=32 BYTES OF SHA256 HASH] */
std::string HashBaseSequence(const char *baseSeq) {
     
     if(baseSeq == NULL) {
          return "";
     }

     int baseSeqLen = strlen(baseSeq);
     unsigned char hashBuf[SHA256_DIGEST_LENGTH];
     SHA256_CTX sha256HasherObj;
     SHA256_Init(&sha256HasherObj);
     SHA256_Update(&sha256HasherObj, baseSeq, MIN(baseSeqLen, MAX_BYTES_TO_HASH));
     SHA256_Final(hashBuf, &sha256HasherObj);

     std::stringstream ss;
     for(int hidx = 0; hidx < SHA256_DIGEST_LENGTH; hidx++) {
          ss << std::hex << std::setw(2) << std::setfill('0') << (int) hashBuf[hidx];
     }
     std::string hashBytes = ss.str();
     std::string baseSeqStr = string(baseSeq);
     hashBytes = baseSeqStr.substr(0, 8) + hashBytes + baseSeqStr.substr(baseSeqLen - 8, 8);

     return hashBytes;

}

char * LookupStickyFolderNameForSequence(const char *cfgFilePath, const char *baseSeqSpec) {
     if(cfgFilePath == NULL || baseSeqSpec == NULL) {
          return NULL;
     }
     off_t fnamePos = FolderNameForSequenceExists(cfgFilePath, baseSeqSpec);
     return LookupStickyFolderNameForSequence(cfgFilePath, fnamePos);
}

char * LookupStickyFolderNameForSequence(const char *cfgFilePath, RNAStructure *rnaStructSpec) {
     if(cfgFilePath == NULL || rnaStructSpec == NULL) {
          return NULL;
     }
     std::string baseSeq = std::string(rnaStructSpec->GetSequenceString());
     return LookupStickyFolderNameForSequence(cfgFilePath, baseSeq.c_str());
}

char * LookupStickyFolderNameForSequence(const char *cfgFilePath, off_t fnameFileOffset) {
     
     if(cfgFilePath == NULL || fnameFileOffset == LSEEK_NOT_FOUND) {
          return NULL;
     }
     std::string fullCfgFilePath = GetStickyFolderConfigPath(cfgFilePath);
     FILE *fpCfgFile = fopen(fullCfgFilePath.c_str(), "r");
     if(!fpCfgFile) {
      TerminalText::PrintError("Unable to open \"%s\" : %s I\n", fullCfgFilePath.c_str(), strerror(errno));
          return NULL;
     }
     char lineBuf[MAX_BUFFER_SIZE];
     fseek(fpCfgFile, fnameFileOffset, SEEK_SET);
     fgets(lineBuf, MAX_BUFFER_SIZE, fpCfgFile);
     std::string stickyStrEntry = std::string(lineBuf);
     fclose(fpCfgFile);

     size_t firstQuotePos = stickyStrEntry.find_first_of('\"');
     size_t lastQuotePos = stickyStrEntry.find_last_of('\"');
     if(firstQuotePos == std::string::npos || lastQuotePos == std::string::npos) {
          return NULL;
     }
     std::string savedFolderName = stickyStrEntry.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);
     char *rstr = (char *) malloc((savedFolderName.length() + 1) * sizeof(char));
     strcpy(rstr, savedFolderName.c_str());
     rstr[savedFolderName.length()] = '\0';
     return rstr;
     
}

off_t FolderNameForSequenceExists(const char *cfgFilePath, const char *baseSeqSpec) {
     if(cfgFilePath == NULL || baseSeqSpec == NULL) {
          return LSEEK_NOT_FOUND;
     }
     std::string fullCfgFilePath = GetStickyFolderConfigPath(cfgFilePath);
     FILE *fpCfgFile = fopen(fullCfgFilePath.c_str(), "r");
     if(!fpCfgFile) {
          TerminalText::PrintDebug("Unable to open \"%s\" : %s II\n", fullCfgFilePath.c_str(), strerror(errno));
          return LSEEK_NOT_FOUND;
     }
     std::string baseSeqHash = HashBaseSequence(baseSeqSpec);
     char lineBuf[MAX_BUFFER_SIZE];
     off_t curFileOffset = 0;
     bool foundMatch = false;
     while(!feof(fpCfgFile) && fgets(lineBuf, MAX_BUFFER_SIZE, fpCfgFile)) {
          if(!strncmp(baseSeqHash.c_str(), lineBuf, BSHASH_BYTES)) {
           foundMatch = true;
           break;
      }
      curFileOffset = ftell(fpCfgFile);
     }
     fclose(fpCfgFile);
     if(foundMatch) {
          return curFileOffset;
     }
     return LSEEK_NOT_FOUND;
}

off_t FolderNameForSequenceExists(const char *cfgFilePath, RNAStructure *rnaStructSpec) {
     if(cfgFilePath == NULL || rnaStructSpec == NULL) {
          return LSEEK_NOT_FOUND;
     }
     std::string baseSeq = std::string(rnaStructSpec->GetSequenceString());
     return FolderNameForSequenceExists(cfgFilePath, baseSeq.c_str());
}

int SaveStickyFolderNameToFirstConfigFile(const char *cfgFilePath, std::string baseSeq, 
                                          std::string folderName) {

     if(FolderNameForSequenceExists(cfgFilePath, baseSeq.c_str()) != LSEEK_NOT_FOUND) {
          return 0;
     }
     FILE *fpCfgFile = fopen(cfgFilePath, "w+");
     if(!fpCfgFile) {
          TerminalText::PrintError("Unable to open \"%s\": %s IV\n", cfgFilePath, strerror(errno));
      return errno;
     }
     std::string baseSeqHash = HashBaseSequence(baseSeq.c_str());
     fprintf(fpCfgFile, "%s;\"%s\"\n", baseSeqHash.c_str(), folderName.c_str());
     fclose(fpCfgFile);
     
     return EXIT_SUCCESS;

}
                     
int SaveStickyFolderNameToConfigFile(const char *cfgFilePath, std::string baseSeq, 
                                     std::string folderName, off_t replacePos) {
     
     if(cfgFilePath == NULL) {
          return EINVAL;
     }
     else if(FolderNameForSequenceExists(cfgFilePath, baseSeq.c_str()) != LSEEK_NOT_FOUND) {
          return EXIT_SUCCESS;
     }
     std::string fullCfgFilePath = GetStickyFolderConfigPath(cfgFilePath);
     if(!ConfigParser::fileExists(fullCfgFilePath.c_str())) {
          return SaveStickyFolderNameToFirstConfigFile(fullCfgFilePath.c_str(), baseSeq, folderName);
     }

     FILE *fpCfgFile = fopen(fullCfgFilePath.c_str(), "r");
     if(!fpCfgFile) {
          TerminalText::PrintError("Unable to open \"%s\" : %s III\n", fullCfgFilePath.c_str(), strerror(errno));
          return errno;
     }
     int fdCfgFile = fileno(fpCfgFile);
     char tempFilePath[MAX_BUFFER_SIZE];
     strcpy(tempFilePath, fullCfgFilePath.c_str());
     strcat(tempFilePath, ".temp");
     FILE *fpTempFile = fopen(tempFilePath, "w+");
     if(!fpTempFile) {
          TerminalText::PrintError("Unable to open \"%s\": %s IV\n", tempFilePath, strerror(errno));
          fclose(fpCfgFile);
          return errno;
     }

     std::string baseSeqHash = HashBaseSequence(baseSeq.c_str());

     char lineBuf[MAX_BUFFER_SIZE];
     off_t curCfgFileOffset = 0;
     while(fgets(lineBuf, MAX_BUFFER_SIZE, fpCfgFile)) {
           off_t nextOffset = ftell(fpCfgFile);
           if(curCfgFileOffset != replacePos) {
                fprintf(fpTempFile, "%s", lineBuf);
           }
           else {
                fprintf(fpTempFile, "%s;\"%s\"\n", baseSeqHash.c_str(), folderName.c_str());
           }
           curCfgFileOffset = nextOffset;
     }
     if(replacePos == LSEEK_NOT_FOUND) {
          fprintf(fpTempFile, "%s;\"%s\"\n", baseSeqHash.c_str(), folderName.c_str());
     }

     fclose(fpCfgFile);
     fclose(fpTempFile);
     if(rename(tempFilePath, fullCfgFilePath.c_str())) {
          TerminalText::PrintError("Unable to move \"%s\" to \"%s\" : %s\n", 
                                   tempFilePath, cfgFilePath, strerror(errno));
           return errno;
     }

     return EXIT_SUCCESS;

}

InputFileTypeSpec ClassifyInputFileTypeByExtension(const char *fileExt) {
     if(!strcasecmp(fileExt, "ct")) {
          return FILETYPE_CT;
     }
     else if(!strcasecmp(fileExt, "nopct")) {
      return FILETYPE_NOPCT;
     }
     else if(!strcasecmp(fileExt, "dot") || !strcasecmp(fileExt, "bracket") || !strcasecmp(fileExt, "dbn")) {
      return FILETYPE_DOTBRACKET;
     }
     else if(!strcasecmp(fileExt, "bpseq")) {
      return FILETYPE_BPSEQ;
     }
     else if(!strcasecmp(fileExt, "gtb")) {
      return FILETYPE_GTB;
     }
     else if(!strcasecmp(fileExt, "helix") || !strcasecmp(fileExt, "hlx")) {
      return FILETYPE_HLXTRIPLE;
     }
     return FILETYPE_NONE;
}

InputFileTypeSpec ClassifyInputFileType(const char *inputFilePath) {
     if(inputFilePath == NULL) {
          return FILETYPE_NONE;
     }
     const char *extPos = strrchr(inputFilePath, '.');
     if(extPos == NULL) {
          return FILETYPE_NONE;
     }
     return ClassifyInputFileTypeByExtension(extPos + 1);
}

/* Reference for header comment stanadards: 
 * https://rna.urmc.rochester.edu/Text/File_Formats.html
 */
std::string GetSequenceFileHeaderLines(const char *filePath, InputFileTypeSpec fileType) {
     
     if(fileType == FILETYPE_NONE) {
          return "";
     }
     // have appended sample numbers to the GTB files (need to remove this data to open the files):
     char actualFilePath[MAX_BUFFER_SIZE];
     if(fileType == FILETYPE_GTB) {
        strcpy(actualFilePath, filePath);
        const char *lastDashPos = strrchr(actualFilePath, '-');
        const char *extPos = strrchr(actualFilePath, '.');
        if(lastDashPos != NULL && extPos != NULL) {
             const char *origExtPos = strrchr(filePath, '.');
             size_t extStrLen = strlen(extPos);
             char *lastDashPosWritable = (char *) lastDashPos;
             strncpy(lastDashPosWritable, origExtPos, extStrLen);
             lastDashPosWritable[extStrLen] = '\0';
             filePath = (const char *) actualFilePath;
        }
     }
     FILE *fpInputSeq = fopen(filePath, "r");
     if(!fpInputSeq) {
          TerminalText::PrintError("Unable to open file \"%s\" : %s V\n", filePath, strerror(errno));
          return "";
     }
     std::string headerLines;
     char lineBuf[MAX_BUFFER_SIZE];
     bool addToHeaderStr = false, stopParsing = false;
     int lineCount = 0;
     while(!feof(fpInputSeq) && fgets(lineBuf, MAX_BUFFER_SIZE, fpInputSeq)) {
          int lineLength = strlen(lineBuf);
          ++lineCount;
          if(lineLength == 0) {
               stopParsing = true;
               break;
          }
          switch(fileType) {
                 case FILETYPE_NOPCT: {
                    std::string searchStr = std::string(lineBuf);
                    if(searchStr.length() > 0 && searchStr.at(0) == '>' || 
		       StringFindNoCase("Filename:", searchStr) != std::string::npos || 
                       StringFindNoCase("Organism:", searchStr) != std::string::npos || 
                       StringFindNoCase("Accession", searchStr) != std::string::npos || 
		       StringFindNoCase("Name:", searchStr) != std::string::npos) {
                        addToHeaderStr = true;
                    }
                    else {
                        stopParsing = true;
                   }
                   break;
                 }
                 case FILETYPE_CT:
                      if(lineCount == 1) {
                          addToHeaderStr = true;
                      } 
                      else {
                          stopParsing = true;
                      }
                      break;
                 case FILETYPE_BPSEQ:
                      if(lineBuf[0] != ';') {
                          addToHeaderStr = true;
                          stopParsing = true;
                      }
                      break;
                 case FILETYPE_DOTBRACKET:
                 case FILETYPE_GTB:
                 case FILETYPE_HLXTRIPLE:
                      if(lineBuf[0] == '>') {
                          addToHeaderStr = true;
                      }
                      else {
                          stopParsing = true;
                      }
                      break;
                 default:
                      stopParsing = true;
                      break;
          }
          if(addToHeaderStr) {
               headerLines += string(lineBuf);
          }
          if(stopParsing) {
               break;
          }
          addToHeaderStr = false;
     }
     fclose(fpInputSeq);
     return headerLines;

}
