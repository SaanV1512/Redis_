#include "commands.h"
#include "database.h"
#include "utils.h"
#include "protocol/resp_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static char *make_response(const char *text) {
    if (!text) {
        return NULL;
    }
    size_t length = strlen(text) + 1;
    char *response = malloc(length + 2);
    if (!response) {
        return NULL;
    }
    memcpy(response, text, length);
    response[length] = '\n';
    response[length + 1] = '\0';
    return response;
}

static char *format_integer(long value) {
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%ld", value);
    if (length < 0) {
        return NULL;
    }
    char *result = malloc((size_t)length + 1);
    if (!result) {
        return NULL;
    }
    memcpy(result, buffer, (size_t)length + 1);
    return result;
}

static char *join_keys(char **keys, size_t count) {
    if (!keys) {
        return make_response("");
    }
    size_t total = 0;
    for (size_t i = 0; i < count; ++i) {
        total += strlen(keys[i]) + 1;
    }
    char *joined = malloc(total + 1);
    if (!joined) {
        return NULL;
    }
    joined[0] = '\0';
    for (size_t i = 0; i < count; ++i) {
        strcat(joined, keys[i]);
        if (i + 1 < count) {
            strcat(joined, " ");
        }
    }
    return joined;
}

char *commands_handle(const char *input) {
    if (!input) {
        return make_response("ERR empty input");
    }

    char **argv = NULL;
    size_t argc = 0;
    bool parsed = resp_parse_request(input, &argv, &argc);
    if (!parsed) {
        argv = utils_split_words(input, &argc);
    }
    if (!argv || argc == 0) {
        utils_free_words(argv, argc);
        return make_response("ERR empty command");
    }
    printf("argc = %zu\n", argc);
    for(size_t i=0;i<argc;i++)
    {
        printf("[%s]\n", argv[i]);
    }
    
    char *command = argv[0];
    char *response = NULL;

    if (strcasecmp(command, "PING") == 0) {
        response = make_response("PONG");
    } else if (strcasecmp(command, "SET") == 0) {
        if (argc < 3) {
            response = make_response("ERR wrong number of arguments for 'SET'");
        } else if (database_set(argv[1], argv[2])) {
            response = make_response("OK");
        } else {
            response = make_response("ERR failed to set key");
        }
    } else if (strcasecmp(command, "GET") == 0) {
        if (argc != 2) {
            response = make_response("ERR wrong number of arguments for 'GET'");
        } else {
            char *value = database_get(argv[1]);
            if (value) {
                response = value;
            } else {
                response = make_response("nil");
            }
        }
    } else if (strcasecmp(command, "DEL") == 0) {
        if (argc != 2) {
            response = make_response("ERR wrong number of arguments for 'DEL'");
        } else if (database_del(argv[1])) {
            response = make_response("1");
        } else {
            response = make_response("0");
        }
    } else if (strcasecmp(command, "EXISTS") == 0) {
        if (argc != 2) {
            response = make_response("ERR wrong number of arguments for 'EXISTS'");
        } else {
            response = make_response(database_exists(argv[1]) ? "1" : "0");
        }
    } else if (strcasecmp(command, "KEYS") == 0) {
        size_t count = 0;
        char **keys = database_keys(&count);
        if (!keys) {
            response = make_response("");
        } else {
            response = join_keys(keys, count);
            database_free_keys(keys, count);
        }
    } else if (strcasecmp(command, "DBSIZE") == 0) {
        response = format_integer((long)database_size());
    } else if (strcasecmp(command, "FLUSHDB") == 0) {
        if (database_flush()) {
            response = make_response("OK");
        } else {
            response = make_response("ERR flush failed");
        }
    } else if (strcasecmp(command, "EXPIRE") == 0) {
        if (argc != 3) {
            response = make_response("ERR wrong number of arguments for 'EXPIRE'");
        } else {
            int seconds = atoi(argv[2]);
            if (seconds < 0) {
                response = make_response("ERR invalid expire time");
            } else if (database_expire(argv[1], seconds)) {
                response = make_response("1");
            } else {
                response = make_response("0");
            }
        }
    } else if (strcasecmp(command, "TTL") == 0) {
        if (argc != 2) {
            response = make_response("ERR wrong number of arguments for 'TTL'");
        } else {
            response = format_integer(database_ttl(argv[1]));
        }
    } else if (strcasecmp(command, "SAVE") == 0) {
        if (database_save()) {
            response = make_response("OK");
        } else {
            response = make_response("ERR failed to save database");
        }
    } else {
        response = make_response("ERR unknown command");
    }

    if (parsed) {
        resp_free_argv(argv, argc);
    } else {
        utils_free_words(argv, argc);
    }
    return response;
}
