#ifndef STRUCTSTATE_H // Include guard
#define STRUCTSTATE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "data.h"
#include "constants.h"

// Function prototypes because of bad source management
char* find_root_path();
int get_author_name(char*);
int get_author_email(char*);


enum Filestat{
    S_ADDED,
    S_MODIFIED,
    S_DELETED,
    S_UNCHANGED
};

typedef struct State {
    int state_id; // Commit id is incremented with each commit, starts with 0xa0
    int parent_id;
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

State* initialize_state(int id, int parent_id, char* message, char* branch, char* name, char* email, char* data_dir) // TODO: decide about the arguments
{
    State* state = (State*) malloc(sizeof(State));
    state->state_id = id;
    state->parent_id = parent_id;
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
    if (meta == NULL) {
        chdir(original_path);
        return NULL;
    }
    // ID
    if (fscanf(meta, "%x\n", &(state->state_id)) == EOF) {
        chdir(original_path);
        return NULL;
    }
    // Parent ID
    if (fscanf(meta, "%x\n", &(state->parent_id)) == EOF) {
        chdir(original_path);
        return NULL;
    }
    // Branch
    if (fgets(state->branch_name, BRANCH_NAME_MAX, meta) == NULL) {
        chdir(original_path);
        return NULL;
    }
    state->branch_name[strcspn(state->branch_name, "\n\r")] = '\0';
    // Author name
    if (fgets(state->author_name, BRANCH_NAME_MAX, meta) == NULL) {
        chdir(original_path);
        return NULL;
    }
    state->author_name[strcspn(state->author_name, "\n\r")] = '\0';
    // Author email
    if (fgets(state->author_email, BRANCH_NAME_MAX, meta) == NULL) {
        chdir(original_path);
        return NULL;
    }
    state->author_email[strcspn(state->author_email, "\n\r")] = '\0';
    // Commit message
    if (fscanf(meta, "~%[^~]", state->message) == EOF) {
        chdir(original_path);
        return NULL;
    }
    fclose(meta);
    // Files
    FILE* tracked_f = fopen("files.txt", "r");
    char line[PATH_MAX];
    while ( fgets(line, PATH_MAX, tracked_f) != NULL ) {
        if (strlen(line) < 2) break;
        char* sep = strchr(line, '|');
        sep[0] = '\0';
        strcpy(state->tracked_files[state->n_files], line);
        state->file_stat[state->n_files] = atoi(sep + 1);
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
    fprintf(meta, "%x\n%x\n%s\n%s\n%s\n~%s~\n", state->state_id, state->parent_id, state->branch_name, state->author_name, state->author_email, state->message); // Important order
    fclose(meta);
    // Writing file info
    FILE* files = fopen("files.txt", "w");
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
            delete_state_file(state, filename);
            continue;
        }
        // Temporarily changing the filename to make the folders
        char system_command[PATH_MAX];
        char* last_sep = strrchr(filename, '\\');
        if ( last_sep != NULL ) {
            *last_sep = '\0';
            if ( strlen(filename) > 0 ) {
                sprintf(system_command, "mkdir \"%s\\root\\%s\" >NUL 2>NUL", state->data_dir, filename);
                // TEMPCOMMENT: printf("MKDIR: %s\n", system_command);
                system(system_command);
            }
            *last_sep = '\\';
        }

        sprintf(system_command, "copy /Y \"%s\" \"%s\\root\\%s\" >NUL 2>NUL", filename, state->data_dir, filename);
        // TEMPCOMMENT: printf("DEBUG: %s\n", system_command);
        system(system_command);
    }
    chdir(original_path);
    return 0;
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
    int file_ind = find_state_file(state, filename);
    if ( file_ind > -1 ) {
        // Update state
        state->file_stat[file_ind] = file_stat;
        return 1;
    }
    strcpy(state->tracked_files[state->n_files], filename);
    state->file_stat[state->n_files] = file_stat;
    state->n_files++;
    return 0;
}
// Returns 1 if the file is not found, 2 if deleted
int copy_state_file_from_wd(State* state, char* filename)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());

    int ind = find_state_file(state, filename);
    if (ind == -1) return 1;
    if (state->file_stat[ind] == S_DELETED) {
        char file_path[PATH_MAX];
        sprintf(file_path, "%s\\root\\%s", state->data_dir, filename);
        if ( remove(file_path) == -1 ) {
            // TEMPCOMMENT: printf("DBG: THIS FILE NOT FOUND\n");
        }
        return 2;
    }
    // Temporarily changing the filename to make the folders
    char system_command[PATH_MAX];
    char* last_sep = strrchr(filename, '\\');
    if ( last_sep != NULL ) {
        *last_sep = '\0';
        if ( strlen(filename) > 0 ) {
            sprintf(system_command, "mkdir \"%s\\root\\%s\" >NUL 2>NUL", state->data_dir, filename);
            // TEMPCOMMENT: printf("MKDIR: %s\n", system_command);
            system(system_command);
        }
        *last_sep = '\\';
    }

    sprintf(system_command, "copy /Y \"%s\" \"%s\\root\\%s\" >NUL 2>NUL", filename, state->data_dir, filename);
    // TEMPCOMMENT: printf("DEBUG: %s\n", system_command);
    system(system_command);

    chdir(original_path);
    return 0;
}
// Returns 1 upon success, 0 if the file is already deleted
int delete_state_file(const State* state, char* relpath)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());

    char file_path[PATH_MAX];
    sprintf(file_path, "%s\\root\\%s", state->data_dir, relpath);
    if ( remove(file_path) == -1 ) {
        chdir(original_path);
        return 1;
    }
    chdir(original_path);
    return 0;
}

// Returns the deleted index
int remove_file_from_state_data(State* state, char* relpath)
{
    int ind = find_state_file(state, relpath);
    if (ind != -1) {
        state->n_files--;
        memmove(state->tracked_files + ind, state->tracked_files + ind + 1, sizeof(state->tracked_files[0]) * (state->n_files - ind));
        memmove(state->file_stat + ind, state->file_stat + ind + 1, sizeof(state->file_stat[0]) * (state->n_files - ind));
    }
    return ind;
}

int get_state_data_dir(char* datadir, int id)
{
    // Stage
    if (id == STAGE_ID) {
        strcpy(datadir, ".lit\\states\\stage");
        return 1; // meaning stage
    // Commits
    } else if (id >= 0xa0 || id == ROOT_ID) {
        sprintf(datadir, ".lit\\states\\commits\\%x", id);
        return 0;
    }
    // TODO: Stash
    return -1;
}

State* inherit_state(State* parent, int id)
{
    State* state = (State*) malloc(sizeof(State));
    state->state_id = id;
    state->parent_id = parent->state_id;
    get_author_name(state->author_name);
    get_author_email(state->author_email);
    strcpy(state->branch_name, parent->branch_name);
    state->n_files = parent->n_files;
    memcpy(state->tracked_files, parent->tracked_files, sizeof(parent->tracked_files));
    memcpy(state->file_stat, parent->file_stat, sizeof(parent->file_stat));
    state->time_created = time(NULL);
    state->message[0] = '\0';
    get_state_data_dir(state->data_dir, id);
    return state;
}

State* get_state_by_id(int id)
{
    char datadir[PATH_MAX];
    get_state_data_dir(datadir, id);
    State* state = read_state(datadir);
    return state;
}

FILE* open_state_file(State* state, char* relpath)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());
    //
    char path[PATH_MAX];
    sprintf(path, "%s\\root\\%s", state->data_dir, relpath);
    FILE* file = fopen(path, "r");
    //
    chdir(original_path);
    return file;
}
// Doesn't validate anything
int copy_only_file(const State* dest, const State* source, char* relpath)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());

    // Temporarily changing the filename to make the folders
    char system_command[PATH_MAX];
    char* last_sep = strrchr(relpath, '\\');
    if ( last_sep != NULL ) {
        *last_sep = '\0';
        if ( strlen(relpath) > 0 ) {
            sprintf(system_command, "mkdir \"%s\\root\\%s\" >NUL 2>NUL", dest->data_dir, relpath);
            // TEMPCOMMENT: printf("MKDIR: %s\n", system_command);
            system(system_command);
        }
        *last_sep = '\\';
    }

    sprintf(system_command, "copy /Y \"%s\\root\\%s\" \"%s\\root\\%s\" >NUL 2>NUL", source->data_dir, relpath, dest->data_dir, relpath);
    // TEMPCOMMENT: printf("DEBUG: %s\n", system_command);
    system(system_command);

    chdir(original_path);
    return 0;
}

int copy_all_files(State* to, State* from)
{
    for (int i = 0; i < from->n_files; i++) {
        if (from->file_stat[i] == S_DELETED) continue;
        copy_only_file(to, from, from->tracked_files[i]);
    }
    return 0;
}

// Also copies the file, 1: deleted the file to match, 0: matched the file data
int copy_file_attributes(State* dest, const State* source, char* relpath, bool is_stage)
{
    int ind_source = find_state_file(source, relpath);
    int ind_dest = find_state_file(dest, relpath);
    if (ind_source == -1) {
        delete_state_file(dest, relpath);
        remove_file_from_state_data(dest, relpath);
        return 1;
    }
    if (ind_dest == -1) {
        add_state_file(dest, relpath, source->file_stat[ind_source]);
    } else {
        dest->file_stat[ind_dest] = source->file_stat[ind_source];
    }
    if (is_stage) {
        if (source->file_stat[ind_source] != S_DELETED) {
            dest->file_stat[ind_dest] = S_UNCHANGED;
        }
    }
    if (source->file_stat[ind_source] != S_DELETED) {
        copy_only_file(dest, source, relpath);
    // TODO: Maybe it's necessary to keep the old versions
    } else {
        // not sure about this
        delete_state_file(dest, relpath);
        remove_file_from_state_data(dest, relpath);
    }
    return 0;
}

#endif