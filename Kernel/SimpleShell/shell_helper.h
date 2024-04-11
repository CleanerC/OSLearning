#ifndef SHELL_HELPER_H
#define SHELL_HELPER_H
#include "myshell_parser.h"


/* get from stdin */
int getCmd(char* buffer, bool pflag);
/* wrapper function for execvp() */
int execute(struct pipeline_command *cmd, int pass, bool waitflag);

#endif
