#include <stdio.h>
#include "utility.h"
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
    printf("init \n");
}