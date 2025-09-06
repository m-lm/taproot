# Examples

- [Quickstart](#quickstart)
- [Commands](#commands)

## Quickstart

To get started, you can clone the repository and then run the `run.sh` script via the command-line as follows: `./run.sh --cli`.

```
Enter keyspace to use
> MyDB

            taprootdb
    ----------------------------------
    ----------------------------------

    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mput    → store multiple values by key
    mdel    → delete multiple values by key
    mget    → retrieve multiple values by key

    show    → display all key-value pairs
    keys    → display all keys
    values  → display all values

    use     → switch keyspace
    stats   → display stats
    help    → display commands
    quit    → close the program


tap> show

============
| Key-values for keyspace: 'MyDB'

============

tap> put shape circle

tap> put "alan turing" "computer scientist"

tap> show

============
| Key-values for keyspace: 'MyDB'

"alan turing": "computer scientist"
shape: circle
============

tap> mput abc 123 "hello world" "this is a test" "foo bar" fizzbuzz

tap> show

============
| Key-values for keyspace: 'MyDB'

"hello world": "this is a test"
abc: 123
"alan turing": "computer scientist"
"foo bar": fizzbuzz
shape: circle
============

tap> del abc

tap> mdel "hello world" "foo bar"

tap> show

============
| Key-values for keyspace: 'MyDB'

"alan turing": "computer scientist"
shape: circle
============

tap> get shape

============
circle
============

tap> get "alan turing" shape

Invalid operator usage: 'get' (get <KEY>)

tap> mget "alan turing" shape

============
"alan turing": "computer scientist"
shape: circle
============

tap> keys

============
| Keys for keyspace: 'MyDB'

"alan turing"
shape
============

tap> values

============
| Values for keyspace: 'MyDB'

"computer scientist"
circle
============

tap> use NewDB

Switched keyspace to NewDB

            taprootdb
    ----------------------------------
    ----------------------------------

    put     → store value by key
    del     → delete key-value pair
    get     → retrieve value by key
    mput    → store multiple values by key
    mdel    → delete multiple values by key
    mget    → retrieve multiple values by key

    show    → display all key-value pairs
    keys    → display all keys
    values  → display all values

    use     → switch keyspace
    stats   → display stats
    help    → display commands
    quit    → close the program


tap> show

============
| Key-values for keyspace: 'NewDB'

============

tap> mput hello world "this is" "a new database"

tap> put "claude shannon" "electrical engineer"

tap> show

============
| Key-values for keyspace: 'NewDB'

"this is": "a new database"
"claude shannon": "electrical engineer"
hello: world
============

tap> del "this is"

tap> show

============
| Key-values for keyspace: 'NewDB'

"claude shannon": "electrical engineer"
hello: world
============

tap> quit

Goodbye
```

## Commands
