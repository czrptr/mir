cmake_minimum_required(VERSION 3.18)

project(mir C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# FMT

set(VENDOR_FMT ${CMAKE_SOURCE_DIR}/vendor/fmt/)

add_subdirectory(${VENDOR_FMT})

include_directories(${VENDOR_FMT}/include)

# DOCTEST

set(VENDOR_DOCTEST ${CMAKE_SOURCE_DIR}/vendor/doctest/)

include_directories(${VENDOR_DOCTEST}/include)

# MIR

set(MIR_SOURCES
  ${CMAKE_SOURCE_DIR}/source/Main.cpp)

add_executable(mir ${MIR_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/source)

add_dependencies(mir fmt)

target_link_libraries(mir fmt)

# test

set(TEST_SOURCES
  ${CMAKE_SOURCE_DIR}/test/Main.cpp)

add_executable(test ${TEST_SOURCES})