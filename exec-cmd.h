#include <stdio.h>
#include "data.h"

int exec_config(int argc, char *argv[])
{
    int opt_global = 0;
    int arg_ind = 2;
    if (argc < 4) {
        printerr(FEW_ARGUMENTS);
        return 1;//code
    }
    if ( strcmp(argv[arg_ind], GLOBAL_OPTION) == 0 ) {
        opt_global = 1;
        arg_ind++;
    }
    if ( !is_in_repo() && !opt_global ) {
        printerr(NO_REPO);
        return 1;//code
    }
    // Create initial config files
    create_config_global();

    if ( strncmp(argv[arg_ind], "user.", 5) == 0 ) {
        if (argc == 4 + opt_global) {
            if ( config_user(argv[arg_ind] + 5, argv[arg_ind + 1], opt_global) == 1 ) {
                printerr(INVALID_CONFIG);
                return 1;
            } // done
        } else {
            printerr(INVALID_USAGE);
            return 1;//code
        }
    } else if ( strncmp(argv[arg_ind], "alias.", 6) == 0 ) {
        if ( config_alias(argv[arg_ind] + 6, argv[arg_ind + 1], opt_global) == 1 ){
            printerr(INVALID_COMMAND_FOR_ALIAS);
            return 1;
        } // done
    } else {
        printerr(INVALID_CONFIG);
        return 1;
    }
    return 0;
}

int exec_init(int argc, char *argv[])
{
    if (argc != 2) {
        printerr(INVALID_USAGE);
        return 1;
    }
    if ( is_in_repo() ) {
        printerr(ALREADY_IN_REPOSITORY);
        return 1;
    }
    initialize_repo();
    printf(REPO_INIT_MESSAGE, find_repo_data()); // Message
}

int exec_add(int argc, char *argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }
    if (argc < 3) {
        printerr(FEW_ARGUMENTS);
        return 1;
    }
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "n") == 0 ) {
            if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            // SHOW FILES AND DIRECTORIES 
        } else if ( strcmp(argv[arg_ind] + 1, "f") == 0 ) {
            arg_ind++;
        // TODO: add -redo option
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    }
    for (int i = arg_ind; i < argc; i++) {
        // stage_file(argv[i]);
    }
    
}