#include <unistd.h>
#include <limits.h>
#include "utility.h"
#include "file-util.h"

char* find_repo_data()
{
    // search in current dir and parents
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    for (;;) {
        if ( file_exists(LITDIR_NAME, 1) ) {
            char* result = (char*) malloc(PATH_MAX * sizeof(char));
            getcwd(result, PATH_MAX);
            strcat(result, "\\"); strcat(result, LITDIR_NAME);
            chdir(original_path);
            return result;
        }
        if ( is_root() ) break;
        chdir("..");
    }
    return NULL;
}

int is_in_repo()
{
    return (find_repo_data() != NULL);
}

char* get_config_path(int is_global)  // Note that it does not use is_in_repo
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

int config_user(char* setting, char* data, int is_global) // Configures user.name / note that it assumes that all files already exist with data in them
{
    if ( ! (strcmp(setting, "name") == 0 || strcmp(setting, "email") == 0) ) return 1; // Error code

    char* filename = get_config_path(is_global);
    strcat(filename, "\\user.txt");
    FILE* f_user = fopen(filename, "r");
    // Keeping the other one
    char *email = (char*) malloc(CONFIG_MAX * sizeof(char));
    char *name = (char*) malloc(CONFIG_MAX * sizeof(char));
    fgets(name, CONFIG_MAX, f_user);
    fgets(email, CONFIG_MAX, f_user);
    name[strcspn(name, "\n")] = '\0'; // fgets will include line break at the end of the string
    email[strcspn(email, "\n")] = '\0';
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
    return 0;
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
                char* output = (char*) malloc(strlen(lines[i][1]) * sizeof(char));
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
            char* output = (char*) malloc(strlen(lines[i][1]) * sizeof(char));
            strcpy(output, lines[i][1]);
            return output;
        }
    }
    return NULL;
}
