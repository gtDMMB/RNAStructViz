/* RadialLayoutImage.cpp : Implementation of the .h display functions and window classes;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#include "RadialLayoutImage.h"

CairoContext_t * RadialLayoutDisplayWindow::GetVRNARadialLayoutData(const char *rnaSubseq, 
		                            size_t startPos, size_t endPos, 
					    VRNAPlotType_t plotType) {
     
     char *effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, 0, MAX_SIZET);
     StringToUppercase(effectiveRNASubseq);
     startPos = 0; endPos = strlen(effectiveRNASubseq);
     unsigned int rnaSubseqLen = endPos - startPos + 1;
    
     vrna_md_t vmdParam; 
     char *mfeStructure = (char *) vrna_alloc((rnaSubseqLen + 1) * sizeof(char));
     vrna_fold_compound_t *vfc = vrna_fold_compound(effectiveRNASubseq, &vmdParam, VRNA_OPTION_DEFAULT);
     double minEnergy = (double) vrna_mfe(vfc, mfeStructure);
     short *pairTable = vrna_ptable(mfeStructure); 
     short *pairTableG = vrna_ptable(mfeStructure);

     int ge = 0, ee, gb, Lg, l[3], workingIdx;
     while( (ee = parse_gquad(mfeStructure + ge, &Lg, l)) > 0) {
          ge += ee;
	  gb = ge - 4 * Lg - l[0] - l[1] - l[2] + 1;
	  for(int pbpIdx = 0; pbpIdx < Lg; pbpIdx++) {
               pairTableG[ge - pbpIdx] = gb + pbpIdx;
	       pairTableG[gb + pbpIdx] = ge - pbpIdx;
	  }
     }

     float *xPosArr = (float *) vrna_alloc((rnaSubseqLen + 1) * sizeof(float));
     float *yPosArr = (float *) vrna_alloc((rnaSubseqLen + 1) * sizeof(float));
     if(plotType == PLOT_TYPE_SIMPLE) { 
          workingIdx = simple_xy_coordinates(pairTableG, xPosArr, yPosArr);
     }
     else if(plotType == PLOT_TYPE_CIRCULAR) {
          int radius = 3 * rnaSubseqLen;
	  workingIdx = simple_circplot_coordinates(pairTableG, xPosArr, yPosArr);
	  for(int idx = 0; idx < rnaSubseqLen; idx++) {
               xPosArr[idx] *= radius;
	       xPosArr[idx] += radius;
	       yPosArr[idx] *= radius;
	       yPosArr[idx] += radius;
	  }
     }
     else {
          workingIdx = naview_xy_coordinates(pairTableG, xPosArr, yPosArr);
     }
     if(workingIdx != rnaSubseqLen) {
          fl_alert("Warning: Strange things are happening with the ViennaRNA PS plot algorithm ...");
     }

     float xmin, xmax, ymin, ymax;
     xmin = xmax = xPosArr[0];
     ymin = ymax = yPosArr[0];
     for(int idx = 1; idx < rnaSubseqLen; idx++) {
          xmin = xPosArr[idx] < xmin ? xPosArr[idx] : xmin;
          xmax = xPosArr[idx] > xmin ? xPosArr[idx] : xmax;
          ymin = yPosArr[idx] < ymin ? yPosArr[idx] : ymin;
          ymax = yPosArr[idx] > xmin ? yPosArr[idx] : ymax;
     }
     CairoContext_t *plotCanvas = new CairoContext_t(2 * (xmax - xmin), 2 * (ymax - ymin));

     // TODO: Draw connecting lines;
     ge = 0;
     while( (ee = parse_gquad(mfeStructure + ge, &Lg, l)) > 0) {
	  ge += ee;
	  gb = ge - 4 * Lg - l[0] - l[1] - l[2] + 1;
	  for(int k = 0; k < Lg; k++) {
               int ii, jj;
	       for(int il = 0, ii = gb + k; il < 3; il++) {
	            jj = ii + l[il] + Lg;
		    plotCanvas->DrawBaseNode(ii, jj, '?', 0, 15, 
				             CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_CYAN), 
					     CairoContext_t::NodeStyle_t::CIRCLE_NODE);
		    ii = jj;
	       }
	       jj = gb + k;
	       plotCanvas->DrawBaseNode(jj, ii, 'X', 0, 15, 
			                CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_MAGENTA), 
					CairoContext_t::NodeStyle_t::SQUARE_NODE);
	  }
     }
     plotCanvase->SaveToFile("/home/maxie/Desktop/testRadial.png");

     free(effectiveRNASubseq);
     free(vfc);
     free(mfeStructure);
     free(pairTable);
     free(pairTableG);
     free(xPosArr);
     free(yPosArr);
     return plotCanvas;

}


