#ifndef CHANGES_H
#define CHANGES_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "file-util.h"
#include "struct-state.h"
#include "data.h"

int change_branch_id(char*, int);

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

int last_commit_id()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());;
    //
    DIR* folder = opendir("states\\commits");
    if (folder == NULL) return 1;
    struct dirent* entry;
    int max_id = 0x9f;
    // Change working directory to the folder and keeping the original path
    while ( (entry = readdir(folder)) != NULL ) {
        if ( is_ignored(entry->d_name) ) continue;
        if (entry->d_type == DT_DIR) {
            int id;
            sscanf(entry->d_name, "%x", &id);
            max_id = (id > max_id) ? id : max_id;
        }
    }
    closedir(folder);
    //
    chdir(original_path);
    return max_id;
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

int get_head_id()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    int id;
    FILE* head_f = fopen("states\\commits\\HEAD", "rb");
    fread(&id, sizeof(int), 1, head_f);
    fclose(head_f);
    chdir(original_path);
    return id;
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

enum Filestat compare_wt_with_state(int state_id, char* relpath)
{
    State* state = get_state_by_id(state_id);
    if (state == NULL) return 30; // idk
    int sfile_ind = find_state_file(state, relpath);
    FILE* wt_file = open_wt_file(relpath);

    if ( (sfile_ind == -1 || state->file_stat[sfile_ind] == S_DELETED) && wt_file != NULL ) {
        fclose(wt_file);
        return S_ADDED;
    } else if ( (sfile_ind == -1 || state->file_stat[sfile_ind] == S_DELETED) && wt_file == NULL ) {
        fclose(wt_file);
        return S_UNCHANGED; // Special case, be careful
    } else if ( sfile_ind > -1 && wt_file == NULL ) {
        fclose(wt_file);
        return S_DELETED;
    } else {
        FILE* s_file = open_state_file(state, relpath);
        if ( is_the_same_textfile(s_file, wt_file) ) {
            fclose(wt_file);
            return S_UNCHANGED;
        } else {
            fclose(wt_file);
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
        if (relpath == NULL) return 1;
        int f_ind = find_state_file(stage_obj, relpath);
        bool is_staged = false;
        enum Filestat stat = compare_wt_with_state(get_head_id(), relpath);
        State* head = get_head_commit();
        if (stat == S_UNCHANGED && head->file_stat[find_state_file(head, relpath)] == S_DELETED) return 2;
        add_state_file(stage_obj, relpath, stat);
        copy_state_file_from_wt(stage_obj, relpath);
        return 0;
    } else {
        return 1;
    }
}

int detect_deleted_files()
{
    State* stage = get_stage_object();
    State* head = get_head_commit();
    bool found = false;
    for (int i = 0; i < head->n_files; i++) {
        if (head->file_stat[i] == S_DELETED) continue;
        char* relpath = head->tracked_files[i];
        int s_ind = find_state_file(stage, relpath);
        if (stage->file_stat[s_ind] == S_DELETED) continue;
        enum Filestat stat = compare_wt_with_state(head->state_id, relpath);
        if (stat == S_DELETED) {
            found = true;
            delete_state_file(stage, relpath);
            stage->file_stat[s_ind] = S_DELETED;
        }
    }
    if (found) {
        write_state(stage, stage->data_dir);
    }
    return found;
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
        if (relpath == NULL) return 1;
        int f_ind = find_state_file(stage_obj, relpath);
        printf("- %s: ", relpath);
        if (f_ind > -1) {
            enum Filestat stat = compare_wt_with_state(STAGE_ID, relpath);
            if (stat == S_UNCHANGED) {
                if (compare_wt_with_state(get_head_id(), relpath) == S_UNCHANGED) {
                    printf(FILE_IS_UNCHANGED_COLOR);
                } else {
                    printf(FILE_IS_STAGED_COLOR);
                }
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
    State* head = get_head_commit();
    State* stage = inherit_state(head, STAGE_ID);
    int ind = 0;
    for (int i = 0; i < head->n_files; i++) {
        enum Filestat stat = stage->file_stat[i];
        if (stat != S_DELETED) {
            strcpy(stage->tracked_files[ind], head->tracked_files[i]);
            stage->file_stat[ind] = S_UNCHANGED;
            ind++;
        }
    }
    stage->n_files = ind;
    write_state(stage, stage->data_dir);
    update_all_state_files(stage);
    return 0;
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
        if (relpath == NULL) return 1;
        copy_file_attributes(stage_obj, get_head_commit(), relpath, true);
        return 0;
    } else {
        return 1;
    }
}
// Return codes 0: success, 1: file not found, 2: already staged
int show_file_status(char* filename, State* head_obj, State* stage_obj) 
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
            show_file_status(entry->d_name, head_obj, stage_obj);
        }
        closedir(folder);
        chdir(original_path);
        return 0;
    } else if ( file_exists(filename, false) ) { // File
        char* relpath = file_relative_to_root(filename, find_root_path());
        if (relpath == NULL) return 1;
        int f_ind = find_state_file(stage_obj, relpath);
        enum Filestat stat_head = compare_wt_with_state(get_head_id(), relpath);
        if (stat_head == S_UNCHANGED) return 1;

        printf("    ~ %s: ", relpath);
        if (f_ind > -1) {
            enum Filestat stat = compare_wt_with_state(STAGE_ID, relpath);
            if (stat == S_UNCHANGED) {
                printf(GREED_PLUS);
            } else {
                printf(RED_MINUS);
            }
        } else {
            printf(RED_MINUS);
        }
        if (stat_head == S_MODIFIED) printf(MODIFIED_COLOR);
        if (stat_head == S_ADDED) printf(ADDED_COLOR);
        if (stat_head == S_DELETED) printf(DELETED_COLOR); // This doesn't happen bruh
        return 0;
        
    } else {
        return 1;
    }
}
// state 0: return current status (true for detached) | state 1: head will be detached | state -1: attached
bool detached_head(int state)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    //
    bool st;
    if (state == 0) {
        FILE* d_file = fopen("DETACHED", "rb");
        fread(&st, sizeof(bool), 1, d_file);
        fclose(d_file);
    } else {
        st = (state == 1) ? true : false;
        FILE* d_file = fopen("DETACHED", "wb");
        fwrite(&st, sizeof(bool), 1, d_file);
        fclose(d_file);
    }
    //
    chdir(original_path);
    return st;
}

bool show_deleted_files()
{
    bool didprint = false;
    State* head = get_head_commit();
    for (int i = 0; i < head->n_files; i++) {
        char* relpath = head->tracked_files[i];
        enum Filestat stat = compare_wt_with_state(get_head_id(), relpath);
        if (stat == S_DELETED) {
            didprint = true;
            printf("    ~ %s: ", relpath);
            enum Filestat stat = compare_wt_with_state(STAGE_ID, relpath);
            if (stat == S_UNCHANGED) {
                printf(GREED_PLUS);
            } else {
                printf(RED_MINUS);
            }
            printf(DELETED_COLOR);
        }
    }
    return didprint;
}

char* get_current_branch_name(char* branch)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    //
    FILE* file = fopen("branch\\current-branch.txt", "r");
    fgets(branch, BRANCH_NAME_MAX, file);
    fclose(file);
    //
    chdir(original_path);
    return branch;
}
// Returns -1 upon failure
int get_branch_commit_id(char* branch)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    chdir("branch");
    //
    DIR* branchdir = opendir(".");
    if (branchdir == NULL) return 1;
    struct dirent* entry;
    int commit_id = -1;
    while ( (entry = readdir(branchdir)) != NULL ) {
        if (entry->d_type != DT_REG || (strcmp(entry->d_name, "current-branch.txt") == 0)) continue;
        if ( strcmp(branch, entry->d_name) == 0 ) {
            FILE* branch = fopen(entry->d_name, "rb");
            fread(&commit_id, sizeof(int), 1, branch);
            fclose(branch);
            break;
        }
    }
    closedir(branchdir);
    //
    chdir(original_path);
    return commit_id;
}

int create_new_branch(char* branch)
{
    if (get_branch_commit_id(branch) != -1) return 1;
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    chdir("branch");
    //
    FILE* br_file = fopen(branch, "wb");
    int head_id = get_head_id();
    fwrite(&head_id, sizeof(int), 1, br_file);
    fclose(br_file);
    //
    chdir(original_path);
    return 0;
}

int change_branch_id(char* branch, int id)
{
    int commit_id = get_branch_commit_id(branch);
    if (commit_id == -1) return -1;
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    chdir("branch");
    //
    FILE* br_file = fopen(branch, "wb");
    fwrite(&id, sizeof(int), 1, br_file);
    fclose(br_file);
    //
    chdir(original_path);
    return 0;
}
// Returns branch commit id
int switch_branch(char* branch)
{
    int commit_id = get_branch_commit_id(branch);
    if (commit_id == -1) return -1;
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    chdir("branch");
    //
    FILE* file = fopen("current-branch.txt", "w");
    fputs(branch, file);
    fclose(file);
    //
    chdir(original_path);
    return commit_id;
}

int show_all_branches()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    chdir("branch");
    //
    char current_branch[BRANCH_NAME_MAX];
    get_current_branch_name(current_branch);
    DIR* branchdir = opendir(".");
    if (branchdir == NULL) return 1;
    struct dirent* entry;
    while ( (entry = readdir(branchdir)) != NULL ) {
        if (entry->d_type != DT_REG || strcmp(entry->d_name, "current-branch.txt") == 0) continue;
        printf(BRANCH_PRINT_FORMAT, entry->d_name);
        if ( strcmp(current_branch, entry->d_name) == 0 )
            printf(" *");
        printf("\n");
    }
    closedir(branchdir);
    //
    chdir(original_path);
    return 0;
}

bool is_stage_empty()
{
    State* stage = get_stage_object();
    for (int i = 0; i < stage->n_files; i++) {
        if (stage->file_stat[i] != S_UNCHANGED)
            return false;
    }
    return true;
}

bool is_wt_unchanged()
{
    State* head = get_head_commit();
    for (int i = 0; i < head->n_files; i++) {
        if (compare_wt_with_state(get_head_id(), head->tracked_files[i]) != S_UNCHANGED)
            return false;
    }
    return true;
}

State* do_a_commit(char* message)
{
    if (is_stage_empty()) return NULL;
    State* stage = get_stage_object();
    State* commit = inherit_state(stage, last_commit_id() + 1);
    strcpy(commit->message, message);
    get_current_branch_name(commit->branch_name);
    commit->parent_id = get_head_id();
    write_state(commit, commit->data_dir);
    copy_all_files(commit, stage);
    change_head(commit->state_id);
    create_stage();
    change_branch_id(commit->branch_name, commit->state_id);
    return commit;
}

int get_n_changes(State* state)
{
    int n = 0;
    for (int i = 0; i < state->n_files; i++) {
        n += (state->file_stat[i] != S_UNCHANGED);
    }
    return n;
}

int change_wt_files_to_commit(const State* commit)
{
    // Delete everything that is tracked by current commit
    State* head = get_head_commit();
    for (int i = 0; i < head->n_files; i++) {
        delete_wt_file(head->tracked_files[i]);
    }
    
    // Paste everything from the other commit
    for (int i = 0; i < commit->n_files; i++) {
        if (commit->file_stat[i] == S_DELETED) continue;
        copy_only_file_to_wt(commit, commit->tracked_files[i]);
    }
    return 0;
}
// 0: to head, 1: to branch, 2: to commit, -1: invalid string, -2: commit id doesn't exist
int checkout(char* where)
{
    // HEAD
    if ( strcmp(where, "HEAD") == 0 ) {
        change_wt_files_to_commit(get_head_commit());
        create_stage();
        return 0;
    } 
    // Branches
    int branch_id = get_branch_commit_id(where);
    if (branch_id != -1) {
        change_wt_files_to_commit(get_state_by_id(branch_id));
        change_head(branch_id);
        switch_branch(where);
        detached_head(-1);
        create_stage();
        return 1;
    }
    // Commit id // DETACHED
    if ( !is_hex(where) ) return -1;
    int id;
    sscanf(where, "%x", &id);
    State* commit = get_state_by_id(id);
    if (commit == NULL) return -2;
    change_wt_files_to_commit(commit);
    detached_head(1);
    return 2;
}

#endif // CHANGES_H