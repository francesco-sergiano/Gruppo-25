cmake_minimum_required(VERSION 3.13)
project(MyPass LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(MyPass SHARED MyPass.cpp)
target_link_libraries(MyPass PRIVATE LLVM)
