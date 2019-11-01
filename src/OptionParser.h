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
     PRINT_ABOUT = 2, 
     PRINT_HELP  = 3,
     PRINT_DEBUG = 4,
     NEW_CONFIG  = 5,
} StructVizOptionAction_t;

static const struct option longarg_options[] = {
     { "about",           no_argument,      NULL,                      PRINT_ABOUT },
     { "debug",           no_argument,      NULL,                      PRINT_DEBUG }, 
     { "help",            no_argument,      NULL,                      PRINT_HELP  },
     { "new-config",      no_argument,      NULL,                      NEW_CONFIG  },
     { "no-ansi-color",   no_argument,      &PRINT_ANSI_COLOR,         SETVAR      },
     { "no-unicode",      no_argument,      &PRINT_TERMINAL_UNICODE,   SETVAR      },
     { "quiet",           no_argument,      &CFG_QUIET_MODE,           SETVAR      },
     { "verbose",         no_argument,      &CFG_VERBOSE_MODE,         SETVAR      },
};

static const char *shortarg_options = "hqvV";

int ParseStructVizCommandOptions(int &argc, char ** &argv);

#endif
