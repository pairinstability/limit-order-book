set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(client Client.cpp)
target_link_libraries(client PRIVATE limitorderbook)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(server Server.cpp)
target_link_libraries(server PRIVATE limitorderbook)