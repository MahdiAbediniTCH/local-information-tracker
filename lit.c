// Include standard libraries

// Include headers
#include "exec-cmd.h"

int (*CMD_FUNCTIONS[N_COMMANDS])(int, char*[]) = {
exec_config,
exec_init
}; // Array of function pointers, indexes are ... to the indexes of strings in COMMANDS

// Main command handler function
int main(int argc, char *argv[])
{
    if (argc == 1) {
        // usage message
        return 1;
    }
    int command_ind = search_str(COMMANDS, argv[1], N_COMMANDS);
    if (command_ind == -1) {
        // check if it's an alias
        // usage
        return 1;
    }
    int call_result;
    call_result = (CMD_FUNCTIONS[command_ind])(argc, argv);
    return call_result;
}