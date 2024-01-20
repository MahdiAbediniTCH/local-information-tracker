
#include "read-data.h"

int main() {
    char* path = find_repo_data();
    printf("%d\n", file_exists("bruh.ok", 0));
    printf(path);printf("\n");
}