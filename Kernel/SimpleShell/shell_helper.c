#include "shell_helper.h"

int getCmd(char* buffer, bool pflag)
{
    if(pflag) {
	printf("myshell$");
    }
    fflush(NULL);
    if(fgets(buffer, MAX_LINE_LENGTH, stdin) == NULL) {
        return -1;      //EOF
    }
    //buffer[strcspn(buffer, "\n")] = '\0';
    return 1;       //normal
}

bool cmdValid(const char* buffer)
{
    char operation[MAX_ARGV_LENGTH];
    int nn = 0, index = 0;
    while(buffer[nn] != '\0') {
        if(buffer[nn] == '<' || buffer[nn] == '&' || buffer[nn] == '>') {
        operation[index++] = buffer[nn];
        }
        nn++;
    }
    operation[index] = '\0';
    for(nn = 1; operation[nn] != '\0'; nn++) {
        if(operation[nn] == '<' || (nn < (strlen(operation) - 2) && (operation[nn] == '>' || operation[nn] == '&'))){
            return false;
        }
    }
    return true;
}

int execute(struct pipeline_command *cmd, int pass, bool waitflag)
{
    int pfd[2];
    pipe(pfd);

    if(fork() == 0) {
        //child
        if(cmd->redirect_in_path) {     //this only happens when it is the beginning 
            pass = open(cmd->redirect_in_path, O_RDONLY);
        }
        if(cmd->redirect_out_path) {
            pfd[1] = creat(cmd->redirect_out_path, 0644);
        }

        if( cmd->next ) {
            dup2(pass, STDIN_FILENO);
            dup2(pfd[1], STDOUT_FILENO);
        } else {
            dup2(pass, STDIN_FILENO);
            if(cmd->redirect_out_path) {
                dup2(pfd[1], STDOUT_FILENO);
            }
        }

        if(execvp(cmd->command_args[0], cmd->command_args) < 0) {
            perror("ERROR: ");
            exit(EXIT_FAILURE);
        }

        close(pfd[1]);
        close(STDIN_FILENO);
        dup(STDIN_FILENO);
        close(pfd[0]);

    } else {
        //parent
      close(pfd[1]);
      if(!waitflag) {
	wait(0);
      }
    }

    if(pass != STDIN_FILENO) {
        close(pass);
    }

    if(!cmd->next) {
        close(pfd[0]);
    }

    return pfd[0];
}
