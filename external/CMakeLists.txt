cmake_minimum_required(VERSION 3.10)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

# glad
add_library(gl3w ${CMAKE_CURRENT_SOURCE_DIR}/GL/gl3w.c)

# Dear ImGui
file(GLOB IMGUI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp)
add_library(imgui ${IMGUI_SRC})
