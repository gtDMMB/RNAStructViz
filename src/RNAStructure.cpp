#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <vector>

#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>

#include "RNAStructure.h"
#include "BranchTypeIdentification.h"
#include "ConfigOptions.h"
#include "ThemesConfig.h"

const unsigned int RNAStructure::UNPAIRED = ~0x0;

RNAStructure::RNAStructure()
    : m_sequenceLength(0), m_sequence(NULL), 
      charSeq(NULL), dotFormatCharSeq(NULL), charSeqSize(0), 
      m_pathname(NULL), m_pathname_noext(NULL), 
      m_ctDisplayString(NULL), m_ctDisplayFormatString(NULL), 
      m_seqDisplayString(NULL), m_seqDisplayFormatString(NULL), 
      m_ctTextDisplay(NULL), m_ctStyleBuffer(NULL), 
      m_seqTextDisplay(NULL), m_seqStyleBuffer(NULL), 
      m_exportExtFilesBox(NULL), m_seqSubwindowBox(NULL), 
      m_ctSubwindowBox(NULL), m_ctViewerNotationBox(NULL), 
      m_exportFASTABtn(NULL), m_exportDBBtn(NULL) 	
{
     branchType = NULL; 
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
    if(branchType != NULL) {
         delete branchType;
         branchType = NULL;
    }
    if(m_contentWindow) { 
	Delete(m_contentWindow); m_contentWindow = NULL;
    	Delete(m_ctTextDisplay); m_ctTextDisplay = NULL;
	Delete(m_seqTextDisplay); m_seqTextDisplay = NULL;
	Delete(m_ctTextBuffer); m_ctTextBuffer = NULL;
	Delete(m_ctStyleBuffer); m_ctStyleBuffer = NULL;
	Delete(m_seqTextDisplay); m_seqTextDisplay = NULL;
        Delete(m_seqStyleBuffer); m_seqStyleBuffer = NULL;
	Delete(m_exportExtFilesBox); m_exportExtFilesBox = NULL;
	Delete(m_seqSubwindowBox); m_seqSubwindowBox = NULL;
	Delete(m_ctSubwindowBox); m_ctSubwindowBox = NULL;
	Delete(m_ctViewerNotationBox); m_ctViewerNotationBox = NULL;
	Delete(m_exportFASTABtn); m_exportFASTABtn = NULL;
	Delete(m_exportDBBtn); m_exportDBBtn = NULL;
    }
}

RNAStructure::BaseData* RNAStructure::GetBaseAt(unsigned int position)
{
    if (position < m_sequenceLength)
    {
        return &m_sequence[position];
    }
    return NULL;
}

RNABranchType_t* RNAStructure::GetBranchTypeAt(unsigned int position)
{
    if (position < m_sequenceLength)
    {
        return &branchType[position];
    }
    return NULL;
}

RNAStructure* RNAStructure::CreateFromFile(const char* filename, 
	const bool isBPSEQ)
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
    unsigned int maxSize = 1024;
    result->m_sequence = (BaseData*)malloc(sizeof(BaseData) * maxSize);
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
		
		    // Ignore this line
		    while (!inStream.eof() && inStream.get() != '\n');
		    {
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
		    maxSize += 1024;
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

    result->m_sequence = (BaseData*)realloc(result->m_sequence, 
                                            sizeof(BaseData)*result->m_sequenceLength);
    if(PERFORM_BRANCH_TYPE_ID) {
        result->branchType = (RNABranchType_t*) malloc( 
		sizeof(RNABranchType_t) * result->m_sequenceLength);
    }
    result->m_pathname = strdup(filename);
    result->charSeqSize = tempSeq.size();
    result->charSeq = (char *) malloc(sizeof(char) * result->charSeqSize);
    result->dotFormatCharSeq = (char *) 
	    malloc(sizeof(char) * result->charSeqSize);
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
    if(PERFORM_BRANCH_TYPE_ID) {
        RNABranchType_t::PerformBranchClassification(result, 
			 result->m_sequenceLength);
    }

    return result;
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
          m_pathname_noext = fullFilename;
          return m_pathname_noext;
     }
     size_t basePathLen = dotPosPtr - fullFilename;
     m_pathname_noext = (char *) malloc((basePathLen + 1) * sizeof(char));
     strncpy(m_pathname_noext, fullFilename, basePathLen);
     m_pathname_noext[basePathLen] = '\0';
     return m_pathname_noext;
}

void RNAStructure::DisplayFileContents()
{
    if (m_ctDisplayString && m_ctDisplayFormatString && 
        m_seqDisplayString && m_seqDisplayFormatString) {
        free(m_ctDisplayString); m_ctDisplayString = NULL;
	free(m_ctDisplayFormatString); m_ctDisplayFormatString = NULL;
        free(m_seqDisplayString); m_seqDisplayString = NULL;
	free(m_seqDisplayFormatString); m_seqDisplayFormatString = NULL;
    }
    GenerateString();
    
    if(m_contentWindow) {
        delete m_contentWindow; m_contentWindow = NULL;
    	delete m_ctTextDisplay; m_ctTextDisplay = NULL;
	delete m_seqTextDisplay; m_seqTextDisplay = NULL;
	delete m_ctTextBuffer; m_ctTextBuffer = NULL;
	delete m_ctStyleBuffer; m_ctStyleBuffer = NULL;
	delete m_seqTextDisplay; m_seqTextDisplay = NULL;
        delete m_seqStyleBuffer; m_seqStyleBuffer = NULL;
	delete m_exportExtFilesBox; m_exportExtFilesBox = NULL;
	delete m_seqSubwindowBox; m_seqSubwindowBox = NULL;
	delete m_ctSubwindowBox; m_ctSubwindowBox = NULL;
	delete m_exportFASTABtn; m_exportFASTABtn = NULL;
	delete m_exportDBBtn; m_exportDBBtn = NULL;
    }

    if (!m_contentWindow)
    {
        int subwinWidth = 325, subwinTotalHeight = 700, 
	    subwinResizeSpacing = 24;
	int curXOffset = 6, curYOffset = 6, windowSpacing = 10;
	int labelHeight = 25, btnHeight = 25, btnWidth = 145;
	Fl_Boxtype labelBoxType = FL_ROUND_DOWN_BOX;
	Fl_Boxtype noteBoxType = FL_DOWN_BOX;

	m_contentWindow = new Fl_Double_Window(subwinWidth, 
			      subwinTotalHeight, GetFilename());
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
	m_seqStyleBuffer = new Fl_Text_Buffer(strlen(m_seqDisplayString));
	m_seqTextBuffer->text(m_seqDisplayString);
	m_seqTextDisplay->buffer(m_seqTextBuffer);
	m_seqTextDisplay->textfont(LOCAL_BFFONT);
	m_seqTextDisplay->color(GUI_CTFILEVIEW_COLOR);
	m_seqTextDisplay->textcolor(GUI_TEXT_COLOR);
	m_seqTextDisplay->cursor_style(Fl_Text_Display::CARET_CURSOR);
	m_seqTextDisplay->cursor_color(fl_darker(GUI_WINDOW_BGCOLOR));
	m_seqTextDisplay->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	m_seqStyleBuffer->text(m_seqDisplayString);
	int stableSize = sizeof(TEXT_BUFFER_STYLE_TABLE) / 
		         sizeof(TEXT_BUFFER_STYLE_TABLE[0]);
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
	m_ctTextDisplay->highlight_data(m_ctStyleBuffer, 
		       TEXT_BUFFER_STYLE_TABLE, stableSize - 1, 'A', 0, 0);
        curYOffset += 300 + windowSpacing;

	int pairNoteSubwinHeight = subwinTotalHeight - subwinResizeSpacing / 2 - curYOffset;
	const char *notationStr = "Note: An asterisk (*) to the left of a sequence\nentry in the CT viewer above denotes that the\nentry is the first in its pair. The pair\nindices to the right of the paired sequence\nentries are also color coded in shades of blue\nto denote this distinction.";
        m_ctViewerNotationBox = new Fl_Box(curXOffset, curYOffset, subwinWidth, 
			                   pairNoteSubwinHeight, notationStr);
        m_ctViewerNotationBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	m_ctViewerNotationBox->color(GUI_BGCOLOR);
	m_ctViewerNotationBox->labelcolor(GUI_BTEXT_COLOR);
	m_ctViewerNotationBox->labelfont(LOCAL_BFFONT);
	m_ctViewerNotationBox->labelsize(LOCAL_TEXT_SIZE / 2);
	m_ctViewerNotationBox->box(noteBoxType);
	//m_ctViewerNotationBox->measure_label(subwinWidth, pairNoteSubwinHeight);

    }
    m_contentWindow->show();

}

void RNAStructure::GenerateString()
{
    /*
	   <id> [ACGU] - [ACGU] <id>
    */
    int size = (m_sequenceLength + 2) * 30;
    m_ctDisplayString = (char*)malloc(sizeof(char) * (size + 1));
    m_ctDisplayFormatString = (char*)malloc(sizeof(char) * (size + 1));
    
    int remainingSize = size;
    int charsWritten = 0;
    char *currentPosn = m_ctDisplayString;
    char *formatPosn = m_ctDisplayFormatString;
    
    // table header labels:
    charsWritten = snprintf(currentPosn, remainingSize, 
		   "   BaseIdx |  Pair  (PairIdx)\n-----------------------------\n");
    snprintf(formatPosn, remainingSize, "%s\n", 
             Util::GetRepeatedString("A", charsWritten + 1).c_str());
    formatPosn[25] = '\n';
    currentPosn += charsWritten;
    formatPosn += charsWritten;
    remainingSize -= charsWritten;
    
    for (int i = 0; i < (int)m_sequenceLength; ++i)
    {
		const char* baseStr = 0;
		switch (m_sequence[i].m_base)
		{
		    case A: baseStr = "A";
			    break;
		    case C: baseStr = "C";
		    	break;
		    case G: baseStr = "G";
		    	break;
		    case U: baseStr = "U";
			    break;
		}
		if (m_sequence[i].m_pair == UNPAIRED)
		{
		    charsWritten = snprintf(currentPosn, remainingSize, 
				            "   %6d  | %s\n", i + 1, baseStr);
		    snprintf(formatPosn, remainingSize, "   AAAAAA  | %c\n", 
		             Util::GetBaseStringFormat(baseStr));

		}
		else
		{
		    int pairID = (int)m_sequence[i].m_pair;
		    const char* pairStr = 0;
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
		    }
		    charsWritten = snprintf(
			    currentPosn,
			    remainingSize,
			    " %c %6d  | %s - %s  (%d)\n",
			    (i <= pairID) ? '*' : ' ', 
			    i + 1,
			    baseStr,
			    pairStr,
			    pairID + 1);
		    const char *pairMarkerFmt = ((i <= pairID) ? "F" : "G");
		    int numDigits = Util::GetNumDigitsBase10(pairID + 1);
		    std::string numFmtStr = Util::GetRepeatedString(
				            pairMarkerFmt, numDigits);
		    snprintf(formatPosn, remainingSize, 
		             "   AAAAAA  | %c - %c  (%s)\n", 
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
    m_seqDisplayFormatString = (char *) malloc((seqDSLen + 1) * sizeof(char));
    strcpy(m_seqDisplayFormatString, m_seqDisplayFormatString);

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
     if(charSeqSize + numSpaces + 1 > maxChars) { 
          return 0;
     }
     char *strBufActivePtr = strBuf, *destPos = NULL;
     size_t charsCopied = 0, csize = 0;
     for(int strpos = 0; strpos < charSeqSize; strpos += clusterSize) { 
          strncpy(strBufActivePtr, charSeq + strpos, clusterSize);
	  csize = (strpos / clusterSize <= numSpaces) ? clusterSize : 
		  charSeqSize % clusterSize;
	  strBufActivePtr[csize] = ' ';
	  strBufActivePtr += csize + 1;
	  charsCopied += csize + 1;
     }
     strBufActivePtr--;
     strBufActivePtr[0] = '\0';
     return MAX(0, charsCopied - 1);
}

size_t RNAStructure::GenerateFASTAFormatString(char *strBuf, 
		                               size_t maxChars) const { 
     if(strBuf == NULL || charSeq == NULL) { 
          return 0;
     }
     char commentLine[DEFAULT_BUFFER_SIZE];
     size_t cmtLineLen = snprintf(commentLine, DEFAULT_BUFFER_SIZE, 
		         "> CT File Source: %s\n\0", 
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
     strncpy(strBuf, fastaFileStr, fastaFileSize);
     strcat(strBuf, "\n");
     strncat(strBuf, dotFormatCharSeq, charSeqSize);
     strBuf[fastaFileSize + charSeqSize] = '\0';
     return fastaFileSize + charSeqSize;
}

void RNAStructure::ExportFASTAFileCallback(Fl_Widget *btn, void *udata) {
     Fl_Button *btnPtr = (Fl_Button *) btn;
     RNAStructure *rnaStructBaseObj = (RNAStructure *) btnPtr->user_data();
     size_t fdataContentStrMaxLen = 2 * rnaStructBaseObj->charSeqSize + 
	                            DEFAULT_BUFFER_SIZE;
     char fastaData[fdataContentStrMaxLen + 1];
     size_t fdataLength = rnaStructBaseObj->GenerateFASTAFormatString(
		                            fastaData, fdataContentStrMaxLen);
     char suggestedOutFile[DEFAULT_BUFFER_SIZE];
     snprintf(suggestedOutFile, DEFAULT_BUFFER_SIZE, "%s%s.fasta\0", 
	      CTFILE_SEARCH_DIRECTORY, 
	      rnaStructBaseObj->GetFilenameNoExtension());
     const char *exportPath = fl_file_chooser("Choose FASTA File Location ...", 
		                              "*.fasta", suggestedOutFile, 0); 
     bool fileWriteStatus = RNAStructure::Util::ExportStringToPlaintextFile(
          exportPath, fastaData, fdataLength);
     if(!fileWriteStatus) { 
          fl_alert("Unable to write output FASTA file \"%s\" to disk!", 
	           exportPath);
     }
}

void RNAStructure::ExportDotBracketFileCallback(Fl_Widget *btn, void *udata) {
     Fl_Button *btnPtr = (Fl_Button *) btn;
     RNAStructure *rnaStructBaseObj = (RNAStructure *) btnPtr->user_data();
     size_t dbdataContentStrMaxLen = 2 * rnaStructBaseObj->charSeqSize + 
	                             DEFAULT_BUFFER_SIZE;
     char dotData[dbdataContentStrMaxLen + 1];
     size_t dotDataLength = rnaStructBaseObj->GenerateDotBracketFormatString(
		                              dotData, dbdataContentStrMaxLen);
     char suggestedOutFile[DEFAULT_BUFFER_SIZE];
     snprintf(suggestedOutFile, DEFAULT_BUFFER_SIZE, "%s%s.dot\0", 
	      CTFILE_SEARCH_DIRECTORY, 
	      rnaStructBaseObj->GetFilenameNoExtension());
     const char *exportPath = fl_file_chooser("Choose DOT File Location ...", 
		                              "*.dot", suggestedOutFile, 0); 
     bool fileWriteStatus = RNAStructure::Util::ExportStringToPlaintextFile(
          exportPath, dotData, dotDataLength);
     if(!fileWriteStatus) { 
          fl_alert("Unable to write output DOT file \"%s\" to disk!", 
	           exportPath);
     }
}

char RNAStructure::Util::GetBaseStringFormat(const char *baseStr) {
     if(!strcasecmp(baseStr, "A")) 
          return 'B';
     else if(!strcmp(baseStr, "C"))
	  return 'C';
     else if(!strcasecmp(baseStr, "G"))
	  return 'D';
     else if(!strcasecmp(baseStr, "U"))
	  return 'E';
     else 
	  return 'A';
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
			 size_t srcDataLength, 
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
     FILE *fpOutFile = fopen(outputFilePath, "w");
     size_t bytesWrittenToFile = 0;
     while(bytesWrittenToFile < srcDataLength && !ferror(fpOutFile)) { 
          bytesWrittenToFile += fwrite(srcData, sizeof(char), srcDataLength, 
			               fpOutFile);
     }
     bool operationStatus = true;
     if(ferror(fpOutFile)) { 
          operationStatus = false;
     }
     fclose(fpOutFile);
     return operationStatus;
}


