// Include standard libraries

// Include headers
#include "utility.h"
#include "exec-cmd.h"

// Definitions and constants
#define N_COMMANDS 2 // Number of allowed commands
const char* COMMANDS[N_COMMANDS] = { // Array of command strings
"config", 
"init", 
};

int (*CMD_FUNCTIONS[N_COMMANDS])(int, char*[]) = {
exec_config,

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
        // command not found error
        return 1;
    }
    int call_result;
    call_result = (*CMD_FUNCTIONS[command_ind])(argc, argv);
    return 0;
}