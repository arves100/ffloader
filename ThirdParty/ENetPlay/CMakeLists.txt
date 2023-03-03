# file: CMakeLists.txt
# date: 16/02/2023
# author: Arves100
# brief: ENetPlay main cmake processor

cmake_minimum_required(VERSION 3.20)
project(ENetPlay)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty)
add_subdirectory(source)

if (NOT ENETPLAY_NO_TESTS)
    add_subdirectory(tests)
endif()
