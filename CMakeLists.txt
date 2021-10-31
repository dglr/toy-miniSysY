cmake_minimum_required(VERSION 3.13)
project(toy_miniSysY)

set(CMAKE_CXX_STANDARD 20)

include(src/conv/CMakeLists.txt)
include(src/frontend/CMakeLists.txt)
include(src/structure/CmakeLists.txt)
include(src/util/CMakeLists.txt)

list(APPEND toy_miniSysY_SOURCES
        src/main.cpp
        )

add_executable(toy_miniSysY ${toy_miniSysY_SOURCES})

