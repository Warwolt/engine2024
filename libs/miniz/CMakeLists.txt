cmake_minimum_required(VERSION 3.24.2)
project(miniz)

add_library(miniz STATIC miniz.c)
target_include_directories(miniz PUBLIC include)
