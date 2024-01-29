#ifndef FILEUTIL_H // Include guard
#define FILEUTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "constants.h"

bool is_file(const char* name)
{
    FILE* tmp = fopen(name, "r");
    if ( tmp == NULL ) return false;
    fclose(tmp);
    return true;
}

bool is_root()
{
    char path[PATH_MAX];
    getcwd(path, sizeof(path));
    return strlen(path) < 4; // In windows, all drive names are 1 letter long (e.g. f:/)
}

bool is_ignored(char* filename)
{
    if (strcmp(filename, ".lit") == 0 ||
        strcmp(filename, "..") == 0 ||
        strcmp(filename, ".") == 0) {
            return true;
        }
    return false;
}

bool file_exists(char name[], bool is_dir)
{
    if (!is_dir)
        return is_file(name); // This will be enough if it's a file
    if (is_file(name)) // Otherwise if it's supposed to be a directory but a file with that name is found, there can be no directory with that
        return 0;
    return access(name, F_OK) == 0;
}

// Returns 1 if relative_to is not a parent directory of path, 0 otherwise
int relative_path(char* path, const char* relative_to)
{
    char* tmp = strstr(path, relative_to);
    if ( tmp == NULL ) return 1;
    memmove(path, path + strlen(relative_to) + 1, strlen(path) - strlen(relative_to) + 1);
    return 0;
}

char* file_relative_to_root(const char* filename_only, const char* relative_to)
{
    char* rel_path = (char*) malloc(PATH_MAX * sizeof(char));
    getcwd(rel_path, PATH_MAX);
    strcat(rel_path, "\\");
    strcat(rel_path, filename_only);
    if ( relative_path(rel_path, relative_to) == 1 ) return NULL;
    return rel_path;
}

#endif // FILEUTIL_H