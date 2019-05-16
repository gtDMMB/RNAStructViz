#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <vector>
#include <stack>
using std::stack;

#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>

#include "RNAStructViz.h"
#include "RNAStructure.h"
#include "StructureManager.h"
#include "ConfigOptions.h"
#include "ThemesConfig.h"

#if PERFORM_BRANCH_TYPE_ID
#include "BranchTypeIdentification.h"
#endif

const RNAStructure::BasePair RNAStructure::UNPAIRED = ~0x0;

RNAStructure::RNAStructure()
    : m_sequenceLength(0), m_sequence(NULL), 
      charSeq(NULL), dotFormatCharSeq(NULL), charSeqSize(0), 
      m_pathname(NULL), m_pathname_noext(NULL), 
      m_fileCommentLine(NULL), m_suggestedFolderName(NULL), 
      m_ctDisplayString(NULL), m_ctDisplayFormatString(NULL), 
      m_seqDisplayString(NULL), m_seqDisplayFormatString(NULL), 
      m_ctTextDisplay(NULL), m_ctStyleBuffer(NULL), 
      m_seqTextDisplay(NULL), m_seqStyleBuffer(NULL), 
      m_exportExtFilesBox(NULL), m_seqSubwindowBox(NULL), 
      m_ctSubwindowBox(NULL), m_ctViewerNotationBox(NULL), 
      m_exportFASTABtn(NULL), m_exportDBBtn(NULL) 	
{
     #if PERFORM_BRANCH_TYPE_ID
     branchType = NULL; 
     #endif
     m_contentWindow = NULL;
}

RNAStructure::RNAStructure(const RNAStructure &rnaStruct) { 
     copyRNAStructure(rnaStruct);
}

RNAStructure & RNAStructure::operator=(const RNAStructure &rhs) { 
     if(this != &rhs) {
          copyRNAStructure(rhs);
     }
     return *this;
}

void RNAStructure::copyRNAStructure(const RNAStructure &rnaStruct) { 
     fprintf(stderr, "TODO: NOT YET IMPLEMENTED!!\n");
}

RNAStructure::~RNAStructure()
{
    free(m_ctDisplayString);
    free(m_ctDisplayFormatString);
    free(m_seqDisplayString);
    free(m_seqDisplayFormatString); 
    free(m_sequence);
    if(charSeqSize > 0) { 
        free((void *) charSeq); 
	free((void *) dotFormatCharSeq);
    }
    if(m_pathname != NULL) {
    	free(m_pathname);
    }
    if(m_pathname_noext != NULL) {
        free(m_pathname_noext);
    }
    #if PERFORM_BRANCH_TYPE_ID
    if(branchType != NULL) {
         delete branchType;
         branchType = NULL;
    }
    #endif
    DeleteContentWindow();
    Delete(m_fileCommentLine); m_fileCommentLine = NULL;
    Delete(m_suggestedFolderName); m_suggestedFolderName = NULL;
}

RNAStructure::BaseData* RNAStructure::GetBaseAt(unsigned int position)
{
    if (position < m_sequenceLength)
    {
        return &m_sequence[position];
    }
    return NULL;
}

#if PERFORM_BRAMCH_TYPE_ID
RNABranchType_t* RNAStructure::GetBranchTypeAt(unsigned int position)
{
    if (position < m_sequenceLength)
    {
        return &branchType[position];
    }
    return NULL;
}
#endif

RNAStructure* RNAStructure::CreateFromFile(const char* filename, const bool isBPSEQ)
{
    std::ifstream inStream(filename);
    if (!inStream.good())
    {
		if (strlen(filename) > 1000)
		{
		    fprintf(stderr, "Unable to open file: <file name too long>");
		}
		else
		{
		    fprintf(stderr, "Unable to open file: %s", filename);
		}
		inStream.close();
		return 0;
    }

    RNAStructure* result = new RNAStructure();
    std::vector<char> tempSeq;
    result->m_sequenceLength = 0;
    unsigned int maxSize = 128;
    result->m_sequence = (BaseData*) malloc(sizeof(BaseData) * maxSize);
    int numElements = 0;
    while (true)
    {
	numElements++;
        unsigned int junk;

		// Check for a number. If not, ignore the line, or maybe the file is 
		// done.
		if (!(inStream >> junk))
		{
		    if (inStream.eof() || inStream.bad())
		    {
		    	break;
	            }
		    inStream.clear(); // Try clearing the fail
		
		    // Ignore the first line as a comment:
		    while (!inStream.eof() && inStream.get() != '\n') {
		         continue;
		    }
		}

		// Check for the next ID. If not, ignore the line.
		if (junk != result->m_sequenceLength + 1)
		{
		    while (!inStream.eof() && inStream.get() != '\n');
		    {
		    	continue;
		    }
		}

		char base = 0;
		inStream >> base;
		switch (base)
		{
		    case 'a':
		    case 'A':
			    result->m_sequence[result->m_sequenceLength].m_base = A;
	            tempSeq.push_back('a');
			    break;
		    case 'c':
		    case 'C':
			    result->m_sequence[result->m_sequenceLength].m_base = C;
	            tempSeq.push_back('c');
			    break;
		    case 'g':
		    case 'G':
			    result->m_sequence[result->m_sequenceLength].m_base = G;
	            tempSeq.push_back('g');
			    break;
		    case 't':
		    case 'T':
		    case 'u':
		    case 'U':
			    result->m_sequence[result->m_sequenceLength].m_base = U;
	            tempSeq.push_back('u');
			    break;
		    default: {
			    if (strlen(filename) > 980)
		    	{
			        fprintf(stderr, "Bad base: id %d, <file name too long>", result->m_sequenceLength + 1);
			    }
			    else
			    {
			        fprintf(stderr, "Bad base: id %d, file %s", result->m_sequenceLength + 1, filename);
			    }
			    delete result;
		    	inStream.close();
			    return 0;
		    }
		}

		if (!isBPSEQ)
		{
		    if (!(inStream >> junk))
		    {
				if (strlen(filename) > 980)
				{
				    fprintf(stderr, "Bad prev id: id %d, <file name too long>", result->m_sequenceLength + 1);
				}
				else
				{
				    fprintf(stderr, "Bad prev id: id %d, file %s", 
				    	    result->m_sequenceLength + 1, filename);
				}
				delete result;
				inStream.close();
				return 0;
		    }
	
		    if (!(inStream >> junk))
		    {
				if (strlen(filename) > 980) 
				{
				    fprintf(stderr, "Bad next id: id %d, <file name too long>", 
				    	    result->m_sequenceLength + 1);
				}
				else
				{
				    fprintf(stderr, "Bad next id: id %d, file %s", 
				    	    result->m_sequenceLength + 1, filename);
				}
				delete result;
				inStream.close();
				return 0;
		    }
		}

		if (!(inStream >> result->m_sequence[result->m_sequenceLength].m_pair))
		{
		    if (strlen(filename) > 980) 
		    {
				fprintf(stderr, "Bad pair: id %d, <file name too long>", 
					result->m_sequenceLength + 1);
		    }
		    else
		    {
				fprintf(stderr, "Bad pair: id %d, file %s", 
					result->m_sequenceLength + 1, filename);
		    }
		    delete result;
		    inStream.close();
		    return 0;
		}
		result->m_sequence[result->m_sequenceLength].m_index = result->m_sequenceLength;
        if (result->m_sequence[result->m_sequenceLength].m_pair == 0)
		{
		    result->m_sequence[result->m_sequenceLength].m_pair = UNPAIRED;
		}
		else
		{
		    result->m_sequence[result->m_sequenceLength].m_pair--;
		}
	
		if (!isBPSEQ)
		{
		    if (!(inStream >> junk))
		    {
				if (strlen(filename) > 980)
				{
				    fprintf(stderr, "Bad trailing id: id %d, <file name too long>", 
				    	    result->m_sequenceLength + 1);
				}
				else
				{
				    fprintf(stderr, "Bad trailing id: id %d, file %s", 
				    	    result->m_sequenceLength + 1, filename);
				}
				delete result;
				inStream.close();
				return 0;
		    }
		}
	
		result->m_sequenceLength++;
		if (result->m_sequenceLength == maxSize)
		{
		    maxSize += 100;
		    result->m_sequence = (BaseData*)realloc(result->m_sequence, 
                                                    sizeof(BaseData) * maxSize);
		}
    }
    inStream.close();

    if (result->m_sequenceLength == 0)
    {
		if (strlen(filename) > 990) 
		    fprintf(stderr, "Empty or malformed file: <file name too long>");
		else
		    fprintf(stderr, "Empty or malformed file: %s", filename);
		delete result;
		return 0;
    }

    result->m_sequence = (BaseData*) realloc(result->m_sequence, 
                                             sizeof(BaseData) * result->m_sequenceLength);
    
    #if PERFORM_BRANCH_TYPE_ID
    result->branchType = (RNABranchType_t*) malloc( 
                         sizeof(RNABranchType_t) * result->m_sequenceLength);
    #endif
    result->m_pathname = strdup(filename);
    result->charSeqSize = tempSeq.size();
    result->charSeq = (char *) malloc((result->charSeqSize + 1) * sizeof(char));
    result->dotFormatCharSeq = (char *) 
	    malloc((result->charSeqSize + 1) * sizeof(char));
    for(unsigned i = 0; i < tempSeq.size(); i++)
    {
        result->charSeq[i] = toupper(tempSeq.at(i));
	RNAStructure::BaseData *curBaseData = result->GetBaseAt(i);
	if(curBaseData->m_pair == UNPAIRED) { 
	     result->dotFormatCharSeq[i] = '.';
	}
	else if(curBaseData->m_index < curBaseData->m_pair) {
	     result->dotFormatCharSeq[i] = '(';
	}
	else { 
	     result->dotFormatCharSeq[i] = ')';
	}
    }
    result->charSeq[result->charSeqSize] = '\0';
    result->dotFormatCharSeq[result->charSeqSize] = '\0';
    #if PERFORM_BRANCH_TYPE_ID    
    RNABranchType_t::PerformBranchClassification(result, result->m_sequenceLength);
    #endif

    return result;
}

RNAStructure * RNAStructure::CreateFromDotBracketFile(const char *filename) {
     
     FILE *fpDotBracketFile = fopen(filename, "r+");
     if(fpDotBracketFile == NULL) {
          fprintf(stderr, "ERROR: Opening file \"%s\" : %s\n", filename, strerror(errno));
     }
     char lineBuf[MAX_SEQUENCE_SIZE + 1];
     char baseDataBuf[MAX_SEQUENCE_SIZE + 1], pairingDataBuf[MAX_SEQUENCE_SIZE + 1];
     bool haveBaseData = false, havePairData = false;
     while(true) {
          char *lineReturn = fgets(lineBuf, MAX_SEQUENCE_SIZE, fpDotBracketFile);
	  if(lineReturn == NULL && feof(fpDotBracketFile)) { 
	       break;
	  }
	  else if(lineReturn == NULL) {
	       fprintf(stderr, "ERROR: Reading DotBracket file \"%s\" : %s\n", filename, strerror(errno));
	       break;
	  }
	  if(lineBuf[0] == '\n' || lineBuf[0] == '>') { // blank or comment line (skip it): 
	       continue;
	  }
	  int lineLength = strnlen(lineBuf, MAX_SEQUENCE_SIZE);
	  if(lineBuf[lineLength - 1] == '\n') {
               lineBuf[lineLength - 1] = '\0';
	  }
	  if(!haveBaseData) {
	       strncpy(baseDataBuf, lineBuf, MAX_SEQUENCE_SIZE + 1);
	       haveBaseData = true;
	       continue;
	  }
	  else if(!havePairData) {
	       strncpy(pairingDataBuf, lineBuf, MAX_SEQUENCE_SIZE + 1);
	       havePairData = true;
	  }
	  if(haveBaseData && havePairData) {
	       break;
	  }
     }
     fclose(fpDotBracketFile);
     if(!haveBaseData || !havePairData || strlen(baseDataBuf) != strlen(pairingDataBuf)) {
          return NULL;
     }
     int seqLength = strlen(baseDataBuf);
     stack<int> unpairedBasePairs;
     RNAStructure *rnaStruct = new RNAStructure();
     rnaStruct->m_sequenceLength = seqLength;
     rnaStruct->m_sequence = (BaseData*) malloc(seqLength * sizeof(BaseData));
     for(int baseIdx = 0; baseIdx < seqLength; baseIdx++) { 
          RNAStructure::BaseData *curBaseData = &(rnaStruct->m_sequence[baseIdx]);
	  switch(baseDataBuf[baseIdx]) {
	       case 'a':
	       case 'A':
	            curBaseData->m_base = A;
		    break;
	       case 'c':
	       case 'C':
		    curBaseData->m_base = C;
		    break;
	       case 'g':
	       case 'G':
		    curBaseData->m_base = G;
		    break;
	       case 'u':
	       case 'U':
		    curBaseData->m_base = U;
		    break;
	       default:
		    curBaseData->m_base = X;
		    break;
	  }
	  curBaseData->m_index = baseIdx + 1;
	  if(pairingDataBuf[baseIdx] == '.') {
	       curBaseData->m_pair = UNPAIRED;
	  }
	  else if(pairingDataBuf[baseIdx] == '(' || pairingDataBuf[baseIdx] == '<' || 
		  pairingDataBuf[baseIdx] == '{') {
	       unpairedBasePairs.push(baseIdx + 1);
	  }
	  else if(pairingDataBuf[baseIdx] == ')' || pairingDataBuf[baseIdx] == '>' || 
		  pairingDataBuf[baseIdx] == '}') {
	       int pairIndex = unpairedBasePairs.top();
	       unpairedBasePairs.pop();
	       curBaseData->m_pair = pairIndex;
	       RNAStructure::BaseData *pairedBaseData = &(rnaStruct->m_sequence[pairIndex - 1]);
	       pairedBaseData->m_pair = baseIdx + 1;
	  }
	  else {
               fprintf(stderr, "ERROR: Unrecognized DOTBracket pairing character delimeter '%c'\n", 
		       pairingDataBuf[baseIdx]);
	       Delete(rnaStruct);
	       return NULL;
	  }
     }
     #if PERFORM_BRANCH_TYPE_ID
     rnaStruct->branchType = (RNABranchType_t*) malloc( 
                             sizeof(RNABranchType_t) * rnaStruct->m_sequenceLength);
     #endif
     rnaStruct->m_pathname = strdup(filename);
     rnaStruct->charSeqSize = seqLength;
     rnaStruct->charSeq = (char *) malloc((rnaStruct->charSeqSize + 1) * sizeof(char));
     strncpy(rnaStruct->charSeq, baseDataBuf, seqLength + 1);
     rnaStruct->charSeq[rnaStruct->charSeqSize] = '\0';
     rnaStruct->GenerateDotFormatDataFromPairings();
     #if PERFORM_BRANCH_TYPE_ID    
     RNABranchType_t::PerformBranchClassification(rnaStruct, rnaStruct->m_sequenceLength);
     #endif
     return rnaStruct;
}

void RNAStructure::GenerateDotFormatDataFromPairings() {
     if(dotFormatCharSeq != NULL) {
          free(dotFormatCharSeq);
     }
     dotFormatCharSeq = (char *) malloc((charSeqSize + 1) * sizeof(char));
     for(int pd = 0; pd < charSeqSize; pd++) {
         charSeq[pd] = toupper(charSeq[pd]);
	 RNAStructure::BaseData *curBaseData = GetBaseAt(pd);
	 if(curBaseData->m_pair == UNPAIRED) { 
	      dotFormatCharSeq[pd] = '.';
	 }
	 else if(curBaseData->m_index < curBaseData->m_pair) {
	      dotFormatCharSeq[pd] = '(';
	 }
	 else { 
	      dotFormatCharSeq[pd] = ')';
	 }
     }
     dotFormatCharSeq[charSeqSize] = '\0';
}

const char* RNAStructure::GetFilename() const
{
    // Get the base file name                             
    const char* basename = strrchr(m_pathname, '/');        
    if (!basename)
    {
        return m_pathname;
    }
    return ++basename;
}

const char* RNAStructure::GetFilenameNoExtension() { 
     if(m_pathname_noext) {
          return m_pathname_noext;
     }
     char *fullFilename = (char *) GetFilename();
     char *dotPosPtr = strrchr(fullFilename, '.');
     if(dotPosPtr == NULL) {
          return fullFilename;
     }
     size_t basePathLen = dotPosPtr - fullFilename;
     m_pathname_noext = (char *) malloc((basePathLen + 1) * sizeof(char));
     strncpy(m_pathname_noext, fullFilename, basePathLen);
     m_pathname_noext[basePathLen] = '\0';
     return m_pathname_noext;
}

const char* RNAStructure::GetInitialFileComment() const {
     return m_fileCommentLine;
}
const char* RNAStructure::GetSuggestedStructureFolderName() {
     
     if(m_suggestedFolderName) { // we have already computed this data:
          return m_suggestedFolderName;
     }
     else {
          m_suggestedFolderName = (char *) malloc((MAX_BUFFER_SIZE + 1) * sizeof(char));
     }
     
     vector<string> searchForStructStrings;
     if(m_fileCommentLine != NULL) { // file we loaded contained a comment:
          searchForStructStrings.push_back(string(m_fileCommentLine));
     }
     const char *fileNamePath = strrchr(GetFilenameNoExtension(), '/');
     fileNamePath = fileNamePath ? fileNamePath : GetFilenameNoExtension();
     searchForStructStrings.push_back(string(fileNamePath));

     // search for patterns of the form <CAPLETTER>.<LOWERCASELETTERS>, e.g., 
     // E.coli or C.elegans, in the file name for a suggested structure name:
     char filePathReplaceChars[][2] = {
          {'_', '-'}, 
     };
     bool foundMatch = false;
     for(int sidx = 0; sidx < searchForStructStrings.size(); sidx++) { 
	strncpy(m_suggestedFolderName, searchForStructStrings[sidx].c_str(), MAX_BUFFER_SIZE + 1);
	m_suggestedFolderName[MAX_BUFFER_SIZE] = '\0';
        for(int rsidx = 0; rsidx < GetArrayLength(filePathReplaceChars); rsidx++) { 
	     StringTranslateCharacters(m_suggestedFolderName, 
			               filePathReplaceChars[rsidx][0], 
				       filePathReplaceChars[rsidx][1]);
	}
	char *dotPos = NULL;
	size_t strOffset = 0, sfnLen = strnlen(m_suggestedFolderName, MAX_BUFFER_SIZE);
	while(*(dotPos = strchrnul(m_suggestedFolderName + strOffset, '.')) != '\0') {
	     strOffset = dotPos - m_suggestedFolderName;
	     if(strOffset > 0 && isupper(m_suggestedFolderName[strOffset - 1])) { 
	          size_t endingLowerPos = strOffset + 1;
		  while((endingLowerPos < sfnLen) && islower(m_suggestedFolderName[endingLowerPos])) {
		       endingLowerPos++;
		  }
		  size_t orgSubnameLen = endingLowerPos - strOffset - 1;
		  if(orgSubnameLen > 0) {
		       char structName[MAX_BUFFER_SIZE + 1];
		       strncpy(structName, &(m_suggestedFolderName[strOffset - 1]), 2);
		       structName[2] = ' ';
		       strncpy(structName + 3, &(m_suggestedFolderName[strOffset + 1]), orgSubnameLen);
		       structName[orgSubnameLen + 3] = '\0';
		       strcpy(m_suggestedFolderName, structName);
		       foundMatch = true;
		       break;
		  }
	     }
        }
	if(foundMatch) { 
             break;
	}
     }
     if(!foundMatch) {
          free(m_suggestedFolderName);
	  m_suggestedFolderName = NULL;
	  return NULL;
     }
     
     size_t sfnLen = strnlen(m_suggestedFolderName, MAX_BUFFER_SIZE);
     if(sfnLen < MAX_BUFFER_SIZE) {
          m_suggestedFolderName = (char *) realloc(m_suggestedFolderName, sfnLen + 1);
	  m_suggestedFolderName[sfnLen] = '\0';
     }
     return m_suggestedFolderName;
}

void RNAStructure::DisplayFileContents(const char *titleSuffix)
{
    if(!m_ctDisplayString || !m_ctDisplayFormatString || 
       !m_seqDisplayString || !m_seqDisplayFormatString) { 
        Free(m_ctDisplayString);
	Free(m_ctDisplayFormatString);
	Free(m_seqDisplayString);
	Free(m_seqDisplayFormatString);
        GenerateString();
    }

    if (!m_contentWindow)
    {
        int subwinWidth = 473, subwinTotalHeight = 675, 
	    subwinResizeSpacing = 24;
	int curXOffset = 6, curYOffset = 6, windowSpacing = 10;
	int labelHeight = 25, btnHeight = 25, btnWidth = 175;
	Fl_Boxtype labelBoxType = FL_ROUND_DOWN_BOX;
	Fl_Boxtype noteBoxType = FL_DOWN_BOX;

	char contentWinTitleString[MAX_BUFFER_SIZE];
	snprintf(contentWinTitleString, MAX_BUFFER_SIZE, "%s%s%s\0", 
	         GetFilenameNoExtension(), titleSuffix == NULL ? "" : " : ", 
		 titleSuffix == NULL ? "" : titleSuffix);
	m_contentWindow = new Fl_Double_Window(subwinWidth, subwinTotalHeight);
	m_contentWindow->copy_label(contentWinTitleString);
	Fl_Box* resizeBox = new Fl_Box(0, curYOffset, subwinWidth, 
			               subwinTotalHeight - subwinResizeSpacing);
	m_contentWindow->resizable(resizeBox);
	m_contentWindow->size_range(subwinWidth, subwinWidth - subwinResizeSpacing);
	subwinWidth -= subwinResizeSpacing;

	m_exportExtFilesBox = new Fl_Box(curXOffset, curYOffset, subwinWidth, 
				         labelHeight, 
				         "   >> Export to External Formats:");
	m_exportExtFilesBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	m_exportExtFilesBox->color(GUI_BGCOLOR);
	m_exportExtFilesBox->labelcolor(GUI_BTEXT_COLOR);
	m_exportExtFilesBox->labelfont(LOCAL_BFFONT);
	m_exportExtFilesBox->labelsize(LOCAL_TEXT_SIZE);
	m_exportExtFilesBox->box(labelBoxType);
	curYOffset += labelHeight + windowSpacing;

	m_exportFASTABtn = new Fl_Button(windowSpacing, curYOffset, 
			                 btnWidth, btnHeight, 
			                 "Export FASTA -- @filesaveas");
	m_exportFASTABtn->user_data((void *) this);
	m_exportFASTABtn->callback(ExportFASTAFileCallback);
	m_exportFASTABtn->labelcolor(GUI_BTEXT_COLOR);
	m_exportDBBtn = new Fl_Button(2 * windowSpacing + btnWidth, curYOffset, 
			              btnWidth, btnHeight, 
                                      "Export DotBracket -- @filesaveas");
	m_exportDBBtn->user_data((void *) this);
	m_exportDBBtn->callback(ExportDotBracketFileCallback);
	m_exportDBBtn->labelcolor(GUI_BTEXT_COLOR);
	curYOffset += btnHeight + windowSpacing;

	m_seqSubwindowBox = new Fl_Box(curXOffset, curYOffset, subwinWidth, 
		                       labelHeight, 
			               "   >> Raw Sequence Data:");
        m_seqSubwindowBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	m_seqSubwindowBox->color(GUI_BGCOLOR);
	m_seqSubwindowBox->labelcolor(GUI_BTEXT_COLOR);
	m_seqSubwindowBox->labelfont(LOCAL_BFFONT);
	m_seqSubwindowBox->labelsize(LOCAL_TEXT_SIZE);
	m_seqSubwindowBox->box(labelBoxType);
	curYOffset += labelHeight + windowSpacing;

	m_seqTextDisplay = new Fl_Text_Display(curXOffset, curYOffset, 
		                               subwinWidth, 135);	
	m_seqTextBuffer = new Fl_Text_Buffer(strlen(m_seqDisplayString));
	m_seqStyleBuffer = new Fl_Text_Buffer(strlen(m_seqDisplayFormatString));
	m_seqTextBuffer->text(m_seqDisplayString);
	m_seqTextDisplay->buffer(m_seqTextBuffer);
	m_seqTextDisplay->textfont(LOCAL_BFFONT);
	m_seqTextDisplay->textsize(LOCAL_TEXT_SIZE);
	m_seqTextDisplay->color(GUI_CTFILEVIEW_COLOR);
	m_seqTextDisplay->textcolor(GUI_TEXT_COLOR);
	m_seqTextDisplay->cursor_style(Fl_Text_Display::CARET_CURSOR);
	m_seqTextDisplay->cursor_color(fl_darker(GUI_WINDOW_BGCOLOR));
	m_seqTextDisplay->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	m_seqStyleBuffer->text(m_seqDisplayFormatString);
	int stableSize = sizeof(TEXT_BUFFER_STYLE_TABLE) / 
		         sizeof(TEXT_BUFFER_STYLE_TABLE[0]);
	m_seqTextDisplay->highlight_data(m_seqStyleBuffer, 
		          TEXT_BUFFER_STYLE_TABLE, stableSize - 1, 'A', 0, 0);
	curYOffset += 135 + windowSpacing;

	m_ctSubwindowBox = new Fl_Box(curXOffset, curYOffset, subwinWidth, 
			              labelHeight, 
				      "   >> CT Style Pairing Data:");
	m_ctSubwindowBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	m_ctSubwindowBox->color(GUI_BGCOLOR);
	m_ctSubwindowBox->labelcolor(GUI_BTEXT_COLOR);
	m_ctSubwindowBox->labelfont(LOCAL_BFFONT);
	m_ctSubwindowBox->labelsize(LOCAL_TEXT_SIZE);
	m_ctSubwindowBox->box(labelBoxType);
	curYOffset += labelHeight + windowSpacing;
                
	m_ctTextDisplay = new Fl_Text_Display(curXOffset, curYOffset, subwinWidth, 300);	
	m_ctTextBuffer = new Fl_Text_Buffer(strlen(m_ctDisplayString));
	m_ctStyleBuffer = new Fl_Text_Buffer(strlen(m_ctDisplayFormatString));
	m_ctTextBuffer->text(m_ctDisplayString);
	m_ctTextDisplay->buffer(m_ctTextBuffer);
	m_ctTextDisplay->textfont(LOCAL_BFFONT);
	m_ctTextDisplay->color(GUI_CTFILEVIEW_COLOR);
	m_ctTextDisplay->textcolor(GUI_TEXT_COLOR);
	m_ctTextDisplay->cursor_style(Fl_Text_Display::CARET_CURSOR);
	m_ctTextDisplay->cursor_color(fl_darker(GUI_WINDOW_BGCOLOR));
	m_ctStyleBuffer->text(m_ctDisplayFormatString);
	m_ctTextDisplay->labelfont(LOCAL_BFFONT);
	m_ctTextDisplay->highlight_data(m_ctStyleBuffer, 
		       TEXT_BUFFER_STYLE_TABLE, stableSize - 1, 'A', 0, 0);
        curYOffset += 300 + windowSpacing;

	int pairNoteSubwinHeight = subwinTotalHeight - subwinResizeSpacing / 2 - curYOffset;
	const char *notationStr = "@line  Note: An asterisk (*) to the left of a sequence\n entry in the CT viewer above denotes that the\n base pair is the first in its pair.     @line";
        m_ctViewerNotationBox = new Fl_Box(curXOffset, curYOffset, subwinWidth, 
			                   pairNoteSubwinHeight, notationStr);
        m_ctViewerNotationBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	m_ctViewerNotationBox->color(GUI_BGCOLOR);
	m_ctViewerNotationBox->labelcolor(GUI_BTEXT_COLOR);
	m_ctViewerNotationBox->labelfont(LOCAL_BFFONT);
	m_ctViewerNotationBox->labelsize(LOCAL_TEXT_SIZE);
	m_ctViewerNotationBox->box(noteBoxType);

    }
    m_contentWindow->user_data((void *) this);
    m_contentWindow->callback(CloseCTViewerContentWindowCallback);
    m_contentWindow->show();

}

void RNAStructure::GenerateString()
{
    /*
	   <id> [ACGU] - [ACGU] <id>
    */
    int size = (m_sequenceLength + 2) * 38;
    m_ctDisplayString = (char*) malloc(sizeof(char) * (size + 1));
    m_ctDisplayFormatString = (char*) malloc(sizeof(char) * (size + 1));
    
    int remainingSize = size;
    int charsWritten = 0;
    char *currentPosn = m_ctDisplayString;
    char *formatPosn = m_ctDisplayFormatString;
    
    // table header labels:
    charsWritten = snprintf(currentPosn, remainingSize, 
		   "   BaseId  |  Pair  (PairId) \n-------------------------------------\n");
    snprintf(formatPosn, remainingSize, "%s\n", 
             Util::GetRepeatedString(TBUFSTYLE_DEFAULT_STRFMT, 
		                     charsWritten).c_str());
    currentPosn += charsWritten;
    formatPosn += charsWritten;
    remainingSize -= charsWritten;
    
    for (int i = 0; i < (int) m_sequenceLength; ++i)
    {
		const char* baseStr = "X";
		switch ((int) m_sequence[i].m_base)
		{
		    case A: baseStr = "A";
			    break;
		    case C: baseStr = "C";
		    	break;
		    case G: baseStr = "G";
		    	break;
		    case U: baseStr = "U";
			    break;
		    case X: baseStr = "X";
			    break;
		    default:
			    break;
		}
		if (m_sequence[i].m_pair == UNPAIRED)
		{
		    charsWritten = snprintf(currentPosn, remainingSize, 
				            "   % 6d  | %s\n", i + 1, baseStr);
		    snprintf(formatPosn, remainingSize, "   AAAAAA  | %c\n", 
		             Util::GetBaseStringFormat(baseStr));

		}
		else
		{
		    RNAStructure::BasePair pairID = m_sequence[i].m_pair;
		    const char* pairStr = "X";
		    switch (m_sequence[pairID].m_base)
		    {
			    case A: 
			        pairStr = "A";
			        break;
			    case C: 
			        pairStr = "C";
			        break;
			    case G: 
			        pairStr = "G";
			        break;
			    case U: 
			        pairStr = "U";
			        break;
		            case X: 
				pairStr = "X";
			        break;
		            default:
			        break;
		    }
		    charsWritten = snprintf(
			    currentPosn,
			    remainingSize,
			    " %c % 6d  | %s - %s  (%d)\n",
			    (i <= pairID) ? '*' : ' ', 
			    i + 1,
			    baseStr,
			    pairStr,
			    pairID + 1);
		    const char *pairMarkerFmt = ((i <= pairID) ? TBUFSTYLE_BPAIR_START_STRFMT : 
				                                 TBUFSTYLE_BPAIR_END_STRFMT);
		    int numDigits = Util::GetNumDigitsBase10(pairID + 1);
		    std::string numFmtStr = Util::GetRepeatedString(
				            pairMarkerFmt, numDigits);
		    snprintf(formatPosn, remainingSize, 
		             " %c AAAAAA  | %c - %c  (%s)\n", 
		             (i <= pairID) ? TBUFSTYLE_BPAIR_END : '*', 
			     Util::GetBaseStringFormat(baseStr), 
			     Util::GetBaseStringFormat(pairStr), 
			     numFmtStr.c_str());    
		}
		remainingSize -= charsWritten;
		currentPosn += charsWritten;
		formatPosn += charsWritten;

    }
    m_ctDisplayString = (char *) realloc(m_ctDisplayString, 
                                 sizeof(char) * (size - remainingSize + 1));
    m_ctDisplayString[size - remainingSize] = '\0';
    m_ctDisplayFormatString = (char *) realloc(m_ctDisplayFormatString, 
                               sizeof(char) * (size - remainingSize + 1));
    m_ctDisplayFormatString[size - remainingSize] = '\0';
    
    m_seqDisplayString = (char *) malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
    size_t seqDSLen = GenerateSequenceString(m_seqDisplayString, 
		                             DEFAULT_BUFFER_SIZE);
    if(seqDSLen + 1 < DEFAULT_BUFFER_SIZE) { 
        m_seqDisplayString = (char *) realloc(m_seqDisplayString, (seqDSLen + 1) * sizeof(char));
	m_seqDisplayString[seqDSLen] = '\0';
    }
    else {
	 m_seqDisplayString[DEFAULT_BUFFER_SIZE - 5] = '\0';
	 strcat(m_seqDisplayString, " ...");
    }
    m_seqDisplayFormatString = (char *) malloc((seqDSLen + 1) * sizeof(char));
    strcpy(m_seqDisplayFormatString, m_seqDisplayString);
    StringToUppercase(m_seqDisplayFormatString);
    StringMapCharacter(m_seqDisplayFormatString, 'A', TBUFSTYLE_SEQPAIR_A);
    StringMapCharacter(m_seqDisplayFormatString, 'C', TBUFSTYLE_SEQPAIR_C);
    StringMapCharacter(m_seqDisplayFormatString, 'G', TBUFSTYLE_SEQPAIR_G);
    StringMapCharacter(m_seqDisplayFormatString, 'U', TBUFSTYLE_SEQPAIR_U);
    StringMapCharacter(m_seqDisplayFormatString, '.', TBUFSTYLE_DEFAULT);

}

size_t RNAStructure::GenerateSequenceString(char *strBuf, size_t maxChars, 
		                            size_t clusterSize) const { 
     if(strBuf == NULL || charSeq == NULL) { 
          return 0;
     }
     size_t numSpaces = 0;
     if(clusterSize > 0) { 
          numSpaces = MAX(0, charSeqSize / clusterSize - 1);
     }
     char *strBufActivePtr = strBuf, *destPos = NULL;
     size_t charsCopied = 0, csize = 0;
     for(int strpos = 0; charsCopied + clusterSize < MIN(charSeqSize, maxChars - 1); strpos += clusterSize) { 
	  strncpy(strBufActivePtr, charSeq + strpos, clusterSize);
	  csize = (strpos / clusterSize <= numSpaces) ? clusterSize : 
		  charSeqSize % clusterSize;
	  if(charsCopied + clusterSize < MIN(charSeqSize, maxChars - 1)) {
	       strBufActivePtr[csize] = ' ';
	       strBufActivePtr += csize + 1;
	       charsCopied += csize + 1;
	  }
	  else {
	       strBufActivePtr[MIN(charSeqSize, maxChars - 1)] = '\0';
	       charsCopied += csize;
	       break;
	  }
     }
     if(charsCopied < MIN(charSeqSize, maxChars - 1)) {
          strBufActivePtr[0] = '\0';
     }
     else {
	  strBufActivePtr[maxChars - 1] = '\0';
     }
     return MAX(0, charsCopied - 1);
}

size_t RNAStructure::GenerateFASTAFormatString(char *strBuf, 
		                               size_t maxChars) const { 
     if(strBuf == NULL || charSeq == NULL) { 
          return 0;
     }
     char commentLine[DEFAULT_BUFFER_SIZE];
     size_t cmtLineLen = snprintf(commentLine, DEFAULT_BUFFER_SIZE, 
		         "> CT File Source: %s\n", 
		         GetFilename());
     if(cmtLineLen + charSeqSize + 1 > maxChars) { 
          return 0;
     }
     strcpy(strBuf, commentLine);
     strcat(strBuf, charSeq);
     strBuf[cmtLineLen + charSeqSize] = '\0';
     return cmtLineLen + charSeqSize;
}

size_t RNAStructure::GenerateDotBracketFormatString(char *strBuf, 
		                                    size_t maxChars) const { 
     if(strBuf == NULL || charSeq == NULL) { 
          return 0;
     }
     char fastaFileStr[maxChars + 1];
     size_t fastaFileSize = GenerateFASTAFormatString(fastaFileStr, maxChars); 
     if(fastaFileSize + charSeqSize + 2 > maxChars) { 
          return 0;
     }
     strcpy(strBuf, fastaFileStr);
     strcat(strBuf, "\n");
     strcat(strBuf, dotFormatCharSeq);
     return fastaFileSize + 1 + charSeqSize;
}

void RNAStructure::ExportFASTAFileCallback(Fl_Widget *btn, void *udata) {
     Fl_Button *btnPtr = (Fl_Button *) btn;
     RNAStructure *rnaStructBaseObj = (RNAStructure *) btnPtr->user_data();
     size_t fdataContentStrMaxLen = 2 * rnaStructBaseObj->charSeqSize + 
	                            DEFAULT_BUFFER_SIZE;
     char fastaData[fdataContentStrMaxLen + 1];
     int fdataLength = rnaStructBaseObj->GenerateFASTAFormatString(
		                         fastaData, fdataContentStrMaxLen);
     char suggestedOutFile[DEFAULT_BUFFER_SIZE];
     snprintf(suggestedOutFile, DEFAULT_BUFFER_SIZE, "%s/%s.fasta\0", 
	      CTFILE_SEARCH_DIRECTORY, 
	      rnaStructBaseObj->GetFilenameNoExtension());
     const char *exportPath = fl_file_chooser("Choose FASTA File Location ...", 
		                              "*.fasta", suggestedOutFile, 0); 
     bool fileWriteStatus = RNAStructure::Util::ExportStringToPlaintextFile(
          exportPath, fastaData, fdataLength);
     if(!fileWriteStatus) { 
          fl_alert("Unable to write output FASTA file \"%s\" to disk! If you have a permissions error saving the file to disk, try saving to a location in your home directory.", 
	           exportPath);
     }
}

void RNAStructure::ExportDotBracketFileCallback(Fl_Widget *btn, void *udata) {
     Fl_Button *btnPtr = (Fl_Button *) btn;
     RNAStructure *rnaStructBaseObj = (RNAStructure *) btnPtr->user_data();
     size_t dbdataContentStrMaxLen = 2 * rnaStructBaseObj->charSeqSize + 
	                             DEFAULT_BUFFER_SIZE;
     char dotData[dbdataContentStrMaxLen + 1];
     int dotDataLength = rnaStructBaseObj->GenerateDotBracketFormatString(
		                           dotData, dbdataContentStrMaxLen);
     char suggestedOutFile[DEFAULT_BUFFER_SIZE];
     snprintf(suggestedOutFile, DEFAULT_BUFFER_SIZE, "%s/%s.dot\0", 
	      CTFILE_SEARCH_DIRECTORY, 
	      rnaStructBaseObj->GetFilenameNoExtension());
     const char *exportPath = fl_file_chooser("Choose DOT File Location ...", 
		                              "*.dot", suggestedOutFile, 0); 
     bool fileWriteStatus = RNAStructure::Util::ExportStringToPlaintextFile(
          exportPath, dotData, dotDataLength);
     if(!fileWriteStatus) { 
          fl_alert("Unable to write output DOT file \"%s\" to disk! If you have a permissions error saving the file to disk, try saving to a location in your home directory.", 
	           exportPath);
     }
}

void RNAStructure::CloseCTViewerContentWindowCallback(Fl_Widget *noWidget, void *udata) {
     Fl_Double_Window *dwin = (Fl_Double_Window *) noWidget;
     RNAStructure *rnaStruct = (RNAStructure *) dwin->user_data();
     rnaStruct->DeleteContentWindow();
}

void RNAStructure::DeleteContentWindow() {
    /*if(m_contentWindow) {
        m_contentWindow->hide();
	while(m_contentWindow->visible()) {
             Fl::wait();
	}
	Delete(m_contentWindow);
    	Delete(m_ctTextDisplay);
	Delete(m_ctTextBuffer);
	Delete(m_seqTextDisplay);
	Delete(m_ctTextBuffer);
        Delete(m_seqStyleBuffer);
	Delete(m_exportExtFilesBox);
	Delete(m_seqSubwindowBox);
	Delete(m_ctSubwindowBox);
	Delete(m_exportFASTABtn);
	Delete(m_exportDBBtn);
	Free(m_ctDisplayString);
	Free(m_ctDisplayFormatString);
	Free(m_seqDisplayString);
	Free(m_seqDisplayFormatString);
    }*/
}

void RNAStructure::HideContentWindowCallback(Fl_Widget *cwin, void *udata) {
     RNAStructure *rnaStruct = (RNAStructure *) cwin->user_data();
     rnaStruct->DeleteContentWindow();
}

char RNAStructure::Util::GetBaseStringFormat(const char *baseStr) {
     if(baseStr && !strcasecmp(baseStr, "A")) 
          return TBUFSTYLE_SEQPAIR_A;
     else if(baseStr && !strcasecmp(baseStr, "C"))
	  return TBUFSTYLE_SEQPAIR_C;
     else if(baseStr && !strcasecmp(baseStr, "G"))
	  return TBUFSTYLE_SEQPAIR_G;
     else if(baseStr && !strcasecmp(baseStr, "U"))
	  return TBUFSTYLE_SEQPAIR_U;
     else 
	  return TBUFSTYLE_DEFAULT;
}

std::string RNAStructure::Util::GetRepeatedString(const char *str, int ntimes) {
     std::string rstr = string(str);
     for(int n = 1; n < ntimes; n++) {
          rstr += string(str);
     }
     return rstr;
}

int RNAStructure::Util::GetNumDigitsBase10(int x) {
     char digitsBuffer[MAX_BUFFER_SIZE];
     int numDigits = snprintf(digitsBuffer, MAX_BUFFER_SIZE - 1, "%d", x);
     return numDigits;
}

bool RNAStructure::Util::ExportStringToPlaintextFile(
		         const char *baseOutPath, 
			 const char *srcData, 
			 int srcDataLength, 
			 const char *fileExtText) { 
     if(baseOutPath == NULL || srcData == NULL || fileExtText == NULL) { 
          return false;
     }
     char outputFilePath[DEFAULT_BUFFER_SIZE];
     if(!strcmp(fileExtText, "")) { 
          strncpy(outputFilePath, baseOutPath, DEFAULT_BUFFER_SIZE);
     }
     else { 
          snprintf(outputFilePath, DEFAULT_BUFFER_SIZE, 
		   "%s.%s\0", baseOutPath, fileExtText);
     }
     FILE *fpOutFile = fopen(outputFilePath, "w+");
     if(fpOutFile == NULL) {
          fprintf(stderr, "ERROR: Opening export file \"%s\" : $s\n", outputFilePath, strerror(errno));
	  return false;
     }
     int bytesWrittenToFile = 0;
     while(bytesWrittenToFile < srcDataLength && !ferror(fpOutFile)) { 
          bytesWrittenToFile += fwrite(srcData + bytesWrittenToFile, sizeof(char), 
			               srcDataLength - bytesWrittenToFile, fpOutFile);
     }
     bool operationStatus = true;
     if(ferror(fpOutFile)) { 
          operationStatus = false;
     }
     fclose(fpOutFile);
     return operationStatus;
}

int RNAStructure::ActionOpenCTFileViewerWindow(int structureFolderIndex, 
		                                int minArcIdx, int maxArcIdx) {
     InputWindow *ctFileSelectionWin = new InputWindow(400, 175, "Select CT File to Highlight ...", 
		                                       "", InputWindow::CTVIEWER_FILE_INPUT, 
						       structureFolderIndex);
     while(ctFileSelectionWin->visible()) {
          Fl::wait();
     }
     if(ctFileSelectionWin->isCanceled() || ctFileSelectionWin->getFileSelectionIndex() < 0) {
          Delete(ctFileSelectionWin);
          return -1;
     }
     ctFileSelectionWin->hide();
     StructureManager *rnaStructManager = RNAStructViz::GetInstance()->GetStructureManager();
     int fileSelectionIndex = ctFileSelectionWin->getFileSelectionIndex();
     int structIndex = rnaStructManager->GetFolderAt(structureFolderIndex)->
	               folderStructs[fileSelectionIndex];
     RNAStructure *rnaStructure = rnaStructManager->GetStructure(structIndex);
     if((rnaStructManager != NULL) && (rnaStructure != NULL)) { 
	  if(minArcIdx <= 0 || maxArcIdx <= 0) { 
	       minArcIdx = 1;
	       maxArcIdx = rnaStructure->m_sequenceLength;
	  }
          char arcIndexDisplaySuffix[MAX_BUFFER_SIZE];
	  snprintf(arcIndexDisplaySuffix, MAX_BUFFER_SIZE, "#%d -- #%d (of %d)\0", 
		   minArcIdx, maxArcIdx, rnaStructure->m_sequenceLength); 
	  rnaStructManager->DisplayFileContents(structIndex, arcIndexDisplaySuffix);
	  return structIndex;
     }
     Delete(ctFileSelectionWin);
     return -1;
}

bool RNAStructure::ScrollOpenCTFileViewerWindow(int structIndex, int pairIndex) {
     RNAStructure *rnaStruct = RNAStructViz::GetInstance()->GetStructureManager()->
	                       GetStructure(structIndex);
     if(rnaStruct == NULL || rnaStruct->m_ctTextDisplay == NULL || 
	rnaStruct->m_contentWindow == NULL) {
          return false;
     }
     else if(pairIndex <= 0 || pairIndex > rnaStruct->GetLength()) {
          return false;
     }
     rnaStruct->m_ctTextDisplay->scroll(pairIndex + 2, 1);
     rnaStruct->m_contentWindow->show();
}
