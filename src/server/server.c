#include "server.h"
#include "commands.h"
#include "database.h"
#include "utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 1024

static int listen_fd = -1; //no socket created yet
static bool server_running = false;
static pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;

static void client_handler(void *arg);

static void *client_handler_entry(void *arg) {
    client_handler(arg);
    return NULL;
}

static void client_handler(void *arg) { //for one client only
    int client_fd = *(int *)arg; //get file descriptor of the client socket
    free(arg); //free the memory allocated for the client file descriptor (malloc)
    char buffer[BUFFER_SIZE];

    // Disable Nagle's algorithm for low-latency communication
    int flag = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    while (true) {
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) { //client disconnected or error
            break;
        }
        buffer[bytes] = '\0'; 
        printf("\n----- RECEIVED -----\n");
        printf("COMMAND: [%s]\n", buffer);
        fflush(stdout);
        printf("--------------------\n");
        
        char *response = commands_handle(buffer);
        if (!response) {
            response = utils_strdup("ERR internal error");
        }
        
        // Send response with newline in a single send operation
        size_t response_len = strlen(response);
        char *response_with_newline = malloc(response_len + 2); // +2 for newline and null terminator
        if (response_with_newline) {
            memcpy(response_with_newline, response, response_len);
            response_with_newline[response_len] = '\n';
            response_with_newline[response_len + 1] = '\0';
            
            ssize_t sent = send(client_fd, response_with_newline, response_len + 1, 0);
            free(response_with_newline);
            
            if (sent < 0) {
                free(response);
                break;
            }
        } else {
            // Fallback: send separately if malloc fails
            ssize_t sent = send(client_fd, response, response_len, 0);
            if (sent < 0) {
                free(response);
                break;
            }
            send(client_fd, "\n", 1, 0);
        }
        
        free(response);
    }
    close(client_fd); //close the client socket
}

bool server_start(unsigned short port) {
    pthread_mutex_lock(&server_mutex);
    if (server_running) {
        pthread_mutex_unlock(&server_mutex);
        return false;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        pthread_mutex_unlock(&server_mutex);
        return false;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //allow reuse of the address

    struct sockaddr_in addr = {0}; //address card for the server socket
    addr.sin_family = AF_INET; //IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //bind to all available interfaces
    addr.sin_port = htons(port); //set the port number

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(listen_fd);
        pthread_mutex_unlock(&server_mutex);
        return false;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        close(listen_fd);
        pthread_mutex_unlock(&server_mutex);
        return false;
    }

    server_running = true;
    pthread_mutex_unlock(&server_mutex);

    printf("Server listening on port %u\n", port);
    while (server_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        struct timeval timeout = {1, 0};

        int ready = select(listen_fd + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) {
            break;
        }
        if (ready == 0) {
            continue;
        }

        if (FD_ISSET(listen_fd, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                continue;
            }

            int *client_fd_ptr = malloc(sizeof(int));
            if (!client_fd_ptr) {
                close(client_fd);
                continue;
            }
            *client_fd_ptr = client_fd;
            pthread_t thread;
            if (pthread_create(&thread, NULL, client_handler_entry, client_fd_ptr) != 0) {
                close(client_fd);
                free(client_fd_ptr);
            } else {
                pthread_detach(thread);
            }
        }
    }

    close(listen_fd);
    listen_fd = -1;
    return true;
}

void server_stop(void) {
    pthread_mutex_lock(&server_mutex);
    if (server_running) {
        server_running = false;
        if (listen_fd >= 0) {
            shutdown(listen_fd, SHUT_RDWR);
        }
    }
    pthread_mutex_unlock(&server_mutex);
}