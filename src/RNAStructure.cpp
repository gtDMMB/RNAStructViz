#include "RNAStructure.h"
#include "BranchTypeIdentification.h"
#include "ConfigOptions.h"
#include <FL/Fl_Box.H>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <vector>

const unsigned int RNAStructure::UNPAIRED = ~0x0;

const Fl_Text_Display::Style_Table_Entry 
      RNAStructure::textBufferStyleTable[] = {
     {GUI_TEXT_COLOR,   FL_SCREEN_BOLD,        12}, // A -- default
     {FL_GREEN,         FL_SCREEN_BOLD,        12}, // B -- pair A
     {FL_MAGENTA,       FL_SCREEN_BOLD,        12}, // C -- pair C
     {FL_YELLOW,        FL_SCREEN_BOLD,        12}, // D -- pair G
     {FL_RED,           FL_SCREEN_BOLD,        12}, // E -- pair U
     {FL_CYAN,          FL_SCREEN_BOLD,        12}, // F -- first pairing
     {FL_BLUE,          FL_SCREEN_BOLD,        12}, // G -- second pairing
     {0,                0,                     0},  // NULL end of array
};

RNAStructure::RNAStructure()
    : m_sequenceLength(0), m_sequence(0), 
      m_displayString(0), m_displayFormatString(NULL) 
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
    free(m_displayString); 
    free(m_sequence); 
    free((void*)charSeq); 
    if (m_pathname != NULL) {
    	free(m_pathname);
    }
    if(branchType != NULL) {
         delete branchType;
         branchType = NULL;
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
    result->charSeq = (char*) malloc(sizeof(char) *result->charSeqSize);
    for(unsigned i = 0; i < tempSeq.size(); i++)
    {
        result->charSeq[i] = tempSeq.at(i);
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

void RNAStructure::DisplayFileContents()
{
    if (m_displayString && m_displayFormatString) {
        free(m_displayString);
	m_displayString = NULL;
	free(m_displayFormatString);
	m_displayFormatString = NULL;
    }
    GenerateString();
    if(m_contentWindow) {
        delete m_contentWindow;
	m_contentWindow = NULL;
    }

    if (!m_contentWindow)
    {
		m_contentWindow = new Fl_Double_Window(275, 600, GetFilename());
		Fl_Box* resizeBox = new Fl_Box(0, 0, 275, 600);
		m_contentWindow->resizable(resizeBox);
		m_contentWindow->size_range(275, 300);

		m_textDisplay = new Fl_Text_Display(0, 0, 275, 600);	
		Fl_Text_Buffer* m_textBuffer = 
			new Fl_Text_Buffer(strlen(m_displayString));
		Fl_Text_Buffer *m_styleBuffer = 
			new Fl_Text_Buffer(strlen(m_displayFormatString));
		m_textBuffer->text(m_displayString);
		m_textDisplay->buffer(m_textBuffer);
		m_textDisplay->textfont(LOCAL_BFFONT);
		m_textDisplay->color(GUI_WINDOW_BGCOLOR);
		m_textDisplay->textcolor(GUI_BTEXT_COLOR);
		m_textDisplay->cursor_style(Fl_Text_Display::CARET_CURSOR);
		m_textDisplay->cursor_color(fl_darker(GUI_WINDOW_BGCOLOR));
		m_styleBuffer->text(m_displayFormatString);
		int stableSize = sizeof(textBufferStyleTable) / 
			         sizeof(textBufferStyleTable[0]);
		m_textDisplay->highlight_data(m_styleBuffer, 
			       textBufferStyleTable, stableSize - 1, 'A', 0, 0);

    }
    m_contentWindow->show();

}

void RNAStructure::GenerateString()
{
    /*
	   <id> [ACGU] - [ACGU] <id>
    */
    int size = (m_sequenceLength + 2) * 27;
    m_displayString = (char*)malloc(sizeof(char) * size + 1);
    m_displayFormatString = (char*)malloc(sizeof(char) * size + 1);
    
    int remainingSize = size;
    int charsWritten = 0;
    char *currentPosn = m_displayString;
    char *formatPosn = m_displayFormatString;
    
    // table header labels:
    charsWritten = snprintf(currentPosn, remainingSize, 
		   "BaseIdx |  Pair  (PairIdx)\n--------------------------\n");
    snprintf(formatPosn, remainingSize, "%s\n", 
             GetRepeatedString("A", charsWritten + 1).c_str());
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
				            "%6d  | %s\n", i + 1, baseStr);
		    snprintf(formatPosn, remainingSize, "AAAAAA  | %c\n", 
		             GetBaseStringFormat(baseStr));

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
			    "%6d  | %s - %s  (%d)\n",
			    i + 1,
			    baseStr,
			    pairStr,
			    pairID + 1);
		    const char *pairMarkerFmt = ((i <= pairID) ? "F" : "G");
		    int numDigits = GetNumDigitsBase10(pairID + 1);
		    std::string numFmtStr = GetRepeatedString(pairMarkerFmt, 
				                        numDigits);
		    snprintf(formatPosn, remainingSize, 
		             "AAAAAA  | %c - %c  (%s)\n", 
		             GetBaseStringFormat(baseStr), 
			     GetBaseStringFormat(pairStr), 
			     numFmtStr.c_str());    
		}
		remainingSize -= charsWritten;
		currentPosn += charsWritten;
		formatPosn += charsWritten;

    }
    m_displayString = (char *) realloc(m_displayString, 
                               sizeof(char) * (size - remainingSize+1));
    m_displayString[size - remainingSize] = '\0';
    m_displayFormatString = (char *) realloc(m_displayFormatString, 
                             sizeof(char) * (size - remainingSize+1));
    m_displayFormatString[size - remainingSize] = '\0';

}

char RNAStructure::GetBaseStringFormat(const char *baseStr) {
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

std::string RNAStructure::GetRepeatedString(const char *str, int ntimes) {
     std::string rstr = string(str);
     for(int n = 1; n < ntimes; n++) {
          rstr += string(str);
     }
     return rstr;
}

int RNAStructure::GetNumDigitsBase10(int x) {
     char digitsBuffer[MAX_BUFFER_SIZE];
     int numDigits = snprintf(digitsBuffer, MAX_BUFFER_SIZE - 1, "%d", x);
     return numDigits;
}
