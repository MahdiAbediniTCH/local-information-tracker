
#include "data.h"
#include "struct-state.h"
#include <time.h>

void debug_state(State* state)
{
    printf("ID: %x\nTime: %sMessage: %s\nBranch: %s\nAuthor name: %s\nAuthor email: %s\nn_files: %d\nData path: %s\n",
         state->state_id, ctime(&(state->time_created)), state->message, state->branch_name, state->author_name, state->author_email, state->n_files, state->data_dir);
    printf("-- Files:\n");
    for (int i = 0; i < state->n_files; i++)
    {
        printf("%s | %d\n", state->tracked_files[i], state->file_stat[i]);
    }
    return;
}

int main() {
    State* state = initialize_state(0xab, "hello\nworsssld!\naaa.", "brrr", "mmd", "mmd@mmd.mdd", ".lit\\states\\stage");
    debug_state(state);
    // printf("Write Value: %d\n", write_state(state, state->data_dir));
    State* state2 = read_state(state->data_dir);
    debug_state(state2);
    add_state_file(state2, "data.exe", S_DELETED);
    add_state_file(state2, "exec-cmd.exe", S_ADDED);
    add_state_file(state2, "testicle\\balls\\ball.txt", S_ADDED);
    write_state(state2, state2->data_dir);
    State* state3 = read_state(state2->data_dir);
    debug_state(state3); // HAVENT TESTED BRUH;
    update_all_state_files(state3);
    printf("done\n");
}