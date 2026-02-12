cmake_minimum_required(VERSION 3.5.0)
project(External VERSION 0.1.0 LANGUAGES C CXX)

add_library(External STATIC)

target_include_directories(External PUBLIC include/)
target_sources(External PRIVATE src/stb_image_write.cpp)
