#ifndef SHELL_HELPER_H
#define SHELL_HELPER_H
#include "myshell_parser.h"


/* get from stdin */
int getCmd(char* buffer, bool pflag);
/* check if the format is valid for pipe */
bool cmdValid(const char* buffer);
/* wrapper function for execvp() */
int execute(struct pipeline_command *cmd, int pass, bool waitflag);

#endif
