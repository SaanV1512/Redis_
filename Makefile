CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -pthread -Iinclude
SERVER_SRC = src/main.c src/server/server.c src/database.c src/hashmap.c src/persistence.c src/commands/command_dispatcher.c src/protocol/resp_parser.c src/threadpool.c src/utils.c
CLIENT_SRC = src/client.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

TARGET = redis-clone
CLIENT = redis-client

.PHONY: all clean test
all: $(TARGET) $(CLIENT)

$(TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ)

$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(CLIENT) $(SERVER_OBJ) $(CLIENT_OBJ) tests/*.o tests/*.out

test: all
	./tests/run_tests.sh
