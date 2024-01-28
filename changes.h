#ifndef CHANGES_H
#define CHANGES_H

#include <stdio.h>
#include <unistd.h>

#include "file-util.h"
#include "struct-state.h"
#include "data.h"

int create_root_commit()
{
    // State* root = initialize_state(0x0, "", "master", get_user_name(), get_user_email(), ".lit\\states\\commits\\0");

}

State* get_stage_object()
{
    return read_state(".lit\\states\\stage");
}

// Return codes 0: success, 1: file not found, 2: already staged
int stage_file(char* filename, State* stage_obj) 
{
    if ( file_exists(filename, true) ) { // Directory
        DIR* folder = opendir(filename);
        if (folder == NULL) return 1;
        struct dirent* entry;
        // Change working directory to the folder and keeping the original path
        char original_path[PATH_MAX];
        getcwd(original_path, sizeof(original_path));
        chdir(filename);
        while ( (entry = readdir(folder)) != NULL ) {
            if ( is_ignored(entry->d_name) ) continue;
            stage_file(entry->d_name, stage_obj);
        }
        chdir(original_path);
        closedir(folder);
        return 0;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        int f_ind = find_state_file(stage_obj, relpath);
        if (f_ind > -1) {
            enum Filestat fstat = stage_obj->file_stat[f_ind];
            // TODO: AFTER DOING THE COMMITS, DO THE CHANGES WITH DIFFERENT STATS
            
        }
        add_state_file(stage_obj, relpath, S_ADDED);
        // TOODODODODOD: Instead of updating all you should only update the new file (im gonna kms)
        return 0;
    } else {
        return 1;
    }
}

int create_new_commit(int id, int parent_id, char* message, char* branch, char* name, char* email, char* data_dir)
{
    
}
#endif // CHANGES_H