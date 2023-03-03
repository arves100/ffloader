# file: source/CMakeLists.txt
# date: 06/02/2023
# author: Arves100
# brief: ENetPlay library cmake processor

add_library(ENetPlay STATIC
    ${PUBLIC_INCS}
    StdAfx.h
    Instance.h
    Peer.h
    Server.cpp
    Server.h
    Client.h
    Client.cpp
    Msg.h
)

target_include_directories(ENetPlay PRIVATE ${CMAKE_CURRENT_LIST_DIR} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/public)
target_link_libraries(ENetPlay PRIVATE enet concurrentqueue ankerl)
target_precompile_headers(ENetPlay PRIVATE StdAfx.h)
