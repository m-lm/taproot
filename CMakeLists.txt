cmake_minimum_required(VERSION 3.15...4.0)
project(TaprootDB VERSION 1.0
                  DESCRIPTION "In-memory key-value store"
                  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SHARED_SRC
  src/db.cpp
  src/log.cpp
  src/query.cpp
  src/utils.cpp
  src/config.cpp
  src/operations.cpp
  third-party/lz4/lz4.c
)

add_library(lz4 STATIC third-party/lz4/lz4.c)

# Taproot server

add_executable(taproot
  src/main.cpp
  ${SHARED_SRC}
)

target_include_directories(taproot PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third-party/lz4
  ${CMAKE_SOURCE_DIR}/third-party/asio
)

target_compile_definitions(taproot PRIVATE asio_standalone)
target_link_libraries(taproot PRIVATE lz4)

# Conducting tests

add_executable(test
  tests/test.cpp
  ${SHARED_SRC}
)

target_include_directories(test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third-party/lz4
  ${CMAKE_SOURCE_DIR}/third-party/asio
)

target_compile_definitions(test PRIVATE asio_standalone)
target_link_libraries(test PRIVATE lz4)

# Command-line interface

add_executable(cli
  src/cli.cpp
  ${SHARED_SRC}
)

target_include_directories(cli PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third-party/lz4
  ${CMAKE_SOURCE_DIR}/third-party/asio
)

target_compile_definitions(cli PRIVATE asio_standalone)
target_link_libraries(cli PRIVATE lz4)