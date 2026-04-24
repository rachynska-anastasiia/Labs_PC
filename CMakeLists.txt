cmake_minimum_required(VERSION 4.0)
project(Labs_PC)

set(CMAKE_CXX_STANDARD 20)

# Додаємо бібліотеку Winsock (ws2_32) для всього проєкту
set(WS2_32_LIB ws2_32)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

add_executable(Labs_PC main.cpp)

#add_executable(poslidovno poslidovno.cpp)

#add_executable(parallel parallel.cpp)

#add_executable(lab_2_simp lab_2_simp.cpp)

#add_executable(lab_2_mutex lab_2_mutex.cpp)

#add_executable(lab_2_atomic lab_2_atomic.cpp)

#add_executable(lab_2_atomic_optimized lab_2_atomic_optimized.cpp)

#add_executable(lab_2_mutex_optimized lab_2_mutex_optimized.cpp)

#add_executable(lab_3 lab_3.cpp)

#add_executable(lab_4_client lab_4_client.cpp)
#target_link_libraries(lab_4_client ${WS2_32_LIB})

#add_executable(lab_4_server lab_4_server.cpp)
#target_link_libraries(lab_4_server ${WS2_32_LIB})

add_executable(lab_5 lab_5.cpp)
target_link_libraries(lab_5 ${WS2_32_LIB})

add_executable(thread_pool thread_pool.cpp)
