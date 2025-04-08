#!/bin/sh
set -e
g++ -std=c++20 -Wall -Wextra src/*.cpp -o Taproot.out
./Taproot.out $@