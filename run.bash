#!/bin/bash
set -e
g++ main.cpp matrix.cpp -std=c++11
./a.out $*