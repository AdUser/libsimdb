set(CNAME "libimgdup")
set(VERSION 0.3)

project($CNAME C)
cmake_minimum_required(VERSION 2.6)

include(CTest)

set(CMAKE_INSTALL_PREFIX "/usr/local")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c99")
add_definitions("-D_XOPEN_SOURCE=500")

add_subdirectory(src)
add_subdirectory("tests")
set_property(DIRECTORY "tests" PROPERTY COMPILE_FLAGS "-g;-ggdb;-Wall;-Wextra;-pedantic;-O0")
