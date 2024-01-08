#include <string.h>

int search_str(const char** arr, const char* str, int last_ind)
{
    for (int i = 0; i <= last_ind; i++)
        if (strcmp(arr[i], str) == 0) return i;
    return -1;
}