/* ViennaBoltzmannSampling.cpp : Implementation of the sampling routines;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.12.04
 */

#include "ViennaBoltzmannSampling.h"
#include "RNAStructure.h"
#include "ConfigOptions.h"
#include "TerminalPrinting.h"

extern "C" {
     #include <ViennaRNA/utils/basic.h>
     #include <ViennaRNA/params/basic.h>
     #include <ViennaRNA/datastructures/basic.h>
     #include <ViennaRNA/utils/strings.h>
     #include <ViennaRNA/fold.h>
     #include <ViennaRNA/fold_compound.h>
     #include <ViennaRNA/gquad.h>
     #include <ViennaRNA/part_func.h>
     #include <ViennaRNA/boltzmann_sampling.h>
}

vector<string> ViennaBoltzmannSampling::ParseFASTAFileFromPath(const char *fastaFilePath) {
     
     FILE *fpFastaFile = fopen(fastaFilePath, "r");
     if(fpFastaFile == NULL) {
          throw string(strerror(errno));
     }
     char lineBuf[MAX_SEQUENCE_SIZE + 1];
     char commentsBuf[MAX_SEQUENCE_SIZE + 1], baseDataBuf[MAX_SEQUENCE_SIZE + 1];
     commentsBuf[0] = baseDataBuf[0] = '\0';
     while(true) {
          char *lineReturn = fgets(lineBuf, MAX_SEQUENCE_SIZE, fpFastaFile);
	  if(lineReturn == NULL && feof(fpFastaFile)) {
               break;
	  }
	  else if(lineReturn == NULL) {
	       throw string(strerror(errno));
	  }
	  else if(lineBuf[0] == '\n') {
	       continue;
	  }
	  int lineLength = strlen(lineBuf);
	  if(lineBuf[lineLength - 1] == '\n') {
               lineBuf[lineLength - 1] = '\0';
	  }
	  if(lineBuf[0] == '>') {
               strcat(commentsBuf, lineBuf);
	  }
	  else {
               strcpy(baseDataBuf, lineBuf);
	       break;
	  }
     }
     fclose(fpFastaFile);

     vector<string> fastaDataResults;
     fastaDataResults.push_back(commentsBuf);
     fastaDataResults.push_back(baseDataBuf);
     return fastaDataResults;

}

bool ViennaBoltzmannSampling::FASTAFileIsValid(const vector<string> &parseResults) {
     return parseResults.size() == 2 && parseResults[INDEX_BASESEQ_DATA].length() > 0;
}

unsigned int ViennaBoltzmannSampling::CountVRNASampleSize(char **sampleDotDataArr) {
     if(sampleDotDataArr == NULL) {
          return 0;
     }
     unsigned int sampleCount = 0;
     while(sampleDotDataArr[sampleCount] != NULL) {
          ++sampleCount;
     }
     return sampleCount;
}

char ** ViennaBoltzmannSampling::ComputeVRNABoltzmannSamples(vector<string> &fastaParseResults, 
		                                             unsigned int sampleSize) {
     
     if(!ViennaBoltzmannSampling::FASTAFileIsValid(fastaParseResults)) {
          return NULL;
     }
     vrna_init_rand();
     
     char *baseSeq = strdup(fastaParseResults[INDEX_BASESEQ_DATA].c_str());
     vrna_md_t vmd; 
     vrna_md_set_default(&vmd);
     vmd.uniq_ML = 1;
     vrna_fold_compound_t *vfc = vrna_fold_compound(baseSeq, &vmd, VRNA_OPTION_DEFAULT | VRNA_OPTION_PF);
     vrna_pf(vfc, baseSeq);
     char **dotSampleData = vrna_pbacktrack_num(vfc, sampleSize, VRNA_PBACKTRACK_NON_REDUNDANT);

     vrna_mx_pf_free(vfc);
     vrna_fold_compound_free(vfc);
     Free(baseSeq);
     return dotSampleData;

}

vector<ViennaBoltzmannSampling::StructureData_t> ViennaBoltzmannSampling::GetBoltzmannSamples( 
		const char *inputFastaFile, unsigned int sampleSize) {
     
     if(inputFastaFile == NULL) {
          throw string("Unable to get Boltzmann samples: Invalid FASTA file parameters.");
     }
     vector<string> fastaParseResults = ViennaBoltzmannSampling::ParseFASTAFileFromPath(inputFastaFile);
     char **dotSampleData = ViennaBoltzmannSampling::ComputeVRNABoltzmannSamples(fastaParseResults, sampleSize);
     unsigned int actualSampleSize = ViennaBoltzmannSampling::CountVRNASampleSize(dotSampleData);
     if(sampleSize == 0) {
          throw string("VRNA returned no samples.");
     }
     else if(actualSampleSize < sampleSize) {
          TerminalText::PrintWarning("Actual computed sample size for \"%s\" is %d < %d\n", 
			             inputFastaFile, actualSampleSize, sampleSize);
     }
     vector<ViennaBoltzmannSampling::StructureData_t> sdv;
     ViennaBoltzmannSampling::StructureData_t sd;
     sd.commentLines = fastaParseResults[INDEX_COMMENT_LINES];
     sd.baseSeqData = fastaParseResults[INDEX_BASESEQ_DATA];
     for(int s = 0; s < actualSampleSize; s++) {
          sd.dotPairingData = string(dotSampleData[s]);
	  sdv.push_back(sd);
	  Free(dotSampleData[s]);
     }
     Free(dotSampleData);
     return sdv;

}

RNAStructure ** ViennaBoltzmannSampling::GenerateStructuresFromSampleData(
		const char *filePath, const vector<ViennaBoltzmannSampling::StructureData_t> &sdv) {
     
     if(sdv.size() == 0) {
          return NULL;
     }
     RNAStructure **rnaStructsArr = (RNAStructure **) malloc(sdv.size() * sizeof(RNAStructure *));
     for(unsigned int si = 0; si < sdv.size(); si++) {
          RNAStructure *rnaStruct = RNAStructure::CreateFromDotBracketData(
			                 filePath, sdv[si].baseSeqData.c_str(), 
			                 sdv[si].dotPairingData.c_str(), si
				    );
	  if(rnaStruct == NULL) {
               for(int j = 0; j < si; j++) {
	            Delete(rnaStructsArr[j], RNAStructure);
	       }
	       Free(rnaStructsArr);
	       return NULL;
	  }
	  rnaStructsArr[si] = rnaStruct;
     }
     return rnaStructsArr;

}
