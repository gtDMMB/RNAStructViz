/*
    A class to hold an RNA structure.
*/

#ifndef RNASTRUCTURE_H
#define RNASTRUCTURE_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include <vector>

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

	/*
    enum Truth
    {
	TruePositive = 0x1,
	Uncounted = 0x2,
	FalsePositive = 0x4,
	FalseNegative = 0x8,
	FalsePosAndNeg = 0x16
    };
	*/

    // A value for the pair of unpaired bases.
    static const unsigned int UNPAIRED;

    // Data on a single base
    struct BaseData
    {
	unsigned int m_pair; // The index of the base it is paired with, or UNPAIRED.
	Base m_base;	// The base type.
	/*Truth m_truth; // How it compares to the reference structure */
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

    /*
	Display the contents of the file in a window (or bring it to the top if already existing).
    */
    void DisplayFileContents();

private:
    /*
	Constructor is private to force use of Create methods.
    */
    RNAStructure();

    /*
	Generate the string used for text display of the structure.
    */
    void GenerateString();
    
    /*
     Get the the char array of the sequence associated with this structure.
     */
    //const char* GetCharSeq() const;

    // The structure data
    unsigned int m_sequenceLength;
    BaseData* m_sequence;

    // The full path name of the file from which this sequence came.
    char* m_pathname;

    // Info for displaying the file contents
    Fl_Double_Window* m_contentWindow;
    Fl_Text_Display* m_textDisplay;
    Fl_Text_Buffer* m_textBuffer;
    char* m_displayString;
    
    char* charSeq;
    unsigned int charSeqSize;
};

#endif //RNASTRUCTURE_H
