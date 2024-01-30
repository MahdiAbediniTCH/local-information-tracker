
#include "changes.h"
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
    State* state2 = initialize_state(0xab, 0x0, "hello\nworsssld!\naaa.", "brrr", "mmd", "mmd@mmd.mdd", ".lit\\states\\stage");
    add_state_file(state2, "data.exe", S_DELETED);
    add_state_file(state2, "exec-cmd.exe", S_ADDED);
    add_state_file(state2, "testicle\\balls\\ball.txt", S_ADDED);
    add_state_file(state2, "balls\\thisis.txt", S_ADDED);
    debug_state(state2);
    remove_file_from_state_data(state2, "exec-cmd.exe");
    debug_state(state2);
    
}