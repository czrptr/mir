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

# MIR

set(MIR_SOURCES
  ${CMAKE_SOURCE_DIR}/source/parsing/Intern.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/Tokenizer.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/Operator.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/ast/Utils.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/ast/TokenExpressions.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/ast/TypeExpression.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/ast/FunctionExpression.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/ast/LetStatement.cpp
  ${CMAKE_SOURCE_DIR}/source/parsing/Parser.cpp)

add_executable(mir
  ${CMAKE_SOURCE_DIR}/source/Main.cpp
  ${MIR_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/source)

add_dependencies(mir fmt)

target_link_libraries(mir fmt)

# TEST

# TODO build mir into static lib and link it into mir and test
# to avoid double building it for MIR and TEST

set(TEST_SOURCES
  ${CMAKE_SOURCE_DIR}/test/Main.cpp
  ${CMAKE_SOURCE_DIR}/test/Tokenizer.cpp
  ${MIR_SOURCES})

add_executable(test ${TEST_SOURCES})

add_dependencies(test fmt)

target_link_libraries(test fmt)

target_include_directories(test PRIVATE ${VENDOR_DOCTEST}/include)