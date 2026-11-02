#ifndef __STRING_UTIL_H__
#define __STRING_UTIL_H__


/**
 * Finds the parent path of a POSIX-style absolute path and copies it into the
 * character array pointed to by parent_path. If the path argument is relative
 * (does not begin with "/") the operation will fail and return -1. Assumes the
 * destination array is large enough for the resulting string; behavior is
 * undefined if it is not.
 *
 * @param path Pointer to a null-terminated string containing a path
 * @param parent_path Pointer to a character array to write the output to
 * @return 0 if the operation succeeded, -1 if it failed
 */
int path_parent(const char* path, char* parent_path);

/**
 * Finds the basename (just the file name) of a POSIX-style path and copies it 
 * the character array pointed to by basename. Assumes the destination array is
 * large enough for the name; behavior is undefined if it is not. This function
 * will only fail (return -1) if path or basename is a NULL pointer.
 *
 * @param path Pointer to a null-terminated string containing a path
 * @param basename Pointer to a character array to write the output to
 * @return 0 if the operation succeeded, -1 if it failed
 */
int path_basename(const char* path, char* basename);

/**
 * Splits a string into substrings using a separator, and returns the
 * substrings in a new array of strings. The caller is responsible for freeing
 * the array of strings. The separator string can be any length, but the entire
 * separator must be found in the source string to split it (i.e. if the
 * separator string is "+=" then the source string will only be split where it
 * contains the sequence "+=", not the individual characters "+" or "="). The
 * output parameter num_substrings is used to return the size of the substring
 * array.
 *
 * @param source Pointer to a null-terminated string to split
 * @param separator Pointer to a null-terminated string containing the separating substring
 * @param num_substrings Pointer to an integer that will be filled with the number of substrings found
 * @return A pointer to a newly allocated array of null-terminated strings, or NULL if there was an error.
 */
char** string_split(const char* source, const char* separator, int* num_substrings);

#endif