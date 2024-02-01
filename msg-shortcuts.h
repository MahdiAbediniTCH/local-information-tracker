#ifndef SHORTCUT_H
#define SHORTCUT_H

#include "constants.h"
#include "file-util.h"
#include "data.h"
#include <stdbool.h>

bool get_message_from_shortcut(char* message, const char* shortcut)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(get_config_path(true));
    //
    FILE* file = fopen("message-shortcuts.txt", "r");
    if (file == NULL) return false;
    char shrt[SHORTCUT_MAX], msg[COMMIT_MESSAGE_MAX];
    while ( fscanf(file, "%[^~]", shrt) != EOF ) {
        fscanf(file, "~%[^~]~\n", msg);
        if ( strcmp(shrt, shortcut) == 0 ) {
            fclose(file);
            chdir(original_path);
            if (message != NULL)
                strcpy(message, msg);
            return true;
        }
    }
    fclose(file);
    //
    chdir(original_path);
    return false;
}

bool add_new_shortcut(char* shortcut, char* message)
{
    if ( get_message_from_shortcut(NULL, shortcut) )
        return false;
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(get_config_path(true));
    //
    FILE* file = fopen("message-shortcuts.txt", "a");
    fprintf(file, "%s~%s~\n", shortcut, message);
    fclose(file);
    //
    chdir(original_path);
    return true;
}

// bool replace_();

#endif