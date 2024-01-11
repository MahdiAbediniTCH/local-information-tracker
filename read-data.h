#include <unistd.h>
#include "file-util.h"

int find_repo_data()
{
    // search in current dir and parents
    while (1) {
        printf("%d\n", file_exists("Users", 1));
        if (is_root()) break;
        chdir("..");
    }
    return 1;
}