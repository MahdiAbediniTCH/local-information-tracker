#ifndef UTILITY_H // Include guard
#define UTILITY_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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

bool is_hex(char* str)
{
    for (unsigned i = 0; i < strlen(str); i++) {
        if ( !( ('0' <= str[i] && str[i] <= '9') || ('a' <= str[i] && str[i] <= 'f') ) ) {
            return false;
        }
    }
    return true;
}

#endif // UTILITY_H