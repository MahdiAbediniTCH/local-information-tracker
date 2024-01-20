#include <unistd.h>
#include <limits.h>
#include "file-util.h"

#define LITDIR_NAME ".lit"

char* find_repo_data()
{
    // search in current dir and parents
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    for (;;) {
        if ( file_exists(LITDIR_NAME, 1) ) {
            char* result = (char*) malloc(PATH_MAX * sizeof(char));
            getcwd(result, PATH_MAX);
            strcat(result, "\\"); strcat(result, LITDIR_NAME);
            chdir(original_path);
            return result;
        }
        if ( is_root() ) break;
        chdir("..");
    }
    return NULL;
}

int is_in_repo()
{
    return find_repo_data() != NULL;
}
