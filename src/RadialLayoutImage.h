/* RadialLayoutImage.h : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#ifndef __RADIAL_LAYOUT_IMAGE_H__
#define __RADIAL_LAYOUT_IMAGE_H__

#include <cairo.h>

#include "ConfigOptions.h"

#define MAX_SIZET                  ((size_t) -1)



TODO GetVRNARadialLayoutData(const char *rnaSubseq, size_t startPos = 0, size_t endPos = MAX_SIZET) {
     
     char *effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, startPos, endPos);
     StringToUppercase(effectiveRNASubseq);
     startPos = 0; endPos = strlen(effectiveRNASubseq);

     unsigned int subseqLen = endPos - startPos + 1;
     char *rec_sequence = NULL, *mfe_structure = NULL;
     double minEnery = 0.0, curEnergy = 0.0;
     vrna_fold_compound_t *vfc = NULL;
     vrna_md_t vmdParam;

     vfc = vrna_fold_compound(effectiveRNASubseq, &vmdParam, VRNA_OPTION_DEFAULT);
     mfe_structure = (char *) vrna_alloc((subseqLen + 1) * sizeof(char));
     minEnergy = (double) vrna_mfe(






     free(effectiveRNASubseq);

}












#endif
