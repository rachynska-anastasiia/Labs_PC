cmake_minimum_required(VERSION 4.0)
project(Labs_PC)

set(CMAKE_CXX_STANDARD 20)

add_executable(Labs_PC main.cpp)

add_executable(poslidovno poslidovno.cpp)

add_executable(parallel parallel.cpp)

add_executable(lab_2_simp lab_2_simp.cpp)

add_executable(lab_2_threads lab_2_threads.cpp)

add_executable(lab_2_atomic lab_2_atomic.cpp)