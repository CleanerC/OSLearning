// #include "myshell_parser.h"
#include "shell_helper.h"
#include <assert.h>

int main() {
    struct pipeline *my_pipeline = pipeline_build("echo hello | wc -l > foo2.txt\0");
    struct pipeline_command *cmd = my_pipeline->commands;

    //output wrtting into file check starts here
    int pass = STDIN_FILENO;
    
    while(cmd) {
        pass = execute(cmd, pass, my_pipeline->is_background);
        cmd = cmd->next;
    }
    
    FILE *fptr;
    fptr = fopen("foo2.txt","r");

    char string[2];
    fgets(string, 2, fptr);
    
    assert(strcmp("1", string) == 0);

    pipeline_free(my_pipeline);
}
