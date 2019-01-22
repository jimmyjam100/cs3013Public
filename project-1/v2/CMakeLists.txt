cmake_minimum_required(VERSION 3.13)
project(v2 C)
set(CMAKE_C_STANDARD 11)

find_package (Threads)
add_executable(mc2 main.c)
target_link_libraries(mc2 ${CMAKE_THREAD_LIBS_INIT})
