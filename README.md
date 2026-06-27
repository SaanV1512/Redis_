# Concurrent In-Memory Key-Value Store

A lightweight in-memory key-value database built from scratch in C using TCP sockets, POSIX threads, and custom data structures. The project explores systems programming concepts such as socket programming, multithreading, synchronization, persistence, and command parsing through the implementation of a networked database server.

---

## Features

* Concurrent TCP server supporting multiple client connections
* Thread-per-client architecture using POSIX threads (`pthread`)
* In-memory hash table for efficient key-value storage
* Thread-safe database operations using mutexes
* Command parser supporting both plain text and RESP-style requests
* Persistent storage with database save functionality
* Key expiration using Time-To-Live (TTL)
* Modular codebase with separated networking, storage, parsing, and persistence layers

---

## Supported Commands

| Command              | Description                          |
| -------------------- | ------------------------------------ |
| `PING`               | Check server availability            |
| `SET key value`      | Store a key-value pair               |
| `GET key`            | Retrieve the value of a key          |
| `DEL key`            | Delete a key                         |
| `EXISTS key`         | Check whether a key exists           |
| `KEYS`               | List all stored keys                 |
| `DBSIZE`             | Return the number of stored keys     |
| `FLUSHDB`            | Remove all keys from the database    |
| `EXPIRE key seconds` | Set an expiration time for a key     |
| `TTL key`            | Get remaining lifetime of a key      |
| `SAVE`               | Persist the current database to disk |

---

## Project Structure

```text
.
├── server.c          # TCP server and client connection handling
├── client.c          # Interactive command-line client
├── commands.c        # Command parsing and execution
├── database.c        # Hash table implementation and database operations
├── persistence.c     # Database save/load functionality
├── resp.c            # RESP protocol parsing
├── utils.c           # Utility helper functions
├── include/          # Header files
├── Makefile
└── README.md
```

---

## Architecture

```
                +----------------+
                |  Redis Client  |
                +-------+--------+
                        |
                  TCP Socket
                        |
        +---------------+---------------+
        |        Server Process         |
        +---------------+---------------+
                        |
              Accept Client Connection
                        |
                 Create Worker Thread
                        |
                Parse Incoming Command
                        |
              Execute Database Operation
                        |
              Thread-Safe Hash Table
                        |
                 Send Response Back
```

Each client connection is handled by an independent worker thread. Shared access to the in-memory database is synchronized using mutexes to ensure correctness during concurrent operations.

---

## Building

```bash
make
```

---

## Running

Start the server:

```bash
./redis-clone
```

In another terminal, launch the client:

```bash
./redis-client
```

---

## Example

```text
> PING
PONG

> SET NAME Alice
OK

> GET NAME
Alice

> EXISTS NAME
1

> DBSIZE
1

> DEL NAME
1

> GET NAME
nil
```

---

## Technologies Used

* C
* POSIX Threads (`pthread`)
* TCP Socket Programming
* Hash Tables
* Mutex Synchronization
* File I/O
* Make

---

## Learning Outcomes

This project was developed to gain practical experience with:

* Concurrent server design
* TCP/IP socket programming
* POSIX multithreading
* Thread synchronization using mutexes
* Custom hash table implementation
* In-memory database design
* Modular software architecture in C

---

## Future Improvements

* Replace the thread-per-client model with an event-driven architecture using Linux `epoll`
* Improve persistence with append-only logging and snapshots
* Implement additional Redis-compatible commands and data structures
* Support transactions and pipelining
* Add benchmarking and performance profiling
* Improve RESP protocol compatibility

---

## License

This project was built for educational purposes to explore systems programming concepts and the design of in-memory databases.
