cmake_minimum_required(VERSION 3.11)
project(muxpp CXX)

set(EVENT__DISABLE_OPENSSL ON)
set(EVENT__DISABLE_SAMPLES ON)
set(EVENT__DISABLE_TESTS ON)
set(EVENT__LIBRARY_TYPE static)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-party/libevent)

include_directories($(PROJECT_SOURCE_DIR)/third-party/spdlog/include)
include_directories($(PROJECT_SOURCE_DIR)/third-party/libevent/include)
include_directories($(PROJECT_SOURCE_DIR)/third-party/json/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -O0 -Wall -Werror")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

add_executable(muxpp muxpp.cpp)
target_link_libraries(muxpp event_extra event_core)
