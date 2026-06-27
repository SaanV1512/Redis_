# ConcurrentKV

> A concurrent in-memory key-value database built from scratch in C using TCP sockets, POSIX threads, and custom hash tables.

ConcurrentKV is a lightweight key-value database designed to explore systems programming concepts including socket programming, multithreading, synchronization, persistence, and in-memory data structures. Inspired by the architecture of modern in-memory databases such as Redis, the project focuses on understanding how network servers process requests, manage shared state, and provide fast key-value operations.

---

## Features

* Multi-client TCP server
* Concurrent request handling using POSIX threads (`pthread`)
* Thread-safe in-memory hash table
* Persistent storage support
* Key expiration (TTL)
* RESP-compatible command parsing (basic support)
* Modular architecture with separated networking, storage, parsing, and persistence layers

---

## Supported Commands

| Command              | Description                 |
| -------------------- | --------------------------- |
| `PING`               | Health check                |
| `SET key value`      | Store a key-value pair      |
| `GET key`            | Retrieve a value            |
| `DEL key`            | Delete a key                |
| `EXISTS key`         | Check if a key exists       |
| `KEYS`               | List all keys               |
| `DBSIZE`             | Number of stored keys       |
| `FLUSHDB`            | Clear the database          |
| `EXPIRE key seconds` | Set key expiration          |
| `TTL key`            | Remaining lifetime of a key |
| `SAVE`               | Persist database to disk    |

---

## Architecture

```text
                 +----------------------+
                 |      Client          |
                 +----------+-----------+
                            |
                        TCP Socket
                            |
                 +----------v-----------+
                 |   ConcurrentKV       |
                 |      Server          |
                 +----------+-----------+
                            |
                 Accept Client Connection
                            |
                 Spawn Worker Thread
                            |
                 Parse Command
                            |
                 Execute Database Operation
                            |
              Thread-safe Hash Table Storage
                            |
                     Send Response
```

---

## Project Structure

```text
server.c         -> TCP server and connection management
client.c         -> Interactive client application
commands.c       -> Command parsing and execution
database.c       -> Hash table implementation
persistence.c    -> Save/load database
resp.c           -> RESP protocol parser
utils.c          -> Helper utilities
```

---

## Building

```bash
make
```

Run the server:

```bash
./redis-clone
```

Run the client:

```bash
./redis-client
```

---

## Example

```text
> PING
PONG

> SET NAME Saanvi
OK

> GET NAME
Saanvi

> DBSIZE
1

> EXPIRE NAME 60
1

> TTL NAME
59
```

---

## Design Highlights

* Implemented a custom hash table for constant-time average lookup and insertion.
* Supports multiple concurrent client connections through a thread-per-client architecture.
* Uses mutexes to synchronize access to shared in-memory data structures.
* Organized into independent networking, parsing, storage, and persistence modules for maintainability and extensibility.

---

## Future Work

* Replace the thread-per-client model with Linux `epoll` for scalable event-driven networking.
* Improve RESP compatibility.
* Add support for additional data structures (Lists, Sets, Hashes).
* Implement Append-Only File (AOF) persistence.
* Benchmark throughput and latency under concurrent workloads.

---

## Motivation

The goal of this project was not to reproduce Redis feature-for-feature, but to understand the systems programming concepts involved in building a concurrent networked database from scratch. The implementation emphasizes modular design, low-level networking, synchronization, and efficient in-memory storage while serving as a foundation for exploring more advanced event-driven architectures.
