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
- Command-line and client-server interfaces (still in development)
- Durable, fast, and human-readable append-only logs for persistent storage
- Compaction, serialization, and LZ4 compression for binary snapshots
- Minimal external dependencies

## Backlog

- Implement multi-word values
- New commands (mset, mdel, stats, use)
- Client-server networking via TCP sockets
- Concurrent writes and log compaction (currently, compaction only occurs on shutdown)
- Implement own hash table
- Data import and export
- Config options

## Setup

This project can be run in multiple ways; while in root, simply run `./run.sh` to launch the key-value server which will listen for any connection attempts by a client. To run the command-line interface as a client along with the Taproot server, run `./run.sh --cli`; this will spawn a background process that will run until the client quits, at which point the process is terminated. Finally, enter `./run.sh --test` to conduct basic testing as defined in `tests/test.cpp`.

Taproot uses a basic configuration file named `config.cfg` that lives in the root to personalize certain details about how it operates. Most notably, the host address and the port number (among other settings) are contained in this file and will automatically spawn with the default settings if the file has not yet been created by the user. The format of the config file is chosen for simplicity: it is in the form of KEY=VALUE, and each pair is separated by a newline. You can go in and edit the config file as needed, but you can also overwrite the settings at runtime by passing in arguments at the command-line. For example, running `./run.sh --cli 127.0.0.1 6379` will set the host address and port number as "127.0.0.1" and "6379" respectively if it is not set already. Finally, the exact name of the config file can be changed by going into `src/main.cpp` and changing `configFilename`, which is currently hard-coded.

## Documentation

See [documentation](docs.md).

## Acknowledgments

This toy project is inspired by [Redis](https://github.com/redis/redis), and makes use of [LZ4](https://github.com/lz4/lz4) for compression and [Asio](https://github.com/chriskohlhoff/asio) for networking.

## License

Taproot is under the GPL-3.0 license. See [LICENSE](LICENSE) for more details.