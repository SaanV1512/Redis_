#include "server.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

static bool keep_running = true;

static void handle_signal(int signum) {
    (void)signum; //ow unused parameter
    keep_running = false;
}

int main(int argc, char *argv[]) {
    const char *filename = "dump.rdb";
    unsigned short port = 6379;

    if (argc > 1) {
        char *end = NULL;
        unsigned long parsed = strtoul(argv[1], &end, 10);
        if (end != argv[1] && *end == '\0' && parsed > 0 && parsed <= 65535) {
            port = (unsigned short)parsed;
        }
    }

    if (!database_init(filename)) {
        fprintf(stderr, "Failed to initialize database from %s\n", filename);
    }

    signal(SIGINT, handle_signal); //CTRL+C
    signal(SIGTERM, handle_signal); //Termination signal
    signal(SIGPIPE, SIG_IGN); //ignore SIGPIPE to prevent server termination on client disconnect
    //SIGTERM allows us to gracefully shutdown the server when running in a container
    //SIGKILL immediately terminates the process and cannot be caught or handled

    if (!server_start(port)) {
        fprintf(stderr, "Server failed to start on port %u\n", port);
        database_shutdown();
        return EXIT_FAILURE;
    }

    while (keep_running) {
        sleep(1); //stay alive and wait for signals
    }

    server_stop(); //stop accepting new clients and close sockets and disconnect clients
    database_save();
    database_shutdown();
    return EXIT_SUCCESS;
}
