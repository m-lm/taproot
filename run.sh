# --- SETUP ---

#!/bin/sh
set -e

# Run CMake and create build directory
if [ ! -d build ]; then
    echo "Setting up CMake..."
    cmake -S . -B build
fi

# Build project
echo "Building..."
cmake --build build -j$(sysctl -n hw.logicalcpu)

# --- EXECUTABLE MODES ---

# Run testing on server
if [ "$1" = "--test" ]; then
    shift
    echo "Performing tests..."
    ./build/test $@
# Run client with server in the background
elif [ "$1" = "--cli" ]; then
    shift
    echo "Running client and server..."
    ./build/taproot $@ > server.log 2>&1 &
    SERVER_PID=$!
    ./build/cli $@
    if kill -0 $SERVER_PID 2>/dev/null; then
        kill $SERVER_PID
    fi
# Run the server only
else
    echo "Running server..."
    ./build/taproot $@
fi