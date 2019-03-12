/* RadialLayoutImage.cpp : Implementation of the .h display functions and window classes;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#include "RadialLayoutImage.h"

//RadialLayoutDisplayWindow::RadialLayoutDisplayWindow(size_t width, size_t height) : 
//	Fl_Cairo_Window(width, height) 
//{
//}

CairoContext_t * RadialLayoutDisplayWindow::GetVRNARadialLayoutData(const char *rnaSubseq, 
		                            size_t startPos, size_t endPos, 
					    VRNAPlotType_t plotType) {
     
     char *effectiveRNASubseq = GetSubstringFromRange(rnaSubseq, 0, MAX_SIZET);
     StringToUppercase(effectiveRNASubseq);
     startPos = 0; endPos = strlen(effectiveRNASubseq);
     endPos = endPos ? endPos - 1 : 0;
     unsigned int rnaSubseqLen = endPos - startPos + 1;
     vrna_seq_toRNA(effectiveRNASubseq);
     fprintf(stderr, "%s\n", rnaSubseq);
     fprintf(stderr, "%s\n", effectiveRNASubseq);

     vrna_fold_compound_t *vfc = vrna_fold_compound(effectiveRNASubseq, NULL, VRNA_OPTION_DEFAULT);
     vrna_md_t vmdParam = vfc->params->model_details;
     int length = vfc->length;
     
     char *mfeStructure = (char *) vrna_alloc((length + 1) * sizeof(char));
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
          fl_alert("Warning: Strange things are happening with the ViennaRNA PS plot algorithm ...\n"
	           " > workingIdx = %d, RNASubseqLen = %d;", 
		   workingIdx, rnaSubseqLen);
     }

     float xmin, xmax, ymin, ymax;
     xmin = xmax = xPosArr[0];
     ymin = ymax = yPosArr[0];
     for(int idx = 1; idx < rnaSubseqLen; idx++) {
          xmin = xPosArr[idx] < xmin ? xPosArr[idx] : xmin;
          xmax = xPosArr[idx] > xmax ? xPosArr[idx] : xmax;
          ymin = yPosArr[idx] < ymin ? yPosArr[idx] : ymin;
          ymax = yPosArr[idx] > ymax ? yPosArr[idx] : ymax;
     }
     float xScale = (float) (0.8 *(DEFAULT_RLWIN_WIDTH / xmax));
     float yScale = (float) (0.8 *(DEFAULT_RLWIN_HEIGHT / ymax));
     
     CairoContext_t *plotCanvas = new CairoContext_t(DEFAULT_RLWIN_WIDTH, DEFAULT_RLWIN_HEIGHT);
     plotCanvas->SetColor(CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK));

     // TODO: Draw connecting lines;
     int idx = 0;
     while(idx < rnaSubseqLen) {
          plotCanvas->DrawBaseNode(xPosArr[idx] * xScale, yPosArr[idx] * yScale, 
			           effectiveRNASubseq[idx], idx + 1, 8, 
				   CairoColor_t::GetCairoColor(CairoColorSpec_t::CR_BLACK));
	  idx++;
     }

     free(effectiveRNASubseq);
     free(vfc);
     free(mfeStructure);
     free(pairTable);
     free(pairTableG);
     free(xPosArr);
     free(yPosArr);
     
     //plotCanvas->SaveToImage("/home/maxie/Desktop/testRadial.png");
     return plotCanvas;

}


