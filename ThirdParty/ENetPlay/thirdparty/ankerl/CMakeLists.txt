# file: ThirdParty/ankerl/CMakeLists.txt
# date: 06/02/2023
# author: Arves100
# brief: unordered dense cmake processor
# see: https://github.com/martinus/unordered_dense

add_library(ankerl INTERFACE unordered_dense.h)
target_include_directories(ankerl INTERFACE ${CMAKE_CURRENT_LIST_DIR})
