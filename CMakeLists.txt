cmake_minimum_required(VERSION 3.13)
project(toy_miniSysY)

set(CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "src/*.h")
add_executable(toy_miniSysY ${source_files} ${header_files} src/frontend/parser.h src/frontend/parser.cpp src/structure/ast.cpp src/structure/ast.h src/util/dbg.h)

#add_executable(toy_miniSysY src/main.cpp util/common.cpp util/common.h)