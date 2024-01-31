#ifndef CHANGES_H
#define CHANGES_H

#include <stdio.h>
#include <unistd.h>

#include "file-util.h"
#include "struct-state.h"
#include "data.h"

int create_root_commit()
{
    State* root = initialize_state(ROOT_ID, ROOT_ID, "", "master", "", "", ".lit\\states\\commits\\0");
    get_author_name(root->author_name);
    get_author_email(root->author_email);
    write_state(root, root->data_dir);
    return 0;
}

State* get_stage_object()
{
    return read_state(".lit\\states\\stage");
}

enum Filestat calculate_new_state(int prior_id, int latter_id, char* relpath)
{
    State* prior = get_state_by_id(prior_id);
    State* latter = get_state_by_id(latter_id);
    if (prior == NULL || latter == NULL) return 30; // idk
    int find_p = find_state_file(prior, relpath);
    int find_l = find_state_file(latter, relpath);

    if ( (find_p == -1 || prior->file_stat[find_p] == S_DELETED) && find_l > -1 ) {
        return S_ADDED;
    } else if ( (find_p == -1 || prior->file_stat[find_p] == S_DELETED) && (find_l == -1 || prior->file_stat[find_p] == S_DELETED) ) {
        return 31; // idk again
    } else if ( find_p > -1 && (find_l == -1 || prior->file_stat[find_p] == S_DELETED) ) {
        return S_DELETED;
    } else {
        FILE* fp = open_state_file(prior, relpath);
        FILE* fl = open_state_file(latter, relpath);
        if ( is_the_same_textfile(fp, fl) ) {
            return S_UNCHANGED;
        } else {
            return S_MODIFIED;
        }
    }
}

enum Filestat compare_wd_with_state(int state_id, char* relpath)
{
    State* state = get_state_by_id(state_id);
    if (state == NULL) return 30; // idk
    int sfile_ind = find_state_file(state, relpath);
    FILE* wd_file = open_wd_file(relpath);

    if ( (sfile_ind == -1 || state->file_stat[sfile_ind] == S_DELETED) && wd_file != NULL ) {
        return S_ADDED;
    } else if ( (sfile_ind == -1 || state->file_stat[sfile_ind] == S_DELETED) && wd_file == NULL ) {
        return 31; // idk again
    } else if ( sfile_ind > -1 && wd_file == NULL ) {
        return S_DELETED;
    } else {
        FILE* s_file = open_state_file(state, relpath);
        if ( is_the_same_textfile(s_file, wd_file) ) {
            return S_UNCHANGED;
        } else {
            return S_MODIFIED;
        }
    }
}

// Return codes 0: success, 1: file not found, 2: already staged
int stage_file(char* filename, State* stage_obj) 
{
    if ( file_exists(filename, true) ) { // Directory
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path));
        chdir(filename);

        DIR* folder = opendir(".");
        if (folder == NULL) return 1;
        struct dirent* entry;
        int returnval = 0;
        // Change working directory to the folder and keeping the original path
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            returnval = !stage_file(entry->d_name, stage_obj) || returnval;
        }
        closedir(folder);
        chdir(original_path);
        return !returnval;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        bool is_staged = false;
        if (f_ind > -1) {
            enum Filestat fstat = stage_obj->file_stat[f_ind];
            // TODO: AFTER DOING THE COMMITS, DO THE CHANGES WITH DIFFERENT STATS ////???/////
            
        }
        
        add_state_file(stage_obj, relpath, S_ADDED);
        copy_state_file_from_wd(stage_obj, relpath);
        return 0;
    } else {
        return 1;
    }
}

int print_stage_status(char* filename, State* stage_obj, int depth) 
{
    if ( file_exists(filename, true) ) { // Directory
        if (depth < 1) return 0;
        depth--;
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path));
        chdir(filename);

        DIR* folder = opendir(".");
        if (folder == NULL) return 1;
        struct dirent* entry;
        // Change working directory to the folder and keeping the original path
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            print_stage_status(entry->d_name, stage_obj, depth);
        }
        closedir(folder);
        chdir(original_path);
        return 0;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        printf("- %s: ", relpath);
        if (f_ind > -1) {
            enum Filestat stat = compare_wd_with_state(STAGE_ID, relpath);
            if (stat == S_UNCHANGED) {
                printf(FILE_IS_STAGED_COLOR);
            } else {
                printf(FILE_ISNT_STAGED_COLOR);
            }
        } else {
            printf(FILE_IS_UNTRACKED_COLOR);
        }
        return 0;
    } else {
        return 1;
    }
}


int create_new_commit(int id, int parent_id, char* message, char* branch, char* name, char* email, char* data_dir)
{
    
}

State* get_head_commit()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    int id;
    FILE* head_f = fopen("states\\commits\\HEAD", "rb");
    fread(&id, sizeof(int), 1, head_f);
    fclose(head_f);
    chdir(original_path);
    return get_state_by_id(id);
}

int change_head(int head_id)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    FILE* head_f = fopen("states\\commits\\HEAD", "wb");
    fwrite(&head_id, sizeof(int), 1, head_f);
    fclose(head_f);
    chdir(original_path);
    return 0;
}

int create_stage()
{
    State* stage = inherit_state(get_head_commit(), STAGE_ID);
    for (int i = 0; i < stage->n_files; i++) {
        enum Filestat stat = stage->file_stat[i];
        if (stat != S_DELETED) stage->file_stat[i] = S_UNCHANGED;
    }
    write_state(stage, stage->data_dir);
    update_all_state_files(stage);
} 

// Return codes 0: success, 1: file not found, 2: already staged
int unstage_file(char* filename, State* stage_obj) 
{
    if ( file_exists(filename, true) ) { // Directory
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path));
        chdir(filename);

        DIR* folder = opendir(".");
        if (folder == NULL) return 1;
        struct dirent* entry;
        int returnval = 0;
        // Change working directory to the folder and keeping the original path
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            returnval = !unstage_file(entry->d_name, stage_obj) || returnval;
        }
        closedir(folder);
        chdir(original_path);
        return !returnval;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        bool is_staged = false;
        if (f_ind > -1) {
            enum Filestat fstat = stage_obj->file_stat[f_ind];
            // TODO: AFTER DOING THE COMMITS, DO THE CHANGES WITH DIFFERENT STATS ////???/////
            
        }
        copy_file_attributes(stage_obj, get_head_commit(), relpath, true);
        return 0;
    } else {
        return 1;
    }
}

#endif // CHANGES_H