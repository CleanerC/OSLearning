//#include "myshell_parser.h"
#include "shell_helper.h"
#include <assert.h>

int main() {
    struct pipeline *my_pipeline = pipeline_build("echo hello > foo1.txt\0");
    struct pipeline_command *cmd = my_pipeline->commands;

    //output wrtting into file check starts here
    int pass = STDIN_FILENO;
    
    while(cmd) {
        pass = execute(cmd, pass, my_pipeline->is_background);
        cmd = cmd->next;
    }

    FILE *fptr;
    fptr = fopen("foo1.txt","r");

    char string[6];
    fgets(string, 6, fptr);
    
    assert(strcmp("hello", string) == 0);

    pipeline_free(my_pipeline);
}
