#ifndef REDIS_CLONE_UTILS_H
#define REDIS_CLONE_UTILS_H

#include <stddef.h>

char *utils_strdup(const char *source);
char **utils_split_words(const char *input, size_t *count_out);
void utils_free_words(char **words, size_t count);
void utils_trim_newline(char *text);

#endif // REDIS_CLONE_UTILS_H
