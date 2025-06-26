cmake_minimum_required(VERSION 3.16)
project(ToshibaT100)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

set(NFD_PORTAL ON)
add_subdirectory(deps/nativefiledialog-extended)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/deps/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS}/imgui_impl_sdl2.cpp
    ${IMGUI_BACKENDS}/imgui_impl_sdlrenderer2.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEB)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(BUILD_REL)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(BUILD_RELWITHDEBINFO)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_definitions(BUILD_MINSIZEREL)
endif()


file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_executable(ToshibaT100
    ${IMGUI_SOURCES}
    ${SOURCES}
    deps/z80/z80.c
)
# target_compile_options(ToshibaT100 PRIVATE
    # $<$<CONFIG:Debug>:-fsanitize=address -g -O1>
# )
# target_link_options(ToshibaT100 PRIVATE
    # $<$<CONFIG:Debug>:-fsanitize=address>
# )


set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(DEST_DIR     "${CMAKE_BINARY_DIR}/resources")
add_custom_command(
    TARGET ToshibaT100 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${RESOURCE_DIR} ${DEST_DIR}
    COMMENT ""   
)

target_include_directories(ToshibaT100 PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
    include/
    deps/nativefiledialog/src/include
    deps/stb
    deps/z80
    deps/include # For z80_cpp
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS}
)

target_link_libraries(ToshibaT100 PRIVATE
    ${SDL2_LIBRARIES}
    ${GTK3_LIBRARIES}
    nfd
)
