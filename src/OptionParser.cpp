/* OptionParser.cpp : 
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.31
 */

#include "OptionParser.h"

int ParseStructVizCommandOptions(int &argc, char ** &argv) {

     int option_index = 0, optch = -1;
     bool doneParsingStructViz = false;
     while(true) {
	  optch = getopt_long(argc, argv, shortarg_options, longarg_options, &option_index);
	  if(optch == -1) {
	       break;
	  }
	  else if(optch == 0) {


	  }
	  switch(optch) {
               case 'h':
	            break;
	       case 'q':
		    break;
	       case 'v':
		    break;
	       case 'V':
		    break;
	       default:
		    doneParsingStructViz = true;
		    break;
	  }
	  if(doneParsingStructViz) {
               break;
	  }
     }
     int numOptionsParsed = optind;
     if(REMOVE_STRUCTVIZ_OPTIONS) {
          argc -= numOptionsParsed;
	  argv[numOptionsParsed] = argv[0];
     }

     return EXIT_SUCCESS;

}
