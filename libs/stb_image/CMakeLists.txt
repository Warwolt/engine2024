cmake_minimum_required(VERSION 3.24.2)
project(stb_image)

add_library(stb_image STATIC stb_image.c)
target_include_directories(stb_image PUBLIC include)
