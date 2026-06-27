#include "client.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool client_run(const char *address, unsigned short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        close(fd);
        return false;
    }

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(fd);
        return false;
    }

    char buffer[1024];
    while (true) {
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        size_t length = strcspn(buffer, "\n");
        buffer[length] = '\0';
        size_t out_len = strlen(buffer);
        char *outbuf = malloc(out_len + 2);
        if (!outbuf) break;
        memcpy(outbuf, buffer, out_len);
        outbuf[out_len] = '\n';
        outbuf[out_len + 1] = '\0';
        if (send(fd, outbuf, out_len + 1, 0) < 0) {
            free(outbuf);
            break;
        }
        free(outbuf);
        ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            break;
        }
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }

    close(fd);
    return true;
}

int main(int argc, char *argv[]) {
    const char *address = "127.0.0.1";
    unsigned short port = 6379;

    if (argc > 1) {
        address = argv[1];
    }
    if (argc > 2) {
        port = (unsigned short)strtoul(argv[2], NULL, 10);
    }

    if (!client_run(address, port)) {
        fprintf(stderr, "Failed to connect to %s:%u\n", address, port);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
