#ifndef DATA_H // Include guard
#define DATA_H

#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utility.h"
#include "file-util.h"
#include "struct-state.h"

// Function prototypes because of bad source management
int make_initial_empty_stage();

// search in current dir and parents
char* find_root_path()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    for (;;) {
        if ( file_exists(LITDIR_NAME, true) ) {
            char* result = (char*) malloc(PATH_MAX * sizeof(char));
            getcwd(result, PATH_MAX);
            return result;
        }
        if ( is_root() ) break;
        chdir("..");
    }
    chdir(original_path);
    return NULL;
}

char* find_repo_data()
{
    char* result = find_root_path();
    if (result == NULL) return NULL;
    strcat(result, "\\"); strcat(result, LITDIR_NAME);
    return result;
}

bool is_in_repo()
{
    return (find_repo_data() != NULL);
}

// Note that it does not use is_in_repo
char* get_config_path(bool is_global) 
{
    char* confpath;
    if (is_global) {
        confpath = (char*) malloc(PATH_MAX * sizeof(char));
        strcpy(confpath, CONFIG_PATH);
    } else {
        confpath = find_repo_data();
    }
    strcat(confpath, "\\config");
    return confpath;
}

int create_config_global()
{
    char* folder_path = get_config_path(1);
    if ( !file_exists(folder_path, true) ) {
        mkdir(folder_path);
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path)); // Keep current path
        chdir(folder_path);
        fclose(fopen("alias.txt", "w")); // Create empty alias file
        FILE* f_user = fopen("user.txt", "w");
        fprintf(f_user, "Default\ndefault@user.info\n");
        fclose(f_user);
        chdir(original_path);
        return 0;
    }
    return 1;
}

// Configures user.name / note that it assumes that all files already exist with data in them
int config_user(char* setting, char* data, int is_global) 
{
    if ( ! (strcmp(setting, "name") == 0 || strcmp(setting, "email") == 0) ) return 1; // Error code

    char* filename = get_config_path(is_global);
    strcat(filename, "\\user.txt");
    FILE* f_user = fopen(filename, "r");
    // Keeping the other one
    char *email = (char*) malloc(EMAIL_MAX * sizeof(char));
    char *name = (char*) malloc(USERNAME_MAX * sizeof(char));
    fgets(name, USERNAME_MAX, f_user);
    fgets(email, EMAIL_MAX, f_user);
    name[strcspn(name, "\n\r")] = '\0'; // fgets will include line break at the end of the string
    email[strcspn(email, "\n\r")] = '\0';
    fclose(f_user);

    // Settling what to keep
    f_user = fopen(filename, "w");
    if (strcmp(setting, "name") == 0) {
        name = data;
    } else if (strcmp(setting, "email") == 0) {
        email = data;
    }

    // Writing
    fprintf(f_user, "%s\n%s\n", name, email);
    fclose(f_user);
    free(email); free(name);
    return 0;
}

int get_author_name(char* name)
{

    char* filename = get_config_path(false);
    strcat(filename, "\\user.txt");
    FILE* f_user = fopen(filename, "r");
    // Keeping the other one
    bool undefined_local = false;
    if ( fgets(name, USERNAME_MAX, f_user) == NULL ) undefined_local = true;
    if ( undefined_local ) {
        char* filename_glob = get_config_path(true);
        strcat(filename_glob, "\\user.txt");
        FILE* f_user_glob = fopen(filename, "r");
        fgets(name, USERNAME_MAX, f_user_glob);
    }
    name[strcspn(name, "\n\r")] = '\0';
    fclose(f_user);
    return undefined_local;
}

int get_author_email(char* email)
{
    char* filename = get_config_path(false);
    strcat(filename, "\\user.txt");
    FILE* f_user = fopen(filename, "r");
    // Keeping the other one
    bool undefined_local = false;
    if ( fgets(email, EMAIL_MAX, f_user) == NULL ) undefined_local = true;
    if ( fgets(email, EMAIL_MAX, f_user) == NULL ) undefined_local = true; // (email is on the second line)
    if ( undefined_local ) {
        char* filename_glob = get_config_path(true);
        strcat(filename_glob, "\\user.txt");
        FILE* f_user_glob = fopen(filename, "r");
        fgets(email, EMAIL_MAX, f_user_glob);
        fgets(email, EMAIL_MAX, f_user_glob);
    }
    email[strcspn(email, "\n\r")] = '\0';
    fclose(f_user);
    return undefined_local;
}

int config_alias(char* alias, char* command, int is_global)
{
    if (strlen(alias) < 1) return 1; // Validation of alias

    char command_copy[COMMAND_MAX];
    strcpy(command_copy, command);
    char* token = strtok(command_copy, " ");
    if (strcmp(token, "lit") != 0) return 1;
    token = strtok(NULL, " ");
    if (token == NULL || search_str(COMMANDS, token, N_COMMANDS) == -1) return 1; // Checking the validity of command
    // Opening config
    char* filename = get_config_path(is_global);
    strcat(filename, "\\alias.txt");
    FILE* f_alias = fopen(filename, "a");
    // Writing
    fprintf(f_alias, "%s %s\n", alias, command);
    fclose(f_alias);
    return 0;
}

char* get_alias(char* alias)
{
    if (is_in_repo()) {
        char* filename = get_config_path(0);
        strcat(filename, "\\alias.txt");
        FILE* f_alias_repo = fopen(filename, "r");
        // Getting lines in reverse order
        char lines[MAX_ALIASES][2][COMMAND_MAX]; // lines[n][0]: alias name // lines[n][1]: command
        int n_lines = 0;
        while ( fscanf(f_alias_repo, "%s ", lines[n_lines][0]) != EOF ) { // Getting the alias name
            fgets(lines[n_lines][1], COMMAND_MAX, f_alias_repo); // Getting the command
            lines[n_lines][1][strcspn(lines[n_lines][1], "\n")] = '\0'; // Removing the trailing line break
            n_lines++;
        }
        fclose(f_alias_repo);
        for (int i = n_lines - 1; i >= 0; i--) {
            if ( strcmp(lines[i][0], alias) == 0 ) {
                char* output = (char*) malloc(COMMAND_MAX * sizeof(char));
                strcpy(output, lines[i][1]);
                return output;
            }
        }
    }
    char* filename = get_config_path(1);
    strcat(filename, "\\alias.txt");
    FILE* f_alias_global = fopen(filename, "r");
    // Getting lines in reverse order
    char lines[MAX_ALIASES][2][COMMAND_MAX]; // lines[n][0]: alias name // lines[n][1]: command
    int n_lines = 0;
    while ( fscanf(f_alias_global, "%s ", lines[n_lines][0]) != EOF ) { // Getting the alias name
        fgets(lines[n_lines][1], COMMAND_MAX, f_alias_global); // Getting the command
        lines[n_lines][1][strcspn(lines[n_lines][1], "\n")] = '\0'; // Removing the trailing line break
        n_lines++;
    }
    fclose(f_alias_global);
    for (int i = n_lines - 1; i >= 0; i--) {
        if ( strcmp(lines[i][0], alias) == 0 ) {
            char* output = (char*) malloc(COMMAND_MAX * sizeof(char));
            strcpy(output, lines[i][1]);
            return output;
        }
    }
    return NULL;
}
// * This function should be further developed in respect to other features that are yet to be added
int initialize_repo() 
{
    mkdir(LITDIR_NAME);
    system("attrib +h .lit");
    // char original_path[PATH_MAX];
    // getcwd(original_path, sizeof(original_path));
    chdir(LITDIR_NAME);
    mkdir("config");
    chdir("config");

    fclose(fopen("alias.txt", "w")); // Create empty alias file
    // User info file
    FILE* f_user = fopen("user.txt", "w");
    fprintf(f_user, "Default\ndefault@user.info\n");
    fclose(f_user);
    chdir("..");
    // States
    mkdir("states"); chdir("states");
    mkdir("stage");
    chdir("stage");
    // make_initial_empty_stage();
    chdir("..");

    mkdir("commits");
    // commits
    // create_root_commit();
    chdir("..\\.."); // Go back to the original path
    return 0;
}

#endif // DATA_H