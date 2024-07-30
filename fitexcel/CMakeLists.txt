cmake_minimum_required(VERSION 3.22)
project(fitexcel)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)
add_compile_options(-Wall -pedantic -Wno-long-long -Wextra )
add_link_options()
#link_directories(${CMAKE_SOURCE_DIR}/x86_64-linux-gnu)

add_executable(fitexcel
        CNode.h
        CPos.cpp
        CPos.h
        CSpreadsheet.cpp
        CSpreadsheet.h
        expression.h
        solution.cpp
        CASTBuilder.h)

target_link_libraries(fitexcel ${CMAKE_SOURCE_DIR}/x86_64-linux-gnu/libexpression_parser.a)

