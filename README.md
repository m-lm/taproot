# Taproot

Taproot is a key-value store written in C++20 with minimal dependencies, supporting data persistence through append-only logs and LZ4-compressed binary snapshots with compaction for storage efficiency. It also supports client-server interfacing via TCP sockets by leveraging Asio, and emphasizes simplicity in setup, configuration, and workflow – all while being self-contained and easy to use.

-----

## Table of Contents

- [Features](#features)
- [Backlog](#backlog)
- [Setup](#setup)
- [Quickstart](#quickstart)
- [Documentation](#documentation)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Features

- Simple Redis-style functionality
- Command-line and client-server interfaces
- Human-readable append-only logs for persistent storage
- Compaction, serialization, and LZ4 compression for binary snapshots
- Minimal external dependencies

## Backlog

- [x] Implement multi-word values
- [x] New commands (mset, mdel, stats, use)
- [x] Client-server networking via TCP sockets
- [ ] Concurrent writes and log compaction (currently, compaction only occurs on shutdown)
- [ ] Implement own hash table
- [ ] Config options
- [ ] Full binary snapshot backups
- [ ] Data import and export

## Setup

This project can be run in multiple ways; while in root, simply run `./run.sh` to launch the key-value server which will listen for any connection attempts by a client. To run the command-line interface as a client along with the Taproot server, run `./run.sh --cli`; this will spawn a background process that will run until the client quits, at which point the process is terminated. Finally, enter `./run.sh --test` to conduct basic testing as defined in `tests/test.cpp`.

Taproot uses a basic configuration file named `config.cfg` that lives in the root to personalize certain details about how it operates. Most notably, the host address and the port number (among other settings) are contained in this file and will automatically spawn with the default settings if the file has not yet been created by the user. The format of the config file is chosen for simplicity: it is in the form of KEY=VALUE, and each pair is separated by a newline. 

You can go in and edit the config file as needed, but you can also overwrite the settings at runtime by passing in arguments from the command-line. For example, running `./run.sh --cli 127.0.0.1 6379` will set the host address and port number as "127.0.0.1" and "6379" respectively if it is not set already. Finally, the exact name of the config file can be changed by going into `src/main.cpp` and changing `configFilename`, which is currently hard-coded.

## Quickstart

Once the server is running and your client is connected to it, you can start entering commands.

```console
./run.sh --cli

tap> put claude shannon
tap> put alan turing
tap> put alonzo church
tap> show

alonzo: church
alan: turing
claude: shannon

tap> get claude
shannon

tap> mget alonzo alan

alonzo: church
alan: turing

tap> del alonzo
tap> show

alan: turing
claude: shannon

tap> put "hello world" goodbye
tap> put physics "quantum entanglement"
tap> show

alan: turing
claude: shannon
"hello world": goodbye
physics: "quantum entanglement"

tap> quit

Goodbye

```

More examples can be found [here](examples.md).

## Documentation

I attempted to document the development process, including how it all works and the reasoning behind the design decisions. See [documentation](docs.md).

## Acknowledgments

This toy project is inspired by [Redis](https://github.com/redis/redis), and makes use of [LZ4](https://github.com/lz4/lz4) for compression and [Asio](https://github.com/chriskohlhoff/asio) for networking.

## License

Taproot is under the GPL-3.0 license. See [LICENSE](LICENSE) for more details.