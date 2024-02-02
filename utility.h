#ifndef UTILITY_H // Include guard
#define UTILITY_H

#include <string.h>
#include <stdio.h>

int search_str(const char** arr, const char* str, int nmemb)
{
    for (int i = 0; i < nmemb; i++)
        if (strcmp(arr[i], str) == 0) return i;
    return -1;
}

void printerr(const char* error)
{
    printf("\033[0;31m");
    fprintf(stderr, error);
    printf("\033[0m");
}

#endif // UTILITY_H