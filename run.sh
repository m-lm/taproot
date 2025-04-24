#!/bin/sh
set -e
if [ "$1" = "--profile" ] && [ "$(uname)" = "Darwin" ]; then
    shift
    echo "Profiling performance..."
    g++ -std=c++20 -Wall -Wextra -g src/*.cpp -o taproot.out
    ./taproot.out $@
    open -a Instruments
else
    g++ -std=c++20 -Wall -Wextra src/*.cpp -o taproot.out
    ./taproot.out $@
fi