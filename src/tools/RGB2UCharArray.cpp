/* RGB2UCharArray.cpp : Apparently this is not standard, so I have to write this on my own; 
 *                      The program converts an input RGB(A) image into a correspoding C-style 
 *                      syntax uchar array buffer that can be included and hence make the 
 *                      image data static at compile time.
 * Author: Maxie D. Schmidt
 * Created: 2018.10.21
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define PAD_WITH_ALPHA           (false)
#define NUM_UCHARS_PER_COLUMN    (8)

int main(int argc, char **argv) { 

     if(argc < 4) { 
          fprintf(stderr, "Usage: %s <INPUT-RGB-FILE> <ARRAY-NAME> <OUTPUT-FILE-PATH>\n", argv[0]);
	  exit(-1);
     }

     const char *inputRGBFile = argv[1], *arrName = argv[2], *outputTextFile = argv[3];
     FILE *fpInput = fopen(inputRGBFile, "rb+");
     FILE *fpOutput = fopen(outputTextFile, "w+");
     int pixCount = 0; 

     fprintf(fpOutput, "static unsigned char %s[] = {", arrName); 
     while(!feof(fpInput)) { 

          unsigned char rgbaPix;
	  fread(&rgbaPix, 1, sizeof(unsigned char), fpInput);
	  if((pixCount++ % NUM_UCHARS_PER_COLUMN) == 0) { 
               fprintf(fpOutput, "\n     ");
	  }
	  fprintf(fpOutput, "\'\\x%02x\', ", rgbaPix);
	  if((pixCount % 3) == 0 && PAD_WITH_ALPHA) {
               fprintf(fpOutput, "\'\\x00\', ");
	       ++pixCount;
	  }

     }
     fprintf(fpOutput, "\n}; /* %s */", arrName);

     fclose(fpInput);
     fclose(fpOutput);

     return 0;

}
