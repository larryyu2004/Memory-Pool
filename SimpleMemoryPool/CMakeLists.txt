cmake_minimum_required(VERSION 3.10)

project(SimpleMemoryPool LANGUAGES CXX)

add_executable(SimpleMemoryPool src/main.cpp)

target_include_directories(SimpleMemoryPool PRIVATE include)

target_compile_features(SimpleMemoryPool PRIVATE cxx_std_17)