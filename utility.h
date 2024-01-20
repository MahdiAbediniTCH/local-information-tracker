#include <string.h>
#include <stdio.h>

int search_str(const char** arr, const char* str, int last_ind)
{
    for (int i = 0; i <= last_ind; i++)
        if (strcmp(arr[i], str) == 0) return i;
    return -1;
}

void printerr(const char* error)
{
    printf("\033[0;31m");
    fprintf(stderr, error);
    printf("\033[0m");
}