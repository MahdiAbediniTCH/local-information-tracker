#ifndef STRUCTSTATE_H // Include guard
#define STRUCTSTATE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "constants.h"

enum Filestat{
    S_ADDED,
    S_MODIFIED,
    S_DELETED,
    S_UNCHANGED
};

typedef struct State {
    int state_id; // Commit id is incremented with each commit, starts with 0x10
    time_t time_created;
    char message[COMMIT_MESSAGE_MAX + 1];
    char branch_name[BRANCH_NAME_MAX];
    char author_name[USERNAME_MAX];
    char author_email[EMAIL_MAX];
    int n_files;
    char tracked_files[MAX_TRACKED_FILES][PATH_MAX]; // This will store the relative location of the files (relative to where the repository is initialized)
    enum Filestat file_stat[MAX_TRACKED_FILES];
    char data_dir[PATH_MAX]; // relative to the root of the repo
} State;

State* initialize_state(int id, char* message, char* branch, char* name, char* email, char* data_dir) // TODO: decide about the arguments
{
    State* state = (State*) malloc(sizeof(State));
    state->state_id = id;
    state->time_created = time(NULL);
    strcpy(state->message, message);
    strcpy(state->branch_name, branch);
    strcpy(state->author_name, name);
    strcpy(state->author_email, email);
    state->n_files = 0;
    strcpy(state->data_dir, data_dir);
    return state;
}

State* read_state(char* path)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    State* state = (State*) malloc(sizeof(State));
    state->n_files = 0;
    // Going to the root of the repository
    chdir(find_root_path());
    chdir(path);
    FILE* meta = fopen("meta.txt", "r");
    // ID
    if (fscanf(meta, "%x\n", &(state->state_id)) == EOF) return NULL; // TODO: sure about the line break?
    // Branch
    if (fgets(state->branch_name, BRANCH_NAME_MAX, meta) == NULL) return NULL;
    state->branch_name[strcspn(state->branch_name, "\n\r")] = '\0';
    // Author name
    if (fgets(state->author_name, BRANCH_NAME_MAX, meta) == NULL) return NULL;
    state->author_name[strcspn(state->author_name, "\n\r")] = '\0';
    // Author email
    if (fgets(state->author_email, BRANCH_NAME_MAX, meta) == NULL) return NULL;
    state->author_email[strcspn(state->author_email, "\n\r")] = '\0';
    // Commit message
    if (fscanf(meta, "~%[^~]", state->message) == EOF) return NULL;
    fclose(meta);
    // Files
    FILE* tracked_f = fopen("files.txt", "r");
    char line[PATH_MAX];
    while ( fgets(line, PATH_MAX, tracked_f) != NULL ) {
        if (strlen(line) < 2) break;
        char* sep = strchr(line, '|');
        sep[0] = '\0';
        strcpy(state->tracked_files[state->n_files], line);
        state->file_stat[state->n_files] = atoi(sep + 1); // WILL IT NOT WORK?
        state->n_files++;
    }
    fclose(tracked_f);
    // Datetime
    FILE* time_file = fopen("time.bin", "rb");
    fread(&(state->time_created), sizeof(time_t), 1, time_file);
    fclose(time_file);
    // Root path
    strcpy(state->data_dir, path); // * root path will be the path of the commit folder and not the actual root folder
    chdir(original_path);
    return state;
}

int write_state(State* state, char* path)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));

    // Going to the root of the repository
    chdir(find_root_path());

    mkdir(path); // Creates the folder if it already doesn't exist
    chdir(path); // TODO: MAKE SURE ABOUT THE ABS OR REL PATH
    mkdir("root");
    // Writing meta data
    FILE* meta = fopen("meta.txt", "w");
    fprintf(meta, "%x\n%s\n%s\n%s\n~%s~\n", state->state_id, state->branch_name, state->author_name, state->author_email, state->message); // Important order
    fclose(meta);
    // Writing file info
    FILE* files = fopen("files.txt", "w");
    // fprintf(files, "%d\n", state->n_files);
    for (int i = 0; i < state->n_files; i++) {
        fprintf(files, "%s|%d\n", state->tracked_files[i], state->file_stat[i]);
    }
    fclose(files);
    // Writing time in binary
    FILE* time_file = fopen("time.bin", "wb");
    fwrite(&(state->time_created), sizeof(time_t), 1, time_file);
    fclose(time_file);
    // Going back to the original path
    chdir(original_path);
    return 0;
}

int update_all_state_files(State* state)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());
    // TODO: check if files exist in root dir???
    for (int i = 0; i < state->n_files; i++) {
        char* filename = state->tracked_files[i];
        if (state->file_stat[i] == S_DELETED) {
            char file_path[PATH_MAX];
            sprintf(file_path, "%s\\root\\%s", state->data_dir, filename);
            if ( remove(file_path) == -1 ) {
                printf("DBG: THIS FILE NOT FOUND\n");
            }
            continue;
        }
        // Temporarily changing the filename to make the folders
        char* last_sep = strrchr(filename, '\\');
        *last_sep = '\0';
        char system_command[PATH_MAX];
        if ( strlen(filename) > 0 ) {
            sprintf(system_command, "mkdir %s", filename);
            system(system_command);
        }
        *last_sep = '\\';

        sprintf(system_command, "copy /Y \"%s\" \"%s\\root\\%s\"", filename, state->data_dir, state->tracked_files[i]);
        printf("DEBUG: %s\n", system_command);
        system(system_command);
    }
    chdir(original_path);
}

// Returns the index, returns -1 if not found
int find_state_file(State* state, char* filename) // Relative to root without .
{
    // TODO: If it's necessary, make sure to change filename to the correct relative form
    for (int i = 0; i < state->n_files; i++) {
        if ( strcmp(state->tracked_files[i], filename) == 0 ) {
            return i;
        }
    }
    return -1;
}

// Returns 0 upon success, returns 1 if the file is already added
int add_state_file(State* state, char* filename, enum Filestat file_stat) // Relative to root
{
    if ( find_state_file(state, filename) > -1 ) return 1;
    strcpy(state->tracked_files[state->n_files], filename);
    state->file_stat[state->n_files] = file_stat;
    state->n_files++;
    return 0;
}
int add_and_copy_file(State* state, char* filename) // Relative to root
{

}
#endif