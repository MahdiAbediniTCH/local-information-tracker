#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "constants.h"

int is_file(const char* name)
{
    struct stat path;
    stat(name, &path);
    return S_ISREG(path.st_mode);
}

int is_root()
{
    char path[PATH_MAX];
    getcwd(path, sizeof(path));
    return strlen(path) < 4; // In windows, all drive names are 1 letter long (e.g. f:/)
}

int file_exists(char name[], int is_dir)
{
    if (!is_dir)
        return is_file(name); // This will be enough if it's a file
    if (is_file(name)) // Otherwise if it's supposed to be a directory but a file with that name is found, there can be no directory with that
        return 0;
    return access(name, F_OK) == 0;
}