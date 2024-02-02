#include "changes.h"



void log_commit(State* commit)
{
    printf(LOG_FORMAT, 
        commit->state_id, 
        commit->branch_name, 
        commit->author_name, commit->author_email, 
        ctime(&(commit->time_created)), 
        get_n_changes(commit), 
        commit->message
    );
}

void print_all_commits()
{
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        log_commit(commit);
    }
}

void print_n_last_commits(int n)
{
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        if (n < 1) break;
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        log_commit(commit);
        n--;
    }
}