#!/bin/sh
set -e
if [ "$1" = "--profile" ]; then
    shift
    echo "Testing..."
    g++ -std=c++20 -Wall -Wextra -g tests/test.cpp $(find src -type f -name "*.cpp" ! -name "main.cpp") -o test.out
    ./test.out $@
    fi
else
    g++ -std=c++20 -Wall -Wextra src/*.cpp -o taproot.out
    ./taproot.out $@
fi