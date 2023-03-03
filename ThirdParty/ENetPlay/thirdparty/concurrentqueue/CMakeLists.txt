# file: ThirdParty/concurrentqueue/CMakeLists.txt
# date: 06/02/2023
# author: Arves100
# brief: concurrent queue cmake processor
# see: https://github.com/cameron314/concurrentqueue

add_library(concurrentqueue INTERFACE concurrentqueue.h)
target_include_directories(concurrentqueue INTERFACE ${CMAKE_CURRENT_LIST_DIR})
