// Include standard libraries

// Include headers
#include "utility.h"
#include "exec-cmd.h"

int (*CMD_FUNCTIONS[N_COMMANDS])(int, char*[]) = {
exec_config,
exec_init,
exec_add,
exec_reset,
exec_status,
exec_commit,
exec_set,
exec_replace,
exec_remove,
exec_log,
exec_branch,
exec_checkout,
exec_revert,
}; // Array of function pointers, indexes are respective to the indexes of strings in COMMANDS

// Main command handler function
int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf(USAGE);
        return 1;
    }
    int command_ind = search_str(COMMANDS, argv[1], N_COMMANDS);
    if (command_ind == -1) {
        char* alias_command = get_alias(argv[1]);
        if (alias_command == NULL) {
            printf(USAGE);
            return 1;
        }
        // Append extra arguments to the command
        for (int i = 2; i < argc; i++) {
            strcat(alias_command, " ");
            if ( strchr(argv[i], ' ') != NULL ) {
                strcat(alias_command, "\"");
                strcat(alias_command, argv[i]);
                strcat(alias_command, "\"");
            } else {
                strcat(alias_command, argv[i]);
            }
        }
        system(alias_command);
    }
    int call_result;
    call_result = (CMD_FUNCTIONS[command_ind])(argc, argv);
    return call_result;
}