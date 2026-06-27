#ifndef REDIS_CLONE_RESP_PARSER_H
#define REDIS_CLONE_RESP_PARSER_H

#include <stdbool.h>
#include <stddef.h>

bool resp_parse_request(const char *input, char ***argv_out, size_t *argc_out);
void resp_free_argv(char **argv, size_t argc);

#endif // REDIS_CLONE_RESP_PARSER_H
