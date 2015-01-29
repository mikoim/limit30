cmake_minimum_required(VERSION 2.8.4)
project(limit30)

set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads)

add_executable(client client.c socket.c packet.c)
add_executable(server server.c socket.c packet.c auth.c game.c linkedList.c)

target_link_libraries (server ${CMAKE_THREAD_LIBS_INIT})