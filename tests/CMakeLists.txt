cmake_minimum_required(VERSION 3.11)

project(Z80Disassembler)
add_executable(Z80Disassembler Z80Disassembler.cpp ${CMAKE_SOURCE_DIR}/src/Z80Disassembler.c)
target_include_directories(Z80Disassembler PRIVATE ${CMAKE_SOURCE_DIR}/include)