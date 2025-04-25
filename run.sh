#!/bin/sh
set -e
if [ "$1" = "--profile" ]; then
    shift
    echo "Testing..."
    g++ -std=c++20 -Wall -Wextra -g tests/test.cpp $(find src -type f -name "*.cpp" ! -name "main.cpp") -Iinclude -o test.out
    ./test.out $@
else
    g++ -std=c++20 -Wall -Wextra src/*.cpp -Iinclude -o taproot.out
    ./taproot.out $@
fi