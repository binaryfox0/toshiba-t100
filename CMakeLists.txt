cmake_minimum_required(VERSION 3.16)
project(ToshibaT100)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# --- Dependencies ---

# SDL2
find_package(SDL2 REQUIRED)

# GTK3 for NativeFileDialog
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# --- Source files ---

# NativeFileDialog
set(NFD_SOURCES
    deps/nativefiledialog/src/nfd_common.c
    deps/nativefiledialog/src/nfd_gtk.c
)


# Dear ImGui
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

# Project sources
file(GLOB SOURCES src/*.c src/*.cpp)
file(GLOB HEADERS include/*.hpp)

# --- Executable ---

add_executable(ToshibaT100
    ${NFD_SOURCES}
    ${IMGUI_SOURCES}
    ${SOURCES}
    ${HEADERS}
)

# --- Include directories ---

target_include_directories(ToshibaT100 PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
    include/
    deps/nativefiledialog/src/include
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS}
)

# --- Libraries ---

target_link_libraries(ToshibaT100 PRIVATE
    ${SDL2_LIBRARIES}
    ${GTK3_LIBRARIES}
)

add_subdirectory(tests)