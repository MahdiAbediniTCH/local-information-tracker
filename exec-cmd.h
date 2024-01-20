#include <stdio.h>
#include "read-data.h"

// Constant strings
const char 
// options
GLOBAL_OPTION[] = "-global", 
// errors
FEW_ARGUMENTS[] = "Too few arguments\n",
NO_REPO[] = "Not a repository\n"
;
// Constants end

int exec_config(int argc, char *argv[])
{
    int global = 0;
    int arg_ind = 2;
    if (argc < 4) {
        fprintf(stderr, FEW_ARGUMENTS);
        return 1;//code
    }
    if (argv[arg_ind] == GLOBAL_OPTION) {
        global = 1;
        arg_ind++;
    }
    if (!is_repo() && !global) {
        fprintf(stderr, NO_REPO);
        return 1;//code
    }

}
int exec_init(int argc, char *argv[])
{
    printf("init \n");
}