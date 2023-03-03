# file: ThirdParty/ENet/CMakeLists.txt
# date: 06/02/2023
# author: Arves100
# brief: enet cmake processor
# see: https://github.com/nxrighthere/ENet-CSharp/tree/master/Source/Native

add_library(enet STATIC enet.c enet.h)
target_include_directories(enet PUBLIC ${CMAKE_CURRENT_LIST_DIR})
