/*
    A class to hold an RNA structure.
*/

#ifndef RNASTRUCTURE_H
#define RNASTRUCTURE_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <Fl/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <stdio.h>
#include <math.h>

#include <vector>
#include <string>

class RNABranchType_t;

#define MAX(x, y)         (x <= y ? (y) : (x))
#define MIN(x, y)         (x <= y ? (x) : (y))
#define ABS(x)            (x >= 0 ? (x) : -1 * (x))
#define MIN3(x, y, z)     MIN(x, MIN(y, z))

#define DEFAULT_BUFFER_SIZE    2048

class RNAStructure
{
    public:
        enum Base
        {
	        A = 0x1,
        	C = 0x2,
	        G = 0x4,
	        U = 0x8
        };

        class RNABranchType_t *branchType;

        // A value for the pair of unpaired bases.
        static const unsigned int UNPAIRED;

        // Data on a single base
        class BaseData
        {
	        public:
                unsigned int m_index; // The index of this structure
                unsigned int m_pair; // The index of the base it is paired with, or UNPAIRED.
	        Base m_base;	// The base type.

                inline char getBaseChar() const {
                     if(m_base == A)
                         return 'A';
                     else if(m_base == C)
                         return 'C';
                     else if(m_base == G)
                         return 'G';
                     else if(m_base == U)
                         return 'U';
                     else
                         return 'X';
                }

            /*
                Determines whether this structure is logically contained within
                a parent structure where containment is defined by enclosure of
                arcs on the diagram circle.
                @see RNABranchIdentification.h
            */
            inline bool isContainedIn(const RNAStructure::BaseData &parentStruct) const {
                unsigned int localIndexMin = MIN(m_index, m_pair);
                unsigned int localIndexMax = MAX(m_index, m_pair);
                unsigned int parentIndexMin = MIN(parentStruct.m_index, parentStruct.m_pair);
                unsigned int parentIndexMax = MAX(parentStruct.m_index, parentStruct.m_pair);
                if(parentStruct.m_pair == UNPAIRED)
                    return false;
                else if(localIndexMax > parentIndexMax || localIndexMin < parentIndexMin)
                    return false;
                else if(localIndexMin >= parentIndexMin && localIndexMax <= parentIndexMax)
                    return true;
                return false;
            }
        
            /*
            For use with the routines in BranchTypeIdentification.*:
            */
            inline unsigned int getPairDistance() {
                return ABS(MAX(m_pair, m_index) - MIN(m_pair, m_index));
            }

        };

        /*
	    Creation method, designed to allow error handling during construction.
	    There is one version for each file type.
	    Returned pointer is owned by the caller, which must call delete on the pointer to remove it.
	    0 is returned if there is an error reading the file.
        */
        static RNAStructure* CreateFromFile(const char* filename, const bool isBPSEQ);

        /*
	    Destructor.
        */
        ~RNAStructure();

        /*
	    Return the base at a given location. Indexed starting at 0.
        */
        BaseData* GetBaseAt(unsigned int position);
        RNABranchType_t* GetBranchTypeAt(unsigned int position);

        /*
	    Return the number of bases in the sequence.
        */
        inline unsigned int GetLength() const
        {
        	return m_sequenceLength;
        }

        /*
	    Get the complete path name of the file associated with this structure.
        */
        inline const char* GetPathname() const
        {
	        return m_pathname;
        }

        inline const char* GetCharSeq() const
        {
            return charSeq;
        }

        inline unsigned int GetCharSeqSize() const
        {
            return charSeqSize;
        }

        /*
	    Get the file name for this sequence.
        */
        const char* GetFilename() const;
        const char* GetFilenameNoExtension();

        /*
	     Display the contents of the file in a window (or bring it to the top if already existing).
        */
        void DisplayFileContents();

        /*
         Returns a reference to the local m_sequence BaseData* pointer.
        */
        inline BaseData* & getSequence() {
            return m_sequence;
        }

        inline void print() {
            fprintf(stderr, "Printing RNAStructure of length = %u\n", GetLength());
            for(int i = 0; i < GetLength(); i++) {
                 BaseData *bdp = GetBaseAt(i);
                 fprintf(stderr, "   => %u -> %u [%s]\n", bdp->m_index, bdp->m_pair, (bdp->m_pair == UNPAIRED) ? "UNPAIRED" : "PAIRED");
            }
        }

    private:
        /*
	     Constructor is private to force use of Create methods.
        */
        RNAStructure();
        RNAStructure(const RNAStructure &rnaStruct);
        RNAStructure & operator=(const RNAStructure &rhs);

        void copyRNAStructure(const RNAStructure &rnaStruct);

        /*
	     Generate the string used for text display of the structure.
        */
        void GenerateString();
	size_t GenerateSequenceString(char *strBuf, size_t maxChars, 
			              size_t clusterSize = 8) const; 
	size_t GenerateFASTAFormatString(char *strBuf, size_t maxChars) const;
	size_t GenerateDotBracketFormatString(char *strBuf, size_t maxChars) const; 

	/* Callbacks for the export buttons: */
        static void ExportFASTAFileCallback(Fl_Widget *btn, void *udata);
	static void ExportDotBracketFileCallback(Fl_Widget *btn, void *udata);

        // The structure data
        unsigned int m_sequenceLength;
        BaseData* m_sequence;

        // The full path name of the file from which this sequence came.
        char *m_pathname, *m_pathname_noext;

        // Info for displaying the file contents
        Fl_Double_Window *m_contentWindow;
        Fl_Text_Display *m_ctTextDisplay, *m_seqTextDisplay;
        Fl_Text_Buffer *m_ctTextBuffer, *m_ctStyleBuffer;
	Fl_Text_Buffer *m_seqTextBuffer, *m_seqStyleBuffer;
	Fl_Box *m_exportExtFilesBox, *m_seqSubwindowBox, *m_ctSubwindowBox;
	Fl_Button *m_exportFASTABtn, *m_exportDBBtn;

        char *m_ctDisplayString, *m_seqDisplayString;
        char *m_ctDisplayFormatString, *m_seqDisplayFormatString;
	char *charSeq, *dotFormatCharSeq;
        unsigned int charSeqSize;

    public:
	class Util { 
	     public:
	          static char GetBaseStringFormat(const char *baseStr);
	          static std::string GetRepeatedString(const char *str, int ntimes);
                  static int GetNumDigitsBase10(int x);

		  static bool ExportStringToPlaintextFile(
		       const char *baseOutPath, 
		       const char *srcData, 
		       size_t srcDataLength, 
		       const char *fileExtText = ""
		  );
	};

};

#endif //RNASTRUCTURE_H
