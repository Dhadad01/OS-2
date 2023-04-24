cmake_minimum_required(VERSION 3.1)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
project(threads VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE Debug)

add_library(uthreads uthreads.h uthreads.cpp utils.h utils.cpp dispatcher.h dispatcher.cpp scheduler.h scheduler.cpp thread_control_block.h thread_control_block.cpp globals.cpp)

set_property(TARGET uthreads PROPERTY CXX_STANDARD 11)
target_compile_options(uthreads PUBLIC -Wall -Wextra)

add_subdirectory(tests)
