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

#include "ConfigOptions.h"
#include "BaseSequenceIDs.h"

class RNABranchType_t;

#define MIN3(x, y, z)                MIN(x, MIN(y, z))
#define ABS(x)                       (x >= 0 ? (x) : -1 * (x))

#define RadiansToDegrees(theta)      (theta * 180.0 / M_PI)
#define Square(x)                    ((x) * (x))

#define DEFAULT_BUFFER_SIZE          (384)

class RNAStructure
{
    public:
        enum Base {
		X = 0x00, 
	        A = 0x01,
        	C = 0x02,
	        G = 0x04,
	        U = 0x08
        } __attribute__ ((__packed__));
        
	typedef uint16_t BasePair;

        #if PERFORM_BRANCH_TYPE_ID
        class RNABranchType_t *branchType;
        #endif

        // A value for the pair of unpaired bases.
        static const BasePair UNPAIRED;

        // Data on a single base
        #pragma pack(push, 1)
        class BaseData
        {
	        public:
                BasePair m_index; // The index of this structure
                BasePair m_pair; // The index of the base it is paired with, or UNPAIRED.
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

        }; //__attribute__ ((__packed__));
        #pragma pack(pop)

        /*
	    Creation method, designed to allow error handling during construction.
	    There is one version for each file type.
	    Returned pointer is owned by the caller, which must call delete on the pointer to remove it.
	    0 is returned if there is an error reading the file.
        */
        static RNAStructure* CreateFromFile(const char* filename, const bool isBPSEQ);
        static RNAStructure* CreateFromDotBracketFile(const char *filename); 
	
        #define RNASTRUCT_ARRAY_SIZE        (16)
	static RNAStructure** CreateFromGTBoltzmannFormatFile(const char *filename, int *arrayCount);
	static RNAStructure** CreateFromHelixTripleFormatFile(const char *filename, int *arrayCount);

    private:
	void GenerateDotFormatDataFromPairings();

    public:
        /*
	    Destructor.
        */
        ~RNAStructure();

        /*
	    Return the base at a given location. Indexed starting at 0.
        */
        BaseData* GetBaseAt(unsigned int position);
        
        #if PERFORM_BRANCH_TYPE_ID
	RNABranchType_t* GetBranchTypeAt(unsigned int position);
        #else
        inline RNABranchType_t*	GetBranchTypeAt(unsigned int position) {
	     return NULL;
	}
        #endif

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
	 * Get the file name for this sequence.
         *
	 * The exactPath parameter specified whether to return the filename as it is 
	 * used in local StructViz folder accounting, which may be a modified 
	 * form of the original file path stored on disk, or whether to suppress the 
	 * suffixes added to files containing multiple samples. 
         */
        const char* GetFilename(bool exactPath = false) const;
        const char* GetFilenameNoExtension();
	const char* GetInitialFileComment() const;
	void SetFileCommentLines(std::string commentLineData, InputFileTypeSpec fileType);
	const char* GetSuggestedStructureFolderName();

        /*
	     Display the contents of the file in a window (or bring it to the top if already existing).
        */
        void DisplayFileContents(const char *titleSuffix = NULL);

        /*
         Returns a reference to the local m_sequence BaseData* pointer.
        */
        inline BaseData* & getSequence() {
            return m_sequence;
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

    public:
	inline const char * GetSequenceString() {
	     if(charSeq == NULL) {
	          GenerateString();
	     }
	     return charSeq;
	}

    private:

	/* Callbacks for the export buttons: */
        static void ExportFASTAFileCallback(Fl_Widget *btn, void *udata);
	static void ExportDotBracketFileCallback(Fl_Widget *btn, void *udata);
        static void CloseCTViewerContentWindowCallback(Fl_Widget *noWidget, void *udata);

        // The structure data
        unsigned int m_sequenceLength;
        BaseData* m_sequence;

        // The full path name of the file from which this sequence came.
        char *m_pathname, *m_pathname_noext, *m_exactPathName;
	char *m_fileCommentLine, *m_suggestedFolderName;
	InputFileTypeSpec m_fileType;

        // Info for displaying the file contents
        Fl_Double_Window *m_contentWindow;
        Fl_Text_Display *m_ctTextDisplay, *m_seqTextDisplay;
        Fl_Text_Buffer *m_ctTextBuffer, *m_ctStyleBuffer;
	Fl_Text_Buffer *m_seqTextBuffer, *m_seqStyleBuffer;
	Fl_Box *m_exportExtFilesBox, *m_seqSubwindowBox; 
	Fl_Box *m_ctSubwindowBox, *m_ctViewerNotationBox;
	Fl_Button *m_exportFASTABtn, *m_exportDBBtn;

	void DeleteContentWindow();
	static void HideContentWindowCallback(Fl_Widget *cwin, void *udata);

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
		       int srcDataLength, 
		       const char *fileExtText = ""
		  );
	};
	
	static int ActionOpenCTFileViewerWindow(int structureFolderIndex, 
			                        int minArcIdx = -1, 
						int maxArcIdx = -1);
	static bool ScrollOpenCTFileViewerWindow(int structIndex, int pairIndex); 

};

#endif //RNASTRUCTURE_H
