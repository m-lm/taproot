#!/bin/bash
set -e
g++ -std=c++20 -Wall -Wextra src/*.cpp
./a.out $*