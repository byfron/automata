cmake_minimum_required(VERSION 2.8)
set(PROJECT_NAME_STR automata)

#CXX FLAGS
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -std=c++11 -pedantic -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(src)
add_subdirectory(tests)
