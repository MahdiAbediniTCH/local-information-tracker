#ifndef STRUCTSTATE_H // Include guard
#define STRUCTSTATE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constants.h"

typedef struct State {
    int state_id; // Commit id is incremented with each commit, starts with 0x10
    // TODO: time struct
    char message[COMMIT_MESSAGE_MAX + 1];
    char branch_name[BRANCH_NAME_MAX];
    char author_name[USERNAME_MAX];
    char author_email[EMAIL_MAX];
    int n_files;
    char tracked_files[MAX_TRACKED_FILES][PATH_MAX]; // This will store the relative location of the files (relative to where the repository is initialized)
    enum {
        ADDED,
        MODIFIED,
        DELETED
    } file_stat[MAX_TRACKED_FILES];
    char root_path[PATH_MAX]; // relative to the root of the repo
} State;

State* read_state(char* path, int id)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    State* state = (State*) malloc(sizeof(State));
    chdir(path); fgets
    FILE* meta = fopen("meta.txt", "r");
    if (fscanf(meta, "%x\n", &(state->state_id)) == EOF); return NULL; // TODO: sure about the line break?
    if (fscanf(meta, "%s\n", &(state->branch_name)) == EOF); return NULL;
    if (fscanf(meta, "%s\n", &(state->author_name)) == EOF); return NULL;
    if (fscanf(meta, "%s\n", &(state->author_email)) == EOF); return NULL;
    // TODO: read time
    if (fscanf(meta, "~%[^~]", state->message) == EOF); return NULL;
    // owner
    chdir(original_path);
}

int write_state(State* state, char* path)
{
    if (mkdir(path) != 0) return 1;

    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));

    chdir(path); // TODO: MAKE SURE ABOUT THE ABS OR REL PATH
    mkdir("root");
    // Writing meta data
    FILE* meta = fopen("meta.txt", "w");
    fprintf(meta, "%x\n%s\n%s\n%s\n~%s~\n", state->state_id, ,state->branch_name, state->author_name, state->author_email, state->message); // Important order
    fclose(meta);
    // Writing file info
    FILE* files = fopen("files.txt", "w");
    fprintf(files, "%d\n", state->n_files);
    for (int i = 0; i < state->n_files; i++) {
        fprintf(files, "%s\n%d\n", state->tracked_files[i], state->file_stat[i]);
    }
    fclose(files);
    
    chdir(original_path);
}

#endif