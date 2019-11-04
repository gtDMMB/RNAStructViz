/* OptionParser.h : Custom parsing of command line arguments for the application; 
 *                  (Allows us to do some cool custom things at runtime.)
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.10.31
 */

#ifndef __OPTION_PARSER_H__
#define __OPTION_PARSER_H__

#include <stdlib.h>
#include <errno.h>
#include <getopt.h>

#include "ConfigOptions.h"
#include "TerminalPrinting.h"

#define REMOVE_STRUCTVIZ_OPTIONS             (true)
#define EXIT_ON_ERROR                        (true)

typedef enum {
     NONE        = 0,
     SETVAR      = 1,
     NOSETVAR    = 2, 
     PRINT_ABOUT = 3, 
     PRINT_HELP  = 4,
     PRINT_DEBUG = 5,
     NEW_CONFIG  = 6,
} StructVizOptionAction_t;

void ProcessAboutOption();
void ProcessHelpOption();
void ProcessDebugOption();
void ProcessNewConfigOption();
void ProcessQuietOption();
void ProcessVerboseOption();

int ParseStructVizCommandOptions(int &argc, char ** &argv);

#endif
