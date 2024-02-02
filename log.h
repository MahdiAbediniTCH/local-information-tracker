#ifndef LOG_H
#define LOG_H

#include "changes.h"
#include <time.h>

time_t parse_time(char* strtime)
{
    struct tm td;
    td.tm_sec = 0;
    if ( sscanf(strtime, "%d/%d/%d %d:%d", &td.tm_year, &td.tm_mon, &td.tm_mday, &td.tm_hour, &td.tm_min) != 5 ) return 0;
    td.tm_year -= 1900; if (td.tm_year <= 0) return 0;
    td.tm_mon--; if (td.tm_mon < 0 || td.tm_mon > 11) return 0;
    if (td.tm_mday < 1 || td.tm_mday > 31) return 0;
    if (td.tm_hour < 0 || td.tm_hour > 23) return 0;
    if (td.tm_min < 0 || td.tm_min > 59) return 0;
    time_t timet = mktime(&td);
    if (timet == -1) perror("Error parsing time");
    return timet;
}

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

void print_branch_commits(char* branch)
{
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        if ( strcmp(commit->branch_name, branch) == 0 ) {
            log_commit(commit);
        }
    }
}

void print_author_commits(char* author)
{
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        if ( strcmp(commit->author_name, author) == 0 ) {
            log_commit(commit);
        }
    }
}

bool print_commits_since(char* strtime)
{
    time_t anchor = parse_time(strtime);
    if (anchor == 0) return false;
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        if ( commit->time_created > anchor ) {
            log_commit(commit);
        }
    }
    return true;
}

bool print_commits_before(char* strtime)
{
    time_t anchor = parse_time(strtime);
    if (anchor == 0) return false;
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        if ( commit->time_created < anchor ) {
            log_commit(commit);
        }
    }
    return true;
}

bool print_message_commits(char* word)
{
    bool found = false;
    int last_id = last_commit_id();
    for (int i = last_id; i >= 0xa0; i--) {
        char data_dir[PATH_MAX]; get_state_data_dir(data_dir, i);
        State* commit = read_state(data_dir);
        if ( strstr(commit->message, word) != NULL ) {
            found = true;
            log_commit(commit);
        }
    }
    return found;
}

#endif