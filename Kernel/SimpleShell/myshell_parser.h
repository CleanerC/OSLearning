#ifndef MYSHELL_PARSER_H
#define MYSHELL_PARSER_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 512
#define MAX_ARGV_LENGTH (MAX_LINE_LENGTH / 2 + 1)

struct pipeline_command {
  char *command_args[MAX_ARGV_LENGTH]; // arg[0] is command, rest are arguments
  char *redirect_in_path;  // NULL or Name of file to redirect in from 
  char *redirect_out_path; // NULL or Name of a file to redirect out to
  struct pipeline_command *next; // next command in the pipeline. NULL if done 
};

struct pipeline {
  struct pipeline_command *commands; // first command
  bool is_background; // TRUE if should execue in background
};

void pipeline_free(struct pipeline *pipeline);

struct pipeline *pipeline_build(const char *command_line);

/* remove white spaces */
char* trim(char* str);

/* to parse the cmd */
int parsecmd(char command[], char** parsedcmd);

/* this function is to tokenize each pipecmd */
void toToken(struct pipeline_command* cmd, char* pipe, bool isBeginEnd);

/* wrapper alloc function */
struct pipeline* pipeline_alloc();

/* wrapper alloc function */
struct pipeline_command* pipeline_command_alloc();

/* string to command_args*/
char* toArg(char* str);


#endif /* MYSHELL_PARSER_H */
