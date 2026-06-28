# MemDB

A lightweight concurrent in-memory key-value database built from scratch in C.

MemDB is a systems programming project that explores the core ideas behind modern in-memory databases such as Redis. Rather than replicating Redis feature-for-feature, the project focuses on implementing the underlying building blocks of a high-performance database server using low-level C and POSIX APIs.

The server follows a client-server architecture where multiple clients communicate with a shared in-memory database over TCP sockets. Each client connection is handled by a dedicated worker thread, allowing multiple requests to be processed concurrently while maintaining data consistency through thread synchronization.

At its core, MemDB stores data inside a custom hash table implementation that provides efficient average-case lookup, insertion, and deletion of key-value pairs. Commands received from connected clients are parsed, validated, executed against the shared database, and returned through a simple command protocol with basic RESP compatibility.

## Features

* Concurrent TCP server supporting multiple simultaneous clients
* Thread-per-client architecture using POSIX threads (`pthread`)
* Thread-safe in-memory hash table protected using mutexes
* Persistent storage for saving and restoring database state
* Time-To-Live (TTL) support for automatic key expiration
* Modular architecture separating networking, command parsing, storage, persistence, and utility components
* Interactive command-line client for communicating with the server

## Supported Commands

* `PING`
* `SET`
* `GET`
* `DEL`
* `EXISTS`
* `KEYS`
* `DBSIZE`
* `FLUSHDB`
* `EXPIRE`
* `TTL`
* `SAVE`

## Architecture

MemDB is organized into several independent modules, each responsible for a specific aspect of the system.

* **Networking Layer** – Accepts TCP connections, manages client sockets, and spawns worker threads.
* **Command Parser** – Parses incoming requests and dispatches them to the appropriate database operation.
* **Database Engine** – Maintains an in-memory hash table and executes key-value operations.
* **Persistence Layer** – Saves and restores database contents from disk.
* **Utility Layer** – Provides helper functions for parsing, memory management, and common operations.

This modular design allows individual components to evolve independently while keeping the overall system maintainable and easy to extend.

## Concurrency Model

Each client connection is served by an independent POSIX thread. Since all threads access the same shared in-memory database, synchronization primitives are used to prevent race conditions and ensure thread-safe reads and writes.

This design emphasizes practical operating systems concepts including multithreading, synchronization, shared memory, and concurrent network programming.

## Motivation

The primary goal of MemDB is educational. The project was developed to gain hands-on experience with systems programming concepts that are difficult to understand through theory alone, including:

* TCP socket programming
* POSIX threads
* Thread synchronization with mutexes
* In-memory data structures
* Client-server architecture
* Concurrent request handling
* Database persistence
* Modular software design in C

Instead of relying on existing libraries or frameworks, most components were implemented from scratch to better understand how networked database systems operate internally.

## Building

```bash
make
```

## Running

Start the server:

```bash
./memdb-clone
```

In another terminal, launch the client:

```bash
./memdb-client
```

## Example

```text
> PING
PONG

> SET NAME SAANVI
OK

> GET NAME
SAANVI

> DBSIZE
1

> EXPIRE NAME 30
1

> TTL NAME
29

> SAVE
OK
```

## Future Improvements

MemDB is intended to continue evolving as more systems programming concepts are explored. Planned improvements include:

* Event-driven networking using Linux `epoll`
* Improved RESP protocol compatibility
* Append-Only File (AOF) persistence
* Additional Redis-style data structures (Lists, Sets, Hashes)
* Transactions and pipelining
* Memory optimization and benchmarking
* Performance profiling under high concurrency

## Technologies

* C
* POSIX Threads (`pthread`)
* TCP/IP Sockets
* Mutex Synchronization
* Hash Tables
* File I/O
* GNU Make

---

**Note:** MemDB is inspired by the architectural concepts of modern in-memory databases but is intentionally designed as an educational implementation focused on learning systems programming rather than reproducing every feature of Redis.
