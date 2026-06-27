#include "protocol/resp_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *read_token(const char *start, size_t length) {
    if (!start || length == 0) {
        return NULL;
    }
    char *token = malloc(length + 1);
    if (!token) {
        return NULL;
    }
    memcpy(token, start, length);
    token[length] = '\0';
    return token;
}

bool resp_parse_request(const char *input, char ***argv_out, size_t *argc_out) {
    if (!input || !argv_out || !argc_out) {
        return false;
    }

    if (*input != '*') {
        return false;
    }

    const char *cursor = input + 1;
    long argc = strtol(cursor, (char **)&cursor, 10);
    if (argc <= 0 || *cursor != '\r' || *(cursor + 1) != '\n') {
        return false;
    }
    cursor += 2;

    char **argv = calloc((size_t)argc, sizeof(char *));
    if (!argv) {
        return false;
    }

    for (long i = 0; i < argc; ++i) {
        if (*cursor != '$') {
            resp_free_argv(argv, (size_t)i);
            return false;
        }
        cursor++;
        long len = strtol(cursor, (char **)&cursor, 10);
        if (len < 0 || *cursor != '\r' || *(cursor + 1) != '\n') {
            resp_free_argv(argv, (size_t)i);
            return false;
        }
        cursor += 2;
        argv[i] = read_token(cursor, (size_t)len);
        if (!argv[i]) {
            resp_free_argv(argv, (size_t)i);
            return false;
        }
        cursor += len;
        if (*cursor != '\r' || *(cursor + 1) != '\n') {
            resp_free_argv(argv, (size_t)i + 1);
            return false;
        }
        cursor += 2;
    }

    *argv_out = argv;
    *argc_out = (size_t)argc;
    return true;
}

void resp_free_argv(char **argv, size_t argc) {
    if (!argv) {
        return;
    }
    for (size_t i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    free(argv);
}
