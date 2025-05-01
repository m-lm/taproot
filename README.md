# Taproot

Taproot is a simple in-memory key-value store written in C++ with minimal dependencies, supporting on-disk data persistence and fast in-memory reads/writes.

-----

Taproot was developed to be a lightweight Redis-like clone for educational purposes with as few dependencies as possible. The impetus for this project was my growing interest in storage engines, data structures, and database management systems (especially NoSQL ones) more generally. In the process, I learned more about C++, system design, algorithms, and data structures. This project serves as a playground for concepts I find interesting, and suggestions are welcome.

## Table of Contents

- [Features](#features)
- [Backlog](#backlog)
- [Future](#future)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Acknowledgements](#acknowledgements)
- [License](#license)

## Features

- Simple Redis-style functionality
- CLI and client-server interfaces (still in development)
- Fast and concurrent in-memory operations
- Durable, fast, and human-readable Append-Only Logs for persistent storage
- Compaction, binary serialization, and LZ4 compression for snapshots
- Log rotation and archiving
- Minimal external dependencies

## Backlog

- Implement multi-word values
- New commands (mset, mdel, stats, use)
- Client-server interface via TCP sockets
- Increased concurrency (real-time rotation & compaction, forked checkpointing, client write-locks)
- Move from unordered_map to ordered_map or skip list data structure for sorted queries
- Data import and export
- More unit/case testing

This is not a comprehensive list of to-dos nor is it a full list of future plans. See [Future](#future) for farther out ideas.

## Future

- Python driver
- JSON document data modeling
- MessagePack serialization
- Search queries
- AWS EC2 single-node deployment

## Installation

## Usage

## Documentation

- [Interfaces](docs/interfaces.md)
- [Architecture](docs/architecture.md)
- [Design Choices](docs/design-choices.md)
- [Storage Engine](docs/storage.md)
- [Performance](docs/performance.md)

## Acknowledgements

This toy project is inspired by [Redis](https://github.com/redis/redis), and makes use of the [LZ4](https://github.com/lz4/lz4) compression library.

## License

Taproot is under the GPL-3.0 license. See [LICENSE](LICENSE) for more details.