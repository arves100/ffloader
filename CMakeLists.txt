# file: CMakeLists.txt
# date: 06/02/2023
# author: Arves100
# brief: Cmake processor

cmake_minimum_required(VERSION 3.20)
project(FFLoader)
set(GLOBAL PROPERTY USE_FOLDERS ON)

set(LOADER_VERSION "2.0")

# Sources

set(RES_SRC resource.h Resource.rc)
set(UNSRT_SRC DllMain.cpp StdAfx.h)

# Groups

# Version generation
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(GENERATED_FILE ${GENERATED_DIR}/Version.h)
generate_file(Version.h.in ${GENERATED_FILE})

# Project 

add_library(NetLib DYNAMIC
    ${RES_SRC}
    ${UNSRT_SRC}
    ${GENERATED_FILE}
)

target_link_libraries(NetLib PRIVATE Detours ENetPlay)
target_precompile_headers(NetLib PRIVATE StdAfx.h)
target_include_directories(NetLib PRIVATE ${GENERATED_DIR})
