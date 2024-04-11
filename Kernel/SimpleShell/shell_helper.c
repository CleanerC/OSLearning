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

int execute(struct pipeline_command *cmd, int pass, bool waitflag)
{
    int pfd[2];
    pipe(pfd);

    if(fork() == 0) {
        //child
        if(cmd->redirect_in_path) {     //this only happens when it is the beginning 
            if( (pass = open(cmd->redirect_in_path, O_RDONLY)) < 0 ) {
				exit(2);
            }
        }
        if(cmd->redirect_out_path) {
            if( (pfd[1] = creat(cmd->redirect_out_path, 0644)) < 0 ) {
				exit(3);
            }
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
			exit(1);
        }

        close(pfd[1]);
        close(STDIN_FILENO);
        dup(STDIN_FILENO);
        close(pfd[0]);

    } else {
        //parent
        int wstatus;

        close(pfd[1]);
        if(!waitflag) {
	        wait(&wstatus);
            if (WIFEXITED(wstatus)) {
                int statusCode = WEXITSTATUS(wstatus);
                switch(statusCode)
                {
                case 1: 
                    printf("ERROR: Fail to exec\n");  
					break;

                case 2: 
                    printf("ERROR: Fail to open\n");
					break;

                case 3:
                    printf("ERROR: Fail to create\n");
					break;
	
                default: 
                    break;
                }
            }
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
