#ifndef CHANGES_H
#define CHANGES_H

#include <stdio.h>
#include <unistd.h>

#include "file-util.h"
#include "struct-state.h"
#include "data.h"

int create_root_commit()
{
    State* root = initialize_state(0x0, 0x0, "", "master", "", "", ".lit\\states\\commits\\0");
    get_author_name(root->author_name);
    get_author_email(root->author_email);
    write_state(root, root->data_dir);
    return 0;
}

State* get_stage_object()
{
    return read_state(".lit\\states\\stage");
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
        // Change working directory to the folder and keeping the original path
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            stage_file(entry->d_name, stage_obj);
        }
        closedir(folder);
        chdir(original_path);
        return 0;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        bool is_staged = false;
        if (f_ind > -1) {
            enum Filestat fstat = stage_obj->file_stat[f_ind];
            // TODO: AFTER DOING THE COMMITS, DO THE CHANGES WITH DIFFERENT STATS
            
        }
        
        add_state_file(stage_obj, relpath, S_ADDED);
        copy_state_file(stage_obj, relpath);
        return 0;
    } else {
        return 1;
    }
}

int add_n_files(char* filename, State* stage_obj, int n) 
{
    if (n < 1) return 0;
    if ( file_exists(filename, true) ) { // Directory
        n--;
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path));
        chdir(filename);

        DIR* folder = opendir(".");
        if (folder == NULL) return 1;
        struct dirent* entry;
        // Change working directory to the folder and keeping the original path
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            add_n_files(entry->d_name, stage_obj, n);
        }
        closedir(folder);
        chdir(original_path);
        return 0;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        if (f_ind > -1) {
            enum Filestat fstat = stage_obj->file_stat[f_ind];
            
        } else {

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
    State* stage = inherit_state(get_head_commit(), 0x01);
    for (int i = 0; i < stage->n_files; i++) {
        enum Filestat stat = stage->file_stat[i];
        if (stat != S_DELETED) stage->file_stat[i] = S_UNCHANGED;
    }
    write_state(stage, stage->data_dir);
    update_all_state_files(stage);
} 
#endif // CHANGES_H