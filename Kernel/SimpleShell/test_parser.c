#include "shell_helper.h"
#include <assert.h>

int main() {
    struct pipeline *my_pipeline = pipeline_build("ls -al<input.txt|cat|hello>foo.txt&\n");

    //background
    assert(my_pipeline->is_background);

    // Test that a pipeline was returned
    assert(my_pipeline != NULL);
    assert(my_pipeline->commands != NULL);
    
    // Test the parsed args pipe1
    assert(strcmp("ls", my_pipeline->commands->command_args[0]) == 0);
    assert(strcmp("-al", my_pipeline->commands->command_args[1]) == 0);
    assert(my_pipeline->commands->command_args[2] == NULL);

    // Test the parsed args pipe2
    assert(strcmp("cat", my_pipeline->commands->next->command_args[0]) == 0);
    assert(my_pipeline->commands->next->command_args[1] == NULL);

    //test the parsed args pipe3
    assert(strcmp("hello", my_pipeline->commands->next->next->command_args[0]) == 0);
    assert(my_pipeline->commands->next->next->command_args[1] == NULL);
    
    // Test the redirect state
    assert(strcmp("input.txt", my_pipeline->commands->redirect_in_path) == 0);
    assert(strcmp("foo.txt", my_pipeline->commands->next->next->redirect_out_path) == 0);
    
    // Test that there are multiple parsed command in the pipeline
    assert(my_pipeline->commands->next != NULL);
    assert(my_pipeline->commands->next->next->next == NULL);

    pipeline_free(my_pipeline);
}
