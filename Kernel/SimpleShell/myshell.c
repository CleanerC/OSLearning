// #include "myshell_parser.h"
#include "shell_helper.h"

void handle_sigchld(int sig) {
	wait(0);	
}

int main(int argc, char** argv) {
	//initial buffer
    static char buffer[MAX_LINE_LENGTH];
    bool pflag = true;
    
    if(argc >= 2) {
	pflag = strcmp("-n", argv[1]);
    }    

    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

	//this rapper fgets function will return -1 if ctrl-d happens;
    while(getCmd(buffer, pflag) > 0) {             
		struct pipeline *pipe = pipeline_build(buffer);
        struct pipeline_command *cmd = pipe->commands;
       
        int pass = STDIN_FILENO;   //to pass a fd to the next pipe_command
	
        while(cmd) {
            pass = execute(cmd, pass, pipe->is_background);
            cmd = cmd->next;
        }

	    if(!pipe->is_background)
		    wait(0);

        pipeline_free(pipe);
        memset(buffer, '\0', sizeof(buffer));      //clear the buffer
    }

    exit(EXIT_SUCCESS);
}
