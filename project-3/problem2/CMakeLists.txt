cmake_minimum_required(VERSION 3.13)
project(problem2 C)

set(CMAKE_C_STANDARD 11)

find_package (Threads)
add_executable(problem2 main.c)
target_link_libraries(problem2 ${CMAKE_THREAD_LIBS_INIT} m)
set(CMAKE_C_FLAGS "-m32")
