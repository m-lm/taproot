# Documentation

- [Design](#design)
- [Storage](#storage)
- [Performance](#performance)
- [Future](#future)

## Design

Taproot is designed to be simple and easy to understand. The motivation for this project was to learn more about storage systems; key-value stores are one of the fundamental database types – many systems are built on top of them. I wanted mine to operate similarly to [Redis](https://redis.io/), which is why I chose an append-only log-based file storage as the persistence medium along with network communication as an interface option. Since this project is meant to be educational, I kept dependency usage to a minimum.

#### Interfaces

Taproot currently supports command-line interface and will soon be able to communicate via sockets.

#### Dependencies

Taproot utilizes the Asio and LZ4 libraries to offload the work that detracts from its didactic focus. For example: initially, the LZW compression algorithm was naively self-implemented. On a write test of 1 million key-value pairs, it resulted in a file size reduction of up to ~80% when applied to the `.db` binary snapshot, presumably because of the highly redundant nature of the data. However, it soon became apparent that relying on hand-implemented solutions would be too inflexible and suboptimal: on my M1 MacBook Air, it took a total of 12003ms. Though LZ4 does not boast the best compression ratio, it is extremely fast. Since Taproot is a Redis-style store, it is permissible to outsource the compression to a proper battle-tested library, as Redis does not typically use compression on its logs anyway.

All third-party libraries are directly integrated from their respective GitHub repositories; no special package managers are required. The benefit to this is simplicity, which I deemed more valuable for my purposes than the long-term maintainability I would gain from setting up a proper package manager. Since I plan to only use a few external libraries, importing the header files is simpler than grappling with the likes of vcpkg or other package managers. 

## Storage 

Taproot currently uses the built-in C++ hash tables (`unordered_map`) provided in the standard library for O(1) reads/writes of strings as well as Redis-style log storage consisting of `.aof` Append-Only Files and `.db` binary snapshots. 

#### Append-Only Files

Append-Only Files (AOFs) are essentially log files which append the mutable query commands (like `PUT` and `DEL`) that were entered during runtime. At shutdown, the program checks to see if the AOF file exceeds a predefined size threshold (typically 100 megabytes); if it does, the AOF is compacted by writing the in-memory state of the store to a temporary file, which is then renamed to replace the actual AOF. Compaction involves updating the final state of each key-value pair according to what commands mutated them, thus rendering the file smaller. For example, a file containing "PUT foo bar" and "PUT foo buzz" would only contain "PUT foo buzz" after compaction. Compaction will make the file smaller at the expense of some overhead during runtime. However, this overhead comes with the benefit that the files representing the state of the store will potentially be significantly smaller, and when it comes time to replay the files to reinstate the data, the time it takes will also be much faster. Note that operations that set a lot of unique data pairs without modification will not benefit much from compaction, because compaction principally takes advantage of redundant query commands. In situations where a many mutations occur after data ingestion, compaction is very effective at reducing the storage footprint.

#### Binary Snapshots

Snapshots are binary-serialized versions of the compacted state of the keyspace, where all the key-value pairs are stored. In this case, each keyspace is tied to a hash table in memory, and will have its own separate files allocated according to the user-given name of the keyspace. The snapshots are denoted by the `.db` file extensions and are meant to resemble the Redis Database (RDB) files. By default, the snapshots occur at shutdown – the same time the compacted AOF is generated. But there will be options to customize when snapshots as well as compaction occur. Once the state is compacted, it is held in memory for binary serialization. If the compacted state exceeds a predefined threshold, Taproot leverages LZ4 compression via Yann Collett's third-party library to optimally compress the binary – it does not compress anything smaller because the overhead introduced would render the speedup ineffective. LZ4 was designed to be a faster version of the LZ77 compression algorithm and is used in many modern systems. As a result, the binary snapshots can potentially be significantly smaller than plaintext AOFs and can be compressed extremely fast, resulting in little to no overhead. LZ4 was an instance where I thought it prudent to offload compression to a library expertly designed for this purpose: my own implementation would not be even remotely as efficient, and would distract me from my focus on the database system as a whole. Case in point: I originally implemented a naive version of the related LZW compression algorithm. Switching to LZ4 resulted in a [~22x speedup](#Performance)

Compaction can be very effective, but write-heavy workloads may experience diminishing returns. This is why compaction is supplemented by compression during serialization to prevent bloat as long as the file size is large enough.

#### Original Design

Originally, three file types were used: changelogs, AOFs, and binary snapshots. Changelogs were the raw, human-readable, and uncompacted logs that saved all command usage; it was used mostly as a historical record of the database. The AOFs were still used to replay the store on startup to persist data because they were also compacted versions of the changelog. However, they were also timestamped on each save, and only when a change actually occurred so as to not waste time overwriting the same data. The timestamped AOFs enabled backups up to a predefined constant (e.g., 5), past which the oldest one is overwritten. If the latest AOF was deleted or missing, Taproot would defer to the next most recent AOF. If all of the AOFs used to replay the state of the data on startup were missing, the changelog would be used as the replay file; if that changelog was also missing, all data (besides the binary snapshot) would be lost. If instead the changelog was missing at the start, the AOFs would be used to catch the changelog up. Juggling these three file types became needlessly convoluted, and did not seem to take after Redis, which typically only uses AOFs and DB files. I also anticipated more difficulty introducing concurrency into this paradigm. Thus, I decided to eschew my original three-file system in favor of the simpler two-file system. Here is the Redis documentation for further details: [Redis persistence](https://redis.io/docs/latest/operate/oss_and_stack/management/persistence/).

An example of a problem that became a very real issue during development was this: How do I keep the three different files synced? If the program runs and is interacted with by the user as intended, this is not necessarily a cause for concern. But what happens when the latest AOFs are missing and the changelog is up-to-date? The keyspace states of each file would be out of sync because the AOFs would be stale. On startup, the system would read the AOF and have the outdated data. On shutdown, any mutable commands run would be appended to the changelog indiscriminately. At this point, the changelog is corrupted because consensus is not reached. This problem was the primary reason I decided to use two files – it also happens to be closer to the way Redis implements its storage.
 
As of August 2025, there is no longer any timestamping or log rotation: AOF writes basically happen in-place. In the future, I am considering introducing it as an option within a config system.

#### Data Structures

Taproot uses the built-in hash table included with the C++ standard library to prioritize speed over sorted queries; in the future I plan to implement my own hash tables as an educational exercise, and my own skip list to handle sorted data.

## Performance

This section is used to evaluate approximate performance based on generated dummy data. This is a record of some major improvements I noticed when changing a part of the system. This is not meant to be rigorous.

---

_During log compaction, Taproot previously read the full changelog to get the final state needed to write a compacted AOF. The problem with this is that it relies on the existence of a changelog and is less efficient than using the in-memory final state due to the fact that there will be significantly more appended commands in the changelog than there are items stored in-memory. Basic write-tests showed about a 30% faster compaction rate (~5.5sec decrease for 1 million dummy writes) when the in-memory state along with a string buffer was used instead. Another problem was data would be written to the compacted AOF even if nothing changed. A flag was added to detect if any change took place since the initial load to prevent this from happening, resulting in faster read-only usage. Altogether, these optimizations made the system snappier._

Naive rebuilding from AOF: 4ms, 18350ms, 3ms  
Pass state: 4ms, 13348ms, 2ms  
Append buffer + No string formatting: 2ms, 12815ms, n/a  

**Result from naive rebuild to buffer + pass state: ~30% time decrease.**

---

_This is comparing my own naively self-implemented version of LZW for use in compressing binary snapshots. The test is on nonrandom generated data._

**Binary snapshot serialization + compression 1M simple writes (nonrandom data):**

Naive LZW compression: 12003ms  
Default LZ4 library compression: 568ms  

---

_This has to do with the append buffer for query commands. "Always" will flush immediately; this is the default output and write behavior in C++, while "Everysec" will append to the AOF by flushing every second via a background thread. In between seconds, any commands executed are stored in a temporary string buffer. "Everysec" is faster but may potentially lose data in the worst case scenario (up to last second) if the buffer is not flushed. "Always" introduces overhead from flushing on every single append but will protect data from being lost._

**Everysec vs always flushing simple writes (nonrandom data):**

Everysec + No string formatting: 4447ms  
Everysec + String formatting: 5837ms (good durability tradeoff)  
Always + String formatting 7539ms (max durability)  

**Result from always flushing to concurrent everysec flushing: ~22% time decrease.**

_After adding everysec background thread, shutdown would take around 500ms on average due to the 1s interval wait. Adding an std::conditional_variable to immediately notify the thread on closeLog() prevented the extra unnecessary wait and dropped the time down from a sporadic range of [5ms, 1s] (often 500-600ms) to a consistent 3ms for normal workflows and 0ms for read-only workflows (due to false dirty flag)._

---

**Random 16byte:96byte key-value pairs at 1 million writes:**

Everysec + No string formatting write: 17727ms

LZW compression: 93033ms (118MB down to 72.5MB)  
LZ4 compression: 2542ms (118MB down to 118MB, same because of random data)  

## Future

I find database management and information retrieval systems very interesting and engaging. I am always seeking to learn more. At its core, Taproot is intended to be a Redis-style key-value store and serves as my first foray into these areas. But it also provides a developmental test bed for future ideas that are more ambitious but potentially worth implementing, such as:

- Skip lists for sorted queries
- JSON document data modeling w/ search