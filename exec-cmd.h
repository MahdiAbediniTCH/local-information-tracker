#ifndef EXECCMD_H
#define EXECCMD_H

#include <stdio.h>
#include "data.h"
#include "changes.h"
#include "action-history.h"

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
    return 0;
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
            print_stage_status(".", get_stage_object(), atoi(argv[3]));
            return 0;
        } else if ( strcmp(argv[arg_ind] + 1, "f") == 0 ) {
            arg_ind++;
        } else if ( strcmp(argv[arg_ind] + 1, "redo") == 0 ) {
            if ( argc != 3) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            if ( execute_last_add_cmd() != 0 ) {
                printerr("No last record was found\n");
            }
            return 0;
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    }
    // TODO: did_stage doesn't get the good result from the function
    bool did_stage = false;
    for (int i = arg_ind; i < argc; i++) {
        // TODO: Later implement deleting files that were staged before
        State* stage_obj = get_stage_object();
        int return_status = stage_file(argv[i], stage_obj);
        write_state(stage_obj, stage_obj->data_dir);
        if (!return_status) {
            did_stage = true;
        } else if (return_status == 1) {
            printf(FILEN_NOT_FOUND, argv[i]);
        }
    }
    if (did_stage) {
        store_last_add_cmd(argc, argv);
        printf(STAGE_SUCCESS);
        return 0;
    }
    return 1;
}

int exec_reset(int argc, char *argv[])
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
        if ( strcmp(argv[arg_ind] + 1, "f") == 0 ) {
            arg_ind++;
        } else if ( strcmp(argv[arg_ind] + 1, "undo") == 0 ) {
            if ( argc != 3) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            if ( unstage_last_add() != 0 ) {
                printerr("No last record was found\n");
            }
            return 0;
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    }
    bool did_unstage = false;
    for (int i = arg_ind; i < argc; i++) {
        // TODO: Later implement deleting files that were staged before
        State* stage_obj = get_stage_object();
        int return_status = unstage_file(argv[i], stage_obj);
        write_state(stage_obj, stage_obj->data_dir);
        if (!return_status) {
            did_unstage = true;
        } else if (return_status == 1) {
            printf(FILEN_NOT_FOUND, argv[i]);
        }
    }
    if (did_unstage) {
        printf(RESET_SUCCESS);
        return 0;
    }
    return 1;
}

int exec_status(int argc, char *argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }
    if (argc > 2) {
        printerr(INVALID_USAGE);
        return 1;
    }
    show_file_status(find_root_path(), get_head_commit(), get_stage_object());
}

int exec_commit(int argc, char *argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    } if (argc < 3) {
        printerr(INVALID_USAGE);
        return 1;
    } if (strcmp(argv[2], "-m") != 0) {
        printerr(INVALID_USAGE);
        return 1;
    } if (argc < 4) {
        printerr(EXPECTED_MSG);
        return 1;
    } if (argc != 4) {
        printerr(INVALID_USAGE);
        return 1;
    } if (strlen(argv[3]) > COMMIT_MESSAGE_MAX) {
        printerr(MSG_TOO_LONG);
        return 1;
    }
    State* commit = do_a_commit(argv[3]);
    if (commit != NULL) {
        printf(COMMIT_SUCCESS, commit->state_id, ctime(&(commit->time_created)));
        return 0;
    } else {
        printerr(NOTHING_TO_COMMIT);
        return 1;
    }
}

#endif