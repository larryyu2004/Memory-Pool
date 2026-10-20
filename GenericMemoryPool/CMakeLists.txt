cmake_minimum_required(VERSION 3.10)

project(GenericMemoryPool LANGUAGES CXX)

add_executable(GenericMemoryPool src/main.cpp)

target_include_directories(GenericMemoryPool PRIVATE include)

target_compile_features(GenericMemoryPool PRIVATE cxx_std_17)