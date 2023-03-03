# file: tests/static_test/CMakeLists.txt
# date: 16/02/2023
# author: Arves100
# brief: ENetPlay static test cmake processor


## --- this are static internal tests about the library itself! not operation tests

add_executable(server_test server_test.cpp)
target_link_libraries(server_test PRIVATE ENetPlay)

add_executable(client_test client_test.cpp)
target_link_libraries(client_test PRIVATE ENetPlay)
