#ifndef ACTIONHISTORY_H
#define ACTIONHISTORY_H

#include "constants.h"
#include "file-util.h"
#include "data.h"

int store_last_add_cmd(int argc, char* argv[])
{
    char original_path[PATH_MAX];
    char path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    getcwd(path, PATH_MAX);
    chdir(find_root_path());
    //
    FILE* file = fopen(".lit\\last_add.txt", "w");
    fprintf(file, "lit ");
    for (int i = 1; i < argc; i++) {
        fprintf(file, "%s ", argv[i]);
    }
    relative_path(path, find_root_path());
    fprintf(file, "\n%s", path);
    fclose(file);
    //
    chdir(original_path);
    return 0;
}

int execute_last_add_cmd()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());
    //
    FILE* file = fopen(".lit\\last_add.txt", "r");
    if ( file == NULL ) {
        chdir(original_path);
        return 1;
    }
    char cmd[COMMAND_MAX] = "";
    char path[PATH_MAX] = "";
    fgets(cmd, COMMAND_MAX, file);
    if ( strlen(cmd) < 7 ) {
        chdir(original_path);
        return 1;
    }
    fgets(path, PATH_MAX, file);
    chdir(path);
    system(cmd);
    //
    chdir(original_path);
    return 0;
}

int unstage_last_add()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());
    //
    FILE* file = fopen(".lit\\last_add.txt", "r");
    if ( file == NULL ) {
        chdir(original_path);
        return 1;
    }
    char cmd[COMMAND_MAX] = "";
    char reset_cmd[COMMAND_MAX] = "";
    char path[PATH_MAX] = "";
    fgets(cmd, COMMAND_MAX, file);
    if ( strlen(cmd) < 7 ) {
        chdir(original_path);
        return 1;
    }
    sprintf(reset_cmd, "lit reset %s", cmd + 8);
    fgets(path, PATH_MAX, file);
    chdir(path);
    system(reset_cmd);
    //
    chdir(original_path);
    return 0;
}

#endif