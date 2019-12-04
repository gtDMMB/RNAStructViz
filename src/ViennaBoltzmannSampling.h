/* ViennaBoltzmannSampling.h : Integrated support for Boltzmann sampling of a given sequence string 
 *                             to produce N unique DotBracket structures via the ViennaRNA library;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.12.04
 */

#ifndef __VIENNA_BOLTZMANN_SAMPLING_H__
#define __VIENNA_BOLTZMANN_SAMPLING_H__

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <vector>
using namespace std;

#include "RNAStructVizTypes.h"

#define DEFAULT_SAMPLE_SIZE             (16)

namespace ViennaBoltzmannSampling {

     typedef enum {
          INDEX_COMMENT_LINES    = 0, 
	  INDEX_BASESEQ_DATA     = 1,
	  INDEX_DOTBRACKET_DATA  = 2,
     } StructureDataIndex_t;

     vector<string> ParseFASTAFileFromPath(const char *fastaFilePath);
     bool FASTAFileIsValid(const vector<string> &parseResults);

     typedef struct {
          string commentLines;
	  string baseSeqData;
	  string dotPairingData;
     } StructureData_t;

     unsigned int CountVRNASampleSize(char **sampleDotDataArr);
     char ** ComputeVRNABoltzmannSamples(vector<string> &fastaParseResults, 
		                         unsigned int sampleSize = DEFAULT_SAMPLE_SIZE);
     vector<StructureData_t> GetBoltzmannSamples(const char *inputFastaFile, 
		                                 unsigned int sampleSize = DEFAULT_SAMPLE_SIZE); 

     RNAStructure ** GenerateStructuresFromSampleData(const char *filePath, 
		                                      const vector<StructureData_t> &sdv);

}

#endif
