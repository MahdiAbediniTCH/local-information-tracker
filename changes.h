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

// Return codes 0: success, 1: file not found, 2: already staged
int stage_file(char* filename) 
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
            stage_file(entry->d_name);
        }
        chdir(original_path);
        closedir(folder);
    } else if ( file_exists(filename, false) ) { // File
        printf("Added: %s\n", filename); // TODO
        return 0;
    } else {
        return 1;
    }
}

int create_new_commit(int id, int parent_id, char* message, char* branch, char* name, char* email, char* data_dir)
{
    
}
#endif // CHANGES_H