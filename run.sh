#!/bin/sh
set -e

echo "Building..."
#cd build
#make -j$(sysctl -n hw.logicalcpu)
cmake -S . -B build
cmake --build build -j$(sysctl -n hw.logicalcpu)
#cd ..

if [ "$1" = "--test" ]; then
    shift
    echo "Testing..."
    g++ -std=c++20 -Wall -Wextra -g tests/test.cpp $(find src -type f -name "*.cpp" ! -name "main.cpp") third-party/lz4/lz4.c -Iinclude -Ithird-party/lz4 -o test.out
    ./test.out $@
else
    echo "Running..."
    #g++ -std=c++20 -Wall -Wextra src/*.cpp third-party/lz4/lz4.c -Iinclude -Ithird-party/lz4 -o taproot.out
    #./taproot.out $@
    ./build/taproot $@
fi