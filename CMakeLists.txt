cmake_minimum_required(VERSION 3.15...4.0)
project(TaprootDB VERSION 1.0
                  DESCRIPTION "In-memory key-value store"
                  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lz4 STATIC third-party/lz4/lz4.c)

add_executable(taproot
  src/main.cpp
  src/db.cpp
  src/log.cpp
  src/query.cpp
  src/utils.cpp
  third-party/lz4/lz4.c
)

target_include_directories(taproot PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third-party/lz4
)

target_link_libraries(taproot PRIVATE lz4)
