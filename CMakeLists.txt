cmake_minimum_required (VERSION 2.6)

project (vss2svg)

LINK_DIRECTORIES(
    ${CMAKE_BINARY_DIR}/lib
    /usr/local/lib
    /usr/lib/
)

include_directories(
    ./inc/
    /usr/include/ 
    /usr/include/librevenge-0.0/ 
    /usr/include/libvisio-0.1/
)

add_library(SVGDrawingGenerator src/lib/SVGDrawingGenerator.cpp)
add_library(EMFSVG src/lib/EMFSVG.cpp)
add_library(cdecode src/lib/cdecode.c)
add_library(cencode src/lib/cencode.c)

add_executable(vss2svg src/conv/vss2svg.cpp)

target_link_libraries(vss2svg revenge-0.0 visio-0.1 revenge-stream-0.0 SVGDrawingGenerator EMFSVG cdecode cencode)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

