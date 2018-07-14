#include "RNAStructure.h"
#include "BranchTypeIdentification.h"
#include <stdlib.h>
#include <fstream>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <string.h>
#include <vector>

const unsigned int RNAStructure::UNPAIRED = ~0x0;

RNAStructure::RNAStructure()
    : m_sequenceLength(0)
    , m_sequence(0)
    , m_contentWindow(0)
    , m_displayString(0)
{
     branchType = NULL; //new RNABranchType_t(BRANCH_UNDEFINED, NULL);
}

RNAStructure::~RNAStructure()
{

    if (m_contentWindow)
    {
		Fl::delete_widget(m_contentWindow);
    }

    free(m_displayString); //m_displayString = NULL;

    free(m_sequence); //m_sequence = NULL;
    
    free((void*)charSeq); //charSeq = NULL;
    
    if (m_pathname != NULL) {
    	free(m_pathname);
    	//m_pathname = NULL;
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
		return m_sequence + position;
    }
    fprintf(stderr, "in GetBaseAt: m_sequenceLength=%d, position=%d\n", m_sequenceLength, position);
    return NULL;
}

RNABranchType_t & RNAStructure::GetBranchTypeAt(unsigned int position)
{
    if (position < m_sequenceLength)
    {
		return *(branchType + position);
    }
    RNABranchType_t NULL_BranchType(BRANCH_UNDEFINED, NULL);
    return NULL_BranchType;
}

RNAStructure* RNAStructure::CreateFromFile(const char* filename, 
	const bool isBPSEQ)
{
    std::ifstream inStream(filename);
    if (!inStream.good())
    {
		if (strlen(filename) > 1000)
		{
		    fl_message("Unable to open file: <file name too long>");
		}
		else
		{
		    fl_message("Unable to open file: %s", filename);
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
			    fl_message("Bad base: id %d, <file name too long>", 
			    	result->m_sequenceLength + 1);
			}
			else
			{
			    fl_message("Bad base: id %d, file %s", 
			    	result->m_sequenceLength + 1, filename);
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
				    fl_message("Bad prev id: id %d, <file name too long>", 
				    	result->m_sequenceLength + 1);
				}
				else
				{
				    fl_message("Bad prev id: id %d, file %s", 
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
				    fl_message("Bad next id: id %d, <file name too long>", 
				    	result->m_sequenceLength + 1);
				}
				else
				{
				    fl_message("Bad next id: id %d, file %s", 
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
				fl_message("Bad pair: id %d, <file name too long>", 
					result->m_sequenceLength + 1);
			}
		    else
		    {
				fl_message("Bad pair: id %d, file %s", 
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
				    fl_message("Bad trailing id: id %d, <file name too long>", 
				    	result->m_sequenceLength + 1);
				}
				else
				{
				    fl_message("Bad trailing id: id %d, file %s", 
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
		    fl_message("Empty or malformed file: <file name too long>");
		else
		    fl_message("Empty or malformed file: %s", filename);
		delete result;
		return 0;
    }

    result->m_sequence = (BaseData*)realloc(result->m_sequence, 
                                            sizeof(BaseData)*result->m_sequenceLength);
    
    result->branchType = (RNABranchType_t*) malloc(sizeof(RNABranchType_t) * result->m_sequenceLength);
    result->m_pathname = strdup(filename);
    
    result->charSeqSize = tempSeq.size();
    result->charSeq = (char*)malloc(sizeof(char)*result->charSeqSize);

    for(unsigned i = 0; i < tempSeq.size(); i++)
    {

        result->charSeq[i] = tempSeq.at(i);
    }

    RNABranchType_t::PerformBranchClassification(result, result->m_sequenceLength);

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
    if (!m_displayString)
	GenerateString();

    if (!m_contentWindow)
    {
		m_contentWindow = new Fl_Double_Window(220, 600, GetFilename());
		Fl_Box* resizeBox = new Fl_Box(0, 0, 220, 600);
		m_contentWindow->resizable(resizeBox);
		m_contentWindow->size_range(220, 300);
	
		m_textDisplay = new Fl_Text_Display(0, 0, 220, 600);
	
		Fl_Text_Buffer* textBuffer = 
			new Fl_Text_Buffer(strlen(m_displayString));
		textBuffer->text(m_displayString);
		m_textDisplay->buffer(textBuffer);
		m_textDisplay->textfont(FL_COURIER);

    }

    m_contentWindow->show();
}

void RNAStructure::GenerateString()
{
    if (m_displayString)
    {
		free(m_displayString); //m_displayString = NULL;
    }
    
    /*
	<id> [ACGU] - [ACGU] <id>
    */
    int size = m_sequenceLength * 22;
    m_displayString = (char*)malloc(sizeof(char) * size + 1);

    int remainingSize = size;
    int charsWritten = 0;
    char* currentPosn = m_displayString;
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
		    charsWritten = snprintf(currentPosn, remainingSize, "%6d  %s\n", 
		    	i + 1, baseStr);
		}
		else
		{
		    int pairID = (int)m_sequence[i].m_pair;
		    const char* pairStr = 0;
		    switch (m_sequence[pairID].m_base)
		    {
			case A: pairStr = "A";
			    break;
			case C: pairStr = "C";
			    break;
			case G: pairStr = "G";
			    break;
			case U: pairStr = "U";
			    break;
		    }
		    charsWritten = snprintf(
			currentPosn,
			remainingSize,
			"%6d  %s - %s  %d\n",
			i + 1,
			baseStr,
			pairStr,
			pairID + 1);
		}
	
		remainingSize -= charsWritten;
		currentPosn += charsWritten;
    }
	
    m_displayString = (char*)realloc(m_displayString, 
                                     sizeof(char) * (size - remainingSize+1));
}

