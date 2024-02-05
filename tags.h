#ifndef TAGS_H
#define TAGS_H

#include <dirent.h>
#include <time.h>
#include "data.h"

// int get_author_name(char*);
// int get_author_email(char*);

typedef struct Tag {
    char name[TAG_NAME_MAX];
    char message[COMMIT_MESSAGE_MAX + 1];
    int commit_id;
    time_t time_created;
    char author_name[USERNAME_MAX];
    char author_email[EMAIL_MAX];
} Tag;

char* tag_path(char* tagname)
{
    char* path = (char*) malloc(PATH_MAX * sizeof(char));
    sprintf(path, ".lit\\tags\\%s", tagname);
    return path;
}

Tag* read_tag(char* path)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    Tag* tag = (Tag*) malloc(sizeof(Tag));
    // Going to the root of the repository
    chdir(find_root_path());
    if (chdir(path)) {
        chdir(original_path);
        return NULL;
    }
    FILE* meta = fopen("meta.txt", "r");
    if (meta == NULL) {
        chdir(original_path);
        return NULL;
    }
    // ID
    if (fscanf(meta, "%x\n", &(tag->commit_id)) == EOF) {
        chdir(original_path);
        return NULL;
    }
    // Author name
    if (fgets(tag->author_name, BRANCH_NAME_MAX, meta) == NULL) {
        chdir(original_path);
        return NULL;
    }
    tag->author_name[strcspn(tag->author_name, "\n\r")] = '\0';
    // Author email
    if (fgets(tag->author_email, BRANCH_NAME_MAX, meta) == NULL) {
        chdir(original_path);
        return NULL;
    }
    tag->author_email[strcspn(tag->author_email, "\n\r")] = '\0';
    // Commit message
    if (fscanf(meta, "~%[^~]", tag->message) == EOF) {
        chdir(original_path);
        return NULL;
    }
    fclose(meta);
    // Datetime
    FILE* time_file = fopen("time.bin", "rb");
    fread(&(tag->time_created), sizeof(time_t), 1, time_file);
    fclose(time_file);
    // Name
    strcpy(tag->name, strrchr(path, '\\'));

    chdir(original_path);
    return tag;
}

int write_tag(Tag* tag, char* path)
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));

    // Going to the root of the repository
    chdir(find_root_path());

    mkdir(path); // Creates the folder if it already doesn't exist
    chdir(path);
    // Writing meta data
    FILE* meta = fopen("meta.txt", "w");
    fprintf(meta, "%x\n%s\n%s\n~%s~\n", tag->commit_id, tag->author_name, tag->author_email, tag->message); // Important order
    fclose(meta);
    // Writing time in binary
    FILE* time_file = fopen("time.bin", "wb");
    fwrite(&(tag->time_created), sizeof(time_t), 1, time_file);
    fclose(time_file);
    // Going back to the original path
    chdir(original_path);
    return 0;
}

Tag* get_tag_by_name(char* tagname)
{
    char* path = tag_path(tagname);
    Tag* tag = read_tag(path);
    return tag;
}

bool add_tag(char* tagname, char* message, int commit_id, bool overwrite)
{
    if (!overwrite && get_tag_by_name(tagname) != NULL) return false;

    Tag* tag = (Tag*) malloc(sizeof(Tag));
    strcpy(tag->name, tagname); strcpy(tag->message, message);
    tag->commit_id = commit_id;
    get_author_name(tag->author_name);
    get_author_email(tag->author_email);
    tag->time_created = time(NULL);
    write_tag(tag, tag_path(tagname));
    return true;
}

void print_all_tags()
{
    char original_path[PATH_MAX];
    getcwd(original_path, sizeof(original_path));
    chdir(find_repo_data());
    //
    DIR* folder = opendir("tags");
    if (folder == NULL) return;
    struct dirent* entry;
    int num = 1;
    while ( (entry = readdir(folder)) != NULL ) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        printf("%d- %s\n", num++, entry->d_name);
    }
    closedir(folder);

    //
    chdir(original_path);
}

bool show_tag(char* tagname)
{
    Tag* tag = read_tag(tag_path(tagname));
    if (tag == NULL) return false;

    printf("Tag \033[0;35m%s\033[0m\n", tagname);
    printf("Commit: \033[0;36m%x\033[0m\n", tag->commit_id);
    printf("Author: %s <%s>\n", tag->author_name, tag->author_email);
    printf("Last modified: %s\n", ctime(&(tag->time_created)) );
    printf("Message: \033[0;33m%s\033[0m\n", tag->message);
    return true;
}
#endif