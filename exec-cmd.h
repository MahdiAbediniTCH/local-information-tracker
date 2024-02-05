#ifndef EXECCMD_H
#define EXECCMD_H

#include <stdio.h>
#include "data.h"
#include "changes.h"
#include "action-history.h"
#include "msg-shortcuts.h"
#include "log.h"

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
    bool did_stage = false;
    State* stage_obj = get_stage_object();
    for (int i = arg_ind; i < argc; i++) {
        int return_status = stage_file(argv[i], stage_obj);
        write_state(stage_obj, stage_obj->data_dir);
        if (!return_status) {
            did_stage = true;
        } else if (return_status == 1) {
            printf(FILEN_NOT_FOUND, argv[i]);
        }
    }
    // Detect deleted files
    if (detect_deleted_files()) {
        printf("Detected file deletion compared to HEAD\n");
    }
    // Success message
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
    char branch[BRANCH_NAME_MAX];
    printf(ON_BRANCH, get_current_branch_name(branch));
    show_file_status(find_root_path(), get_head_commit(), get_stage_object());
    show_deleted_files();
}

int exec_commit(int argc, char *argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    } if (argc < 3) {
        printerr(INVALID_USAGE);
        return 1;
    } if (detached_head(0)) {
        printerr(COMMIT_HEAD_DETACHED);
        return 1;
    }
    State* commit = NULL;
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "m") == 0 ) {
            if (argc < 4) {
                printerr(EXPECTED_MSG);
                return 1;
            } if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            } if (strlen(argv[3]) > COMMIT_MESSAGE_MAX) {
                printerr(MSG_TOO_LONG);
                return 1;
            }
            commit = do_a_commit(argv[3]);

        } else if ( strcmp(argv[arg_ind] + 1, "s") == 0 ) {
            if (argc < 4) {
                printerr(EXPECTED_SHORTCUT);
                return 1;
            } if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            char message[COMMIT_MESSAGE_MAX + 1];
            if ( !get_message_from_shortcut(message, argv[3]) ) {
                printf(SHORTCUT_NOT_FOUND, argv[3]);
                return 1;
            }
            commit = do_a_commit(message);

        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    } else {
        printerr(OPTION_REQUIRED);
        return 1;
    }
    if (commit != NULL) {
        printf(COMMIT_SUCCESS, commit->state_id, ctime(&(commit->time_created)), commit->message);
        return 0;
    } else {
        printerr(NOTHING_TO_COMMIT);
        return 1;
    }
}

int exec_set(int argc, char* argv[])
{
    if (argc != 6) {
        printerr(INVALID_USAGE);
        return 1;
    }
    int arg_ind = 2;
    char shortcut[SHORTCUT_MAX] = "";
    char message[COMMIT_MESSAGE_MAX] = "";

    for (int i = 0; i < 2; i++) {
        if (argv[arg_ind][0] == '-') {
            if ( strcmp(argv[arg_ind] + 1, "m") == 0 ) {
                arg_ind++;
                strcpy(message, argv[arg_ind]);
                if (strlen(message) > COMMIT_MESSAGE_MAX) {
                    printerr(MSG_TOO_LONG);
                    return 1;
                }
            } else if ( strcmp(argv[arg_ind] + 1, "s") == 0 ) {
                arg_ind++;
                strcpy(shortcut, argv[arg_ind]);
            } else {
                printerr(INVALID_OPTION);
                return 1;
            }
        } else {
            printerr(OPTION_REQUIRED);
            return 1;
        }
        arg_ind++;
    }
    if ( strlen(shortcut) == 0 || strlen(message) == 0 ) {
        printerr(INVALID_USAGE);
        return 1;
    } if ( !add_new_shortcut(shortcut, message) ) {
        printf(SHORTCUT_ALREADY_EXISTS, shortcut);
        return 1;
    } else {
        printf(SHORTCUT_ADD_SUCCESS, shortcut);
        return 0;
    }
}

int exec_replace(int argc, char* argv[])
{
    if (argc != 6) {
        printerr(INVALID_USAGE);
        return 1;
    }
    int arg_ind = 2;
    char shortcut[SHORTCUT_MAX] = "";
    char message[COMMIT_MESSAGE_MAX] = "";

    for (int i = 0; i < 2; i++) {
        if (argv[arg_ind][0] == '-') {
            if ( strcmp(argv[arg_ind] + 1, "m") == 0 ) {
                arg_ind++;
                strcpy(message, argv[arg_ind]);
                if (strlen(message) > COMMIT_MESSAGE_MAX) {
                    printerr(MSG_TOO_LONG);
                    return 1;
                }
            } else if ( strcmp(argv[arg_ind] + 1, "s") == 0 ) {
                arg_ind++;
                strcpy(shortcut, argv[arg_ind]);
            } else {
                printerr(INVALID_OPTION);
                return 1;
            }
        } else {
            printerr(OPTION_REQUIRED);
            return 1;
        }
        arg_ind++;
    }
    if (strlen(shortcut) == 0 || strlen(message) == 0) {
        printerr(INVALID_USAGE);
        return 1;
    }
    if ( !replace_shortcut(shortcut, message) ) {
        printf(SHORTCUT_NOT_FOUND, shortcut);
        return 1;
    } else {
        printf(SHORTCUT_REPLACE_SUCCESS, shortcut);
        return 0;
    }
}

int exec_remove(int argc, char *argv[])
{
    if (argc < 3) {
        printerr(INVALID_USAGE);
        return 1;
    } 
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "s") == 0 ) {
            if (argc < 4) {
                printerr(EXPECTED_SHORTCUT);
                return 1;
            } if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    } else {
        printerr(OPTION_REQUIRED);
        return 1;
    }
    char* shortcut = argv[3];
    if ( remove_shortcut(shortcut) ) {
        printf(SHORTCUT_REMOVE_SUCCESS, shortcut);
        return 0;
    } else {
        printf(SHORTCUT_NOT_FOUND, shortcut);
        return 1;
    }
}

int exec_log(int argc, char *argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }
    if (argc == 2) { // Log all
        print_all_commits();
        return 0;
    }
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {

        if ( strcmp(argv[arg_ind] + 1, "n") == 0 ) {
            if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            } 
            arg_ind++;
            int n = atoi(argv[arg_ind]);
            if (n < 1) {
                printerr(INVALID_USAGE);
                return 1;
            }
            print_n_last_commits(n);
            return 0;
        } else if ( strcmp(argv[arg_ind] + 1, "branch") == 0 ) {
            if ( argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            print_branch_commits(argv[arg_ind]);
            return 0;
        } else if ( strcmp(argv[arg_ind] + 1, "author") == 0 ) {
            if ( argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            print_author_commits(argv[arg_ind]);
            return 0;
        } else if ( strcmp(argv[arg_ind] + 1, "since") == 0 ) {
            if ( argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            if ( !print_commits_since(argv[arg_ind]) ) {
                printerr(INVALID_TIME_FORMAT);
                return 1;
            }
            return 0;

        } else if ( strcmp(argv[arg_ind] + 1, "before") == 0 ) {
            if ( argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            if ( !print_commits_before(argv[arg_ind]) ) {
                printerr(INVALID_TIME_FORMAT);
                return 1;
            }
            return 0;

        } else if ( strcmp(argv[arg_ind] + 1, "search") == 0 ) {
            if ( argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            // TOOD: multiple words support
            if ( !print_message_commits(argv[arg_ind]) ) {
                printerr(NO_MESSAGES_FOUND);
                return 1;
            }
            return 0;

        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    } else {
        printerr(INVALID_USAGE);
        return 1;
    }
}

int exec_branch(int argc, char* argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }
    if (argc == 2) {
        show_all_branches();
        return 0;
    } else if (argc == 3) {
        if ( detached_head(0) ) {
            printerr(BRANCH_HEAD_DETACHED);
            return 1;
        }
        if ( create_new_branch(argv[2]) == 1 ) {
            printerr(BRANCH_EXISTS);
            return 1;
        } else {
            printf(BRANCH_CREATE_SUCCESS, argv[2], get_head_id());
            return 0;
        }
    } else {
        printerr(INVALID_USAGE);
        return 1;
    }
}

int exec_checkout(int argc, char* argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }
    if (argc == 3) {
        
        if ( !is_wt_unchanged() ) {
            bool stop = true;
            if ( strcmp(argv[2], "HEAD") == 0 ) { // Prompt user to make sure
                char input[20];
                printf(CHECKOUT_HEAD_WARNING);
                scanf("%s", input);
                if (input[0] == 'Y' || input[0] == 'y') {
                    stop = false;
                } else {
                    printerr(CHECKOUT_CANCELLED);
                    return 1;
                }
            }
            if (stop) {
                printerr(CHECKOUT_UNCHANGED_ERR);
                return 1;
            }
        }
        int result = checkout(argv[2]);
        if (result < 0) {
            printerr(CHECKOUT_FAILURE);
            return 1;
        } else {
            printf(CHECKOUT_SUCCESS, argv[2]);
        }
        if (detached_head(0)) {
            printf(DETACHED_WARNING);
        }
        return 0;
    } else {
        printerr(INVALID_USAGE);
        return 1;
    }
}

int exec_revert(int argc, char* argv[])
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    }

    if (argc < 3) {
        printerr(FEW_ARGUMENTS);
        return 1;
    }
    if ( detached_head(0) ) {
        printerr(REVERT_HEAD_DETACHED);
        return 1;
    }
    int arg_ind = 2;
    char message[COMMIT_MESSAGE_MAX] = "";
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "m") == 0 ) {
            if (argc != 5) {
                printerr(INVALID_USAGE);
                return 1;
            }
            arg_ind++;
            strcpy(message, argv[arg_ind]);
        } else if ( strcmp(argv[arg_ind] + 1, "n") == 0 ) {
            if (argc > 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            State* result = NULL;
            if (argc == 3) {
                char id_str[50];
                sprintf(id_str, "%x", get_head_id());
                result = revert_no_commit(id_str);
            } else {
                arg_ind++;
                result = revert_no_commit(argv[arg_ind]);
            }
            if ( result == NULL ) {
                printerr(REVERT_INVALID_ID);
                return 1;
            } else {
                printf(REVERT_SUCCESS_N, argv[arg_ind]);
                return 0;
            }
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
        arg_ind++;
    }
    if (argc != arg_ind + 1) {
        printerr(INVALID_USAGE);
        return 1;
    }
    int result = revert(argv[arg_ind], message);
    if ( result == 1 ) {
        printerr(REVERT_INVALID_ID);
        return 1;
    } else if ( result == 0 ) {
        printf(REVERT_SUCCESS, argv[arg_ind]);
        return 0;
    } else if ( result == 2 ) {
        printf(REVERT_IS_SAME);
        return 0;
    }
}

int exec_diff(int argc, char* argv[]) 
{
    if ( !is_in_repo() ) {
        printerr(NOT_REPO);
        return 1;
    } if (argc < 3) {
        printerr(INVALID_USAGE);
        return 1;
    }
    State* commit = NULL;
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "f") == 0 ) {
            if (argc < 5) {
                printerr(INVALID_USAGE);
                return 1;
            }
            char* file1_name = argv[3];
            char* file2_name = argv[4];
            int s1 = 0, e1 = 0;
            int s2 = 0, e2 = 0;
            if (argc > 5) {
                int arg_ind = 5;
                for (int i = 0; i < 2; i++) {
                    if (argv[arg_ind][0] == '-') {
                        if ( strcmp(argv[arg_ind] + 1, "line1") == 0 ) {
                            arg_ind++;
                            if ( sscanf(argv[arg_ind], "%d-%d", &s1, &e1) != 2 || s1 < 0 || e1 < 0) {
                                printerr(INVALID_LINE_NUMBERS);
                                return 1;
                            }

                        } else if ( strcmp(argv[arg_ind] + 1, "line2") == 0 ) {
                            arg_ind++;
                            if ( sscanf(argv[arg_ind], "%d-%d", &s2, &e2) != 2 || s2 < 0 || e2 < 0) {
                                printerr(INVALID_LINE_NUMBERS);
                                return 1;
                            }
                        } else {
                            printerr(INVALID_OPTION);
                            return 1;
                        }
                    } else {
                        printerr(INVALID_USAGE);
                        return 1;
                    }
                    arg_ind++;
                }
            }
            FILE* file1 = fopen(file1_name, "r");
            FILE* file2 = fopen(file2_name, "r");
            if (file1 == NULL || file2 == NULL) {
                printerr(FILES_NOT_FOUND);
                return 1;
            }
            
            print_differences(file1, file2, s1, e1, s2, e2);

        } else if ( strcmp(argv[arg_ind] + 1, "c") == 0 ) {
            if (argc < 4) {
                printerr(EXPECTED_SHORTCUT);
                return 1;
            } if (argc != 4) {
                printerr(INVALID_USAGE);
                return 1;
            }
            char message[COMMIT_MESSAGE_MAX + 1];
            if ( !get_message_from_shortcut(message, argv[3]) ) {
                printf(SHORTCUT_NOT_FOUND, argv[3]);
                return 1;
            }
            commit = do_a_commit(message);

        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    } else {
        printerr(OPTION_REQUIRED);
        return 1;
    }
    return 0;
}

int exec_merge(int argc, char* argv[])
{
    if (argc < 3) {
        printerr(INVALID_USAGE);
        return 1;
    } 
    int arg_ind = 2;
    if (argv[arg_ind][0] == '-') {
        if ( strcmp(argv[arg_ind] + 1, "b") == 0 ) {
            if (argc != 5) {
                printerr(INVALID_USAGE);
                return 1;
            }
        } else {
            printerr(INVALID_OPTION);
            return 1;
        }
    } else {
        printerr(OPTION_REQUIRED);
        return 1;
    }
    if ( !is_wt_unchanged() ) {
        char input[20];
        printf(CHECKOUT_HEAD_WARNING);
        scanf("%s", input);
        if (input[0] == 'Y' || input[0] == 'y') {
            ;
        } else {
            printerr(MERGE_CANCELLED);
            return 1;
        }
    }
    // DOING THE MERGE
    int result = merge(argv[3], argv[4]);
    switch (result)
    {
    case 0:
        printf(MERGE_SUCCESS, argv[3], argv[4]);
        return 0;
        break;
    case 1:
        printerr(BRANCH_NOT_FOUND);
        return 1;
        break;
    case 2:
        printerr(CONFLICT_FOUND);
        return 1;
        break;
    default:
        break;
    }
}
#endif