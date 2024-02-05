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

enum Filestat compare_states(State* prior, State* latter, char* relpath)
{
    if (prior == NULL || latter == NULL) return 30; // idk
    int find_p = find_state_file(prior, relpath);
    int find_l = find_state_file(latter, relpath);

    if ( (find_p == -1 || prior->file_stat[find_p] == S_DELETED) && (find_l > -1 && latter->file_stat[find_l] != S_DELETED) ) {
        return S_ADDED;
    } else if ( (find_p == -1 || prior->file_stat[find_p] == S_DELETED) && (find_l == -1 || prior->file_stat[find_l] == S_DELETED) ) {
        return S_UNCHANGED; // idk again
    } else if ( (find_p > -1 && prior->file_stat[find_p] != S_DELETED) && (find_l == -1 || prior->file_stat[find_l] == S_DELETED) ) {
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
    } else if ( (sfile_ind > -1 && state->file_stat[sfile_ind] != S_DELETED) && wt_file == NULL ) {
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
    // checking deleted newly added files
    char* filestoremove[MAX_TRACKED_FILES];
    int n = 0;
    for (int i = 0; i < stage->n_files; i++) {
        if (stage->file_stat[i] == S_ADDED) {
            enum Filestat stat = compare_wt_with_state(STAGE_ID, stage->tracked_files[i]);
            if (stat == S_DELETED) {
                filestoremove[n++] = stage->tracked_files[i];
            }
        }
    }
    for (int i = 0; i < n; i++) {
        delete_state_file(stage, filestoremove[i]);
        remove_file_from_state_data(stage, filestoremove[i]);
    }
    if (n > 0) {
        write_state(stage, stage->data_dir);
    }
    return found || n > 0;
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
    change_head(id);
    detached_head(1);
    return 2;
}


State* revert_no_commit(char* str_id)
{
    int id;
    if ( strncmp(str_id, "HEAD-", 5) == 0 ) {
        int n_back = atoi(str_id + 5);
        if (n_back < 1) return NULL;
        id = get_head_id();
        for (int i = 0; i < n_back; i++) {
            State* commit = get_state_by_id(id);
            if (commit->second_parent_id != 0) { // Means we had a merge
                free(commit);
                return NULL;
            }
            id = commit->parent_id;
            free(commit);
        }
        if (id < 0xa0) return NULL;
        
    } else  {
        if ( !is_hex(str_id) ) return NULL;
        sscanf(str_id, "%x", &id);
    }
    // TODO: checking if there was a merge in the middle
    State* commit = NULL;
    int p_id = get_head_id();
    bool found_commit_id = false;
    while (id >= 0xa0) {
        commit = get_state_by_id(p_id);
        if (p_id == id) {
            found_commit_id = true;
            break;
        }
        if (commit->second_parent_id != 0) { // Means we had a merge
            free(commit);
            return NULL;
        }
        p_id = commit->parent_id;
        free(commit);
    }
    if (!found_commit_id) return NULL;
    change_wt_files_to_commit(commit);
    return commit;
}

int revert(char* str_id, char* message)
{
    State* commit = revert_no_commit(str_id);
    if (commit == NULL) return 1;
    State* stage = get_stage_object();

    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_root_path());
    //
    for (int i = 0; i < commit->n_files; i++) {
        stage_file(commit->tracked_files[i], stage);
    }
    write_state(stage, stage->data_dir);
    detect_deleted_files();
    if (strlen(message) < 1) {
        message = commit->message;
    }
    //
    chdir(original_path);

    // TODO: if the stage is the same, it won't make a new commit 
    if (do_a_commit(message) == NULL) return 2;
    return 0;
}
// 0: success | 1: branch not found | 2: conflict 
int merge(char* b1, char* b2)
{
    char currbranch[BRANCH_NAME_MAX]; get_current_branch_name(currbranch);
    int id1 = get_branch_commit_id(b1);
    int id2 = get_branch_commit_id(b2);
    if (id1 == -1 || id2 == -1) return 1;
    State* head = get_state_by_id(id1);
    State* other = get_state_by_id(id2);
    // Checking for conflicts
    bool found_conflict = false;
    for (int i = 0; i < other->n_files; i++) {
        char* relpath = other->tracked_files[i];
        enum Filestat stat = compare_states(head, other, relpath);
        if (stat == S_MODIFIED) {
            printf("Conflict: %s\n", relpath);
            FILE* file1 = open_state_file(head, relpath);
            FILE* file2 = open_state_file(other, relpath);
            // Creating absolute path
            char abspath[PATH_MAX];
            sprintf(abspath, "%s\\%s", find_root_path(), relpath);

            print_differences(file1, file2, 0, 0, 0, 0, abspath, abspath);
            fclose(file1); fclose(file2);
            found_conflict = true;
        }
    }
    if (found_conflict) return 2;
    // Moving to branch1
    if (checkout(b1) != 1) return 1;
    // Comparing and merging files
    State* stage = get_stage_object();
    char* root = find_root_path();
    char abs_path[PATH_MAX];
    for (int i = 0; i < other->n_files; i++) {
        char* relpath = other->tracked_files[i];
        enum Filestat stat = compare_states(head, other, relpath);
        if (stat == S_ADDED) {
            copy_only_file_to_wt(other, relpath);
            sprintf(abs_path, "%s\\%s", root, relpath);
            stage_file(abs_path, stage);
        } 
    }
    write_state(stage, stage->data_dir);

    char message[COMMIT_MESSAGE_MAX + 1];
    sprintf(message, "Merged branches \"%s\" and \"%s\".", b1, b2);

    State* merge_commit = do_a_commit(message);
    merge_commit->second_parent_id = id2;
    write_state(merge_commit, merge_commit->data_dir);
    return 0;
}

void print_differences(FILE* file1, FILE* file2, int start_1, int end_1, int start_2, int end_2, char* file1name, char* file2name)
{
    char line_1[MAX_LINE_LENGTH];
    char line_2[MAX_LINE_LENGTH];
    int line1_num = 1, line2_num = 1;
    if (start_1 != 0 && end_1 != 0 && start_2 != 0 && end_2 != 0) {
        // Seek to the beginning
        for (int i = 0; i < start_1 - 1; i++) {
            fgets(line_1, MAX_LINE_LENGTH, file1);
        }
        for (int i = 0; i < start_2 - 1; i++) {
            fgets(line_2, MAX_LINE_LENGTH, file2);
        }
        line1_num = start_1; line2_num = start_2;
    } else {
        end_1 = 100000;
        end_2 = 100000;
    }
    bool one_ended = false, two_ended = false;
    bool found_difference = false;
    char* tmp;
    for (;;) {
        tmp = fgets(line_1, MAX_LINE_LENGTH, file1);
        if (tmp == NULL || line1_num > end_1) one_ended = true;
        tmp = fgets(line_2, MAX_LINE_LENGTH, file2);
        if (tmp == NULL || line2_num > end_2) two_ended = true;
        while (strspn(line_1, "\n\r\t ") == strlen(line_1)) {// empty line
            line1_num++;
            if (fgets(line_1, MAX_LINE_LENGTH, file1) == NULL || line1_num > end_1) {one_ended = true; break;}
        }
        while (strspn(line_2, "\n\r\t ") == strlen(line_2)) {// empty line
            line2_num++;
            if (fgets(line_2, MAX_LINE_LENGTH, file2) == NULL || line2_num > end_2) {two_ended = true; break;}
        }
        if (strcmp(line_1, line_2) != 0 && !one_ended && !two_ended) { // we have a diff
            found_difference = true;

            printf("\t\t<<<<<<<<<<\n");
            printf("\t\t\t(\033[0;33m%s - %d\033[0m)\n", file1name, line1_num);

            printf("\t\t\t\033[0;35m"); printf(line_1); printf("\033[0m");
            if (line_1[strlen(line_1) - 1] != '\n') printf("\n");

            printf("\t\t\t(\033[0;33m%s - %d\033[0m)\n", file2name, line2_num);

            printf("\t\t\t\033[0;36m"); printf(line_2); printf("\033[0m");
            if (line_2[strlen(line_2) - 1] != '\n') printf("\n");

            printf("\t\t>>>>>>>>>>\n");
        }
        if (one_ended && two_ended) break;
        line1_num++; line2_num++;
    }
    if (!found_difference) {
        printf("No differences were found\n");
    }
}

int commit_differences(char* strid1, char* strid2)
{
    int id1, id2;
    if ( !is_hex(strid1) ) return 1;
    sscanf(strid1, "%x", &id1);
    if ( !is_hex(strid2) ) return 1;
    sscanf(strid2, "%x", &id2);
    State* prior = get_state_by_id(id1);
    State* latter = get_state_by_id(id2);
    if (prior == NULL || latter == NULL) return 1;

    printf("\nCommit \033[0;36m%s\033[0m compared to \033[0;36m%s\033[0m:\n", strid2, strid1);
    bool found_diff = false;
    for (int i = 0; i < prior->n_files; i++) {
        char* relpath = prior->tracked_files[i];
        enum Filestat stat = compare_states(prior, latter, relpath);
        if (stat == S_DELETED) {
            found_diff = true;
            printf("\t\t\033[0;31m- %s\033[0m\n", relpath);
        }
    }
    printf("\n");
    for (int i = 0; i < latter->n_files; i++) {
        char* relpath = latter->tracked_files[i];
        enum Filestat stat = compare_states(prior, latter, relpath);
        if (stat == S_ADDED) {
            found_diff = true;
            printf("\t\t\033[0;32m+ %s\033[0m\n", relpath);
        } else if (stat == S_MODIFIED) {
            found_diff = true;
            printf("\t\t\033[0;36mModified: %s\033[0m\n", relpath);
            FILE* f_prior = open_state_file(prior, relpath);
            FILE* f_latter = open_state_file(latter, relpath);
            print_differences(f_prior, f_latter, 0, 0, 0, 0, relpath, relpath);
            fclose(f_prior); fclose(f_latter);
        }
    }
    if (!found_diff) {
        printf("No differences found\n");
    }
    return 0;
}

#endif // CHANGES_H