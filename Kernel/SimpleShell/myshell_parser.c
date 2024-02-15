#include "myshell_parser.h"
#include "stddef.h"

char whiteSpace[] = " \t\n";
char delim[] = "|";
int ii = 0;	  		//this is a globle var for all the for loops, since we are using C99 CFLAG

struct pipeline *pipeline_build(const char *command_line)
{
	struct pipeline *this = pipeline_alloc();
	/* make a copy of the command_line */
	char buffer[MAX_LINE_LENGTH] = {0};
	strncpy(buffer, command_line, MAX_LINE_LENGTH);
	
	trim(buffer);
	if(strcmp("&", &buffer[strlen(buffer) - 1]) == 0) {
		buffer[strlen(buffer) - 1] = '\0';
		this->is_background = true;
	}

	/* parse the command_line */
	char* parsedcmd[MAX_ARGV_LENGTH] = {0};
	int num_pipes = parsecmd(buffer, parsedcmd);	//also return the number of pipes

	/* malloc spaces for the pipeline struct */
	struct pipeline_command *prev = NULL;
	for(ii = 0; ii < num_pipes; ii++) {
		struct pipeline_command *command = pipeline_command_alloc();
		if(ii == 0) {
			this->commands = command;
		}
		//passing third argument to avoid checking
		toToken(command, parsedcmd[ii], (ii == 0)||(ii == num_pipes - 1));
		if(prev != NULL) {
			prev->next = command;
		}
		prev = command;
	}
	return this;
}

void pipeline_free(struct pipeline *pipeline)
{
	struct pipeline_command *command = pipeline->commands;
	while(command != NULL) {
		struct pipeline_command *next = command->next;
		free(command->redirect_in_path);
		free(command->redirect_out_path);
		int jj = 0;
		for(jj = 0; jj < MAX_ARGV_LENGTH; jj++) {
			if(command->command_args[jj] != NULL) {
				free(command->command_args[jj]);
			}
		}
		free(command);
		command = next;
	}
	free(pipeline);
}

char* trim(char* str)
{
	char *start, *end;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    start = str;
    while(*start && isspace((unsigned char)*start)) {
        start++;
    }
    memmove(str, start, end - start + 2);
	
	return str;
}

int parsecmd(char command[], char** parsedcmd)
{
	int index = 0;
	char* ptr = strtok(command, delim);
	while(ptr != NULL) {
		trim(ptr);
		parsedcmd[index++] = ptr;
		ptr = strtok(NULL, delim);
	}
	parsedcmd[index] = NULL;	// NULL, so we know where to stop
	return index;				//return the number of pipes
}

void toToken(struct pipeline_command* cmd, char* pipe, bool isBeginEnd)
{
	trim(pipe);
	int index = 0;
	//front and the end need check for file in and out direction
	if(isBeginEnd){
		char *inFound = strchr(pipe, '<');		//this could be done outside this bracket, but will increase runtime
		char *outFound = strchr(pipe, '>');
		if( (inFound == NULL) && (outFound == NULL) ) {
			char *ptr = strtok(pipe, whiteSpace);
			while(ptr != NULL) {
				cmd->command_args[index++] = toArg(ptr);
				ptr = strtok(NULL, whiteSpace);
			}
		} else {
			char *unToken = NULL;
			if(inFound != NULL) {
				char* fname = (char*)trim(inFound + sizeof(char));
				cmd->redirect_in_path = toArg(fname);
				unToken = strtok(pipe, "<"); 
			} else {
				char* fname = (char*)trim(outFound + sizeof(char));
				cmd->redirect_out_path = toArg(fname);
				unToken = strtok(pipe, ">"); 
			}
			char *ptr = strtok(unToken, whiteSpace);
			while(ptr != NULL) {
				cmd->command_args[index++] = toArg(ptr);
				ptr = strtok(NULL, whiteSpace);
			}
		}

	} else {
		char *ptr = strtok(pipe, whiteSpace);
		while(ptr != NULL) {
			cmd->command_args[index++] = toArg(ptr);
			ptr = strtok(NULL, whiteSpace);
		}
	}
	cmd->command_args[index] = NULL;
}

struct pipeline* pipeline_alloc()
{
	struct pipeline* this = malloc(sizeof(struct pipeline));

	this->commands = NULL;
	this->is_background = false;

	return this;
}

struct pipeline_command* pipeline_command_alloc()
{
	struct pipeline_command* this = malloc(sizeof(struct pipeline_command));
	int jj = 0;
	for(jj = 0; jj < MAX_ARGV_LENGTH; jj++) {
		this->command_args[ii] = NULL;
	}
	this->next = NULL;
	this->redirect_in_path = NULL;
	this->redirect_out_path = NULL;

	return this;
}

char* toArg(char* str)
{
	trim(str);
	int len = strlen(str) + 1;
	char* arg = malloc(len * sizeof(char));
	strncpy(arg, str, len - 1);
	arg[len - 1] = '\0';
	return arg;
}
