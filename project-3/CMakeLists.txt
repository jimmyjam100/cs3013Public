cmake_minimum_required(VERSION 3.13)
project(project_3 C)

set(CMAKE_C_STANDARD 11)

find_package (Threads)
add_executable(project_3 main.c)
target_link_libraries(project_3 ${CMAKE_THREAD_LIBS_INIT} m)
