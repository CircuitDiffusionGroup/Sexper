cmake_minimum_required(VERSION 3.31)
project(Sexper)

set(CMAKE_CXX_STANDARD 23)

add_library(Sexper STATIC)

target_sources(Sexper PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES FILES library.ixx)