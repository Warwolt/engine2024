cmake_minimum_required(VERSION 3.24.2)
project(stb_image_write)

add_library(stb_image_write STATIC stb_image_write.c)
target_include_directories(stb_image_write PUBLIC include)
