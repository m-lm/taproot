# Taproot

Taproot is a simple in-memory key-value store written in C++ with minimal dependencies, supporting data persistence and fast in-memory reads/writes.

-----

Taproot was developed to be a lightweight Redis-like clone for educational purposes with as few dependencies as possible. The impetus for this project was my growing interest in storage engines, data structures, and database management systems (especially NoSQL ones). In the process, I learned more about C++, system design, algorithms, and data structures. This project serves as a playground for concepts I find interesting.

## Table of Contents

- [Features](#features)
- [Backlog](#backlog)
- [Setup](#setup)
- [Documentation](#documentation)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Features

- Simple Redis-style functionality
- CLI and client-server interfaces (still in development)
- Durable, fast, and human-readable append-only logs for persistent storage
- Compaction, serialization, and LZ4 compression for binary snapshots
- Minimal external dependencies

## Backlog

- Implement multi-word values
- New commands (mset, mdel, stats, use)
- Client-server interface via TCP sockets
- Concurrent writes and log compaction (currently, compaction only occurs on shutdown)
- Implement own hash table
- Data import and export
- Config options

## Setup

Currently, this project can only be run via the command-line interface; in root, simply run `./run.sh` to interact with the main program or `./run.sh --test` to run basic testing defined in `tests/test.cpp`.

## Documentation

See [documentation](docs.md).

## Acknowledgments

This toy project is inspired by [Redis](https://github.com/redis/redis), and makes use of [LZ4](https://github.com/lz4/lz4) for compression and [Asio](https://github.com/chriskohlhoff/asio) for networking.

## License

Taproot is under the GPL-3.0 license. See [LICENSE](LICENSE) for more details.