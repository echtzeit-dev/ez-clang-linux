cmake_minimum_required(VERSION 3.13)
project(ez-clang-linux)

add_executable(ez-clang-linux-socket
  main.cpp
  src/abi.cpp
  src/assert.cpp
  src/protocol.cpp
  src/raspi32.cpp
  src/response.cpp
  src/serialize.cpp
  src/symbols.cpp
)

target_include_directories(ez-clang-linux-socket PRIVATE include)
target_link_libraries(ez-clang-linux-socket PRIVATE dl)
target_link_options(ez-clang-linux-socket PRIVATE -rdynamic)

set(EZ_RESTART_PORT 10819)
set(EZ_RESTART_BINARY ez-clang-linux-socket)
configure_file(ez-clang-linux.service.in ez-clang-linux-socket.service @ONLY)
