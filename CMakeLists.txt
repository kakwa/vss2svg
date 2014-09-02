cmake_minimum_required (VERSION 2.6)

project (vss2svg)

LINK_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/lib
    /usr/local/lib
    /usr/lib/
)

include_directories(
    /usr/include/ 
    /usr/include/librevenge-0.0/ 
    /usr/include/libvisio-0.1
)



add_executable(vss2svg src/conv/vss2svg.cpp)

target_link_libraries(vss2svg revenge-0.0 visio-0.1 revenge-stream-0.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

