#!/bin/sh
set -e

if [ ! -d build ]; then
    # If build/ is missing
    echo "Setting up CMake..."
    cmake -S . -B build
fi

echo "Building..."
cmake --build build -j$(sysctl -n hw.logicalcpu)

if [ "$1" = "--test" ]; then
    shift
    echo "Testing..."
    ./build/test $@
else
    echo "Running..."
    ./build/taproot $@
fi