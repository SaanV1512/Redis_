#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *utils_strdup(const char *source) {
    if (!source) {
        return NULL;
    }
    size_t len = strlen(source) + 1;
    char *copy = malloc(len);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, source, len);
    return copy;
}

char **utils_split_words(const char *input, size_t *count_out) {
    if (!input || !count_out) {
        return NULL;
    }

    char *copy = utils_strdup(input);
    if (!copy) {
        *count_out = 0;
        return NULL;
    }

    size_t capacity = 8;
    size_t count = 0;
    char **words = calloc(capacity, sizeof(char *));
    if (!words) {
        free(copy);
        *count_out = 0;
        return NULL;
    }

    char *token = strtok(copy, " \t\r\n");
    while (token) {
        if (count >= capacity) {
            capacity *= 2;
            char **new_words = realloc(words, capacity * sizeof(char *));
            if (!new_words) {
                break;
            }
            words = new_words;
        }
        words[count++] = utils_strdup(token);
        token = strtok(NULL, " \t\r\n");
    }

    free(copy);
    *count_out = count;
    return words;
}

void utils_free_words(char **words, size_t count) {
    if (!words) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(words[i]);
    }
    free(words);
}

void utils_trim_newline(char *text) {
    if (!text) {
        return;
    }
    char *end = text + strlen(text);
    while (end > text && (*(end - 1) == '\n' || *(end - 1) == '\r')) {
        *(--end) = '\0';
    }
}
