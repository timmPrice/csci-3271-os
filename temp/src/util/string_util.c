#include "string_util.h"
#include "checked_malloc.h"

#include <string.h>
#include <stdlib.h>

int path_parent(const char* path, char* parent_path) {
    if (path == NULL || parent_path == NULL) {
        return -1;
    }
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        // No slash found. For now this is an error; create() and open() will only support absolute paths.
        return -1;
    }
    size_t parent_length = last_slash - path;
    if (parent_length == 0) {
        // The parent is the root directory
        strcpy(parent_path, "/");
        return 0;
    }
    strncpy(parent_path, path, parent_length);
    parent_path[parent_length] = '\0';
    return 0;
}

int path_basename(const char* path, char* basename) {
    if (path == NULL || basename == NULL) {
        return -1;
    }
    const char* last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        // The path is already a basename
        strcpy(basename, path);
        return 0;
    }
    // last_slash + 1 points to the first character of the basename,
    // which ends at the null-terminated end of the string
    strcpy(basename, last_slash + 1);
    return 0;
}

char** string_split(const char* source, const char* separator, int* num_substrings) {
    if (source == NULL || separator == NULL || num_substrings == NULL) {
        return NULL;
    }
    size_t separator_len = strlen(separator);
    // Make a copy to modify
    char* tokenized_str = strdup(source);
    if (tokenized_str == NULL) {
        return NULL;
    }
    *num_substrings = 1;
    char* search_start = tokenized_str;
    char* separator_pos = strstr(search_start, separator);
    while (separator_pos != NULL) {
        // Replace separator with null
        memset(separator_pos, '\0', separator_len);
        (*num_substrings)++;
        // Advance start pointer and search again
        search_start = separator_pos + separator_len;
        separator_pos = strstr(search_start, separator);
    }

    char** substring_array = checked_malloc(*num_substrings * sizeof(char*));
    char* cur_substring = tokenized_str;
    for (int i = 0; i < *num_substrings; ++i) {
        substring_array[i] = strdup(cur_substring);
        // Duplicate string traversal, but strdup/strcpy don't tell you the length
        cur_substring += strlen(cur_substring);
        // Advance past one or more nulls to the beginning of the next substring
        cur_substring += separator_len;
    }
    free(tokenized_str);
    return substring_array;
}
