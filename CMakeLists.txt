cmake_minimum_required(VERSION 3.10)

project(SysDist-chat CXX C)

set(BOOST_MIN_VERSION "1.55")
set(OpenGL_GL_PREFERENCE GLVND)

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED system)
find_package(OpenAL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)


# Folders configuration
get_filename_component(root_dir         ${CMAKE_SOURCE_DIR}               REALPATH)
get_filename_component(SFML_dir         ${root_dir}/external/SFML         REALPATH)
get_filename_component(imgui_dir        ${root_dir}/external/imgui        REALPATH)
get_filename_component(imgui-sfml_dir   ${root_dir}/external/imgui-sfml   REALPATH)

# By default build in Release mode
if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Add SFML targets
add_subdirectory(${SFML_dir})

# Add common CMake functions and macros
include(common.cmake)

# Dependencies check
directory_is_empty(is_empty "${SFML_dir}")
if(is_empty)
    message(FATAL_ERROR "SFML dependency is missing, maybe you didn't pull the git submodules")
endif()
directory_is_empty(is_empty "${imgui_dir}")
if(is_empty)
    message(FATAL_ERROR "ImGui dependency is missing, maybe you didn't pull the git submodules")
endif()
directory_is_empty(is_empty "${imgui-sfml_dir}")
if(is_empty)
    message(FATAL_ERROR "ImGui-SFML dependency is missing, maybe you didn't pull the git submodules")
endif()

# Add _d suffix in debug mode
set(CMAKE_DEBUG_POSTFIX _d)

# configure SFML folder in IDE
foreach(sfml_target IN ITEMS sfml-system sfml-network sfml-window sfml-graphics sfml-audio sfml-main)
    if(TARGET ${sfml_target})
        set_target_properties(${sfml_target} PROPERTIES FOLDER external/SFML)
    endif()
endforeach()

# Configure ImGui for ImGui-SFML
message(STATUS "ImGui configuration for SFML")
if(NOT EXISTS "${imgui_dir}/CMake_ImGui_SFML_Config_Done")
    set(tmp ${CMAKE_DISABLE_SOURCE_CHANGES})
    set(CMAKE_DISABLE_SOURCE_CHANGES OFF)
    file(READ "${imgui-sfml_dir}/imconfig-SFML.h" imconfig-SFML_content)
    file(APPEND "${imgui_dir}/imconfig.h" "${imconfig-SFML_content}")
    file(WRITE "${imgui_dir}/CMake_ImGui_SFML_Config_Done" "")
    message(STATUS "ImGui configuration for SFML - done")
    set(CMAKE_DISABLE_SOURCE_CHANGES ${tmp})
else()
    message(STATUS "ImGui configuration for SFML - already done")
endif()

# Setup ImGui-SFML
get_files(
        files
        "${imgui_dir}"
        "${imgui-sfml_dir}"
)
make_target(
        ImGui-SFML "external/ImGui-SFML" ${files}
        INCLUDES "${SFML_dir}/include/" "${imgui_dir}" "${imgui-sfml_dir}"
        OPTIONS cxx no_warnings static_runtime
)
target_link_libraries(ImGui-SFML PRIVATE "sfml-system" "sfml-network" "sfml-window" "sfml-graphics" "sfml-audio")

# Setup BaseConverter
get_files(
        files
        "${root_dir}/src"
        "${root_dir}/include"
        OPTIONS recurse
)
make_target(
        SysDist-chat "SysDist-chat" ${files}
        INCLUDES "include/"
        EXT_INCLUDES "${SFML_dir}/include/" "${imgui_dir}" "${imgui-sfml_dir}" "breep/include"
        OPTIONS cxx executable static_runtime
)

set_property(TARGET SysDist-chat PROPERTY CXX_STANDARD 17)
set_property(TARGET SysDist-chat PROPERTY CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdisabled-optimization -Wvector-operation-performance -Winvalid-pch \
                            -Wunused -Wconversion -Wlogical-op -Wuninitialized -Wsuggest-final-types -Wdouble-promotion \
                            -Wmissing-include-dirs -Wall -pedantic  -Wcast-qual -Wswitch-default -Wunreachable-code \
                            -Wfloat-equal -Wundef -Wcast-align -Wcast-align -Wredundant-decls -Winit-self -Wshadow \
                            -Wnon-virtual-dtor -Weffc++ -Wswitch-enum -Wmissing-declarations -fmessage-length=150 -W \
                            -Wpointer-arith -Wwrite-strings -Wmissing-noreturn")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined -g")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O2")
endif()

link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(SysDist-chat ${OPENAL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        ImGui-SFML sfml-window sfml-graphics sfml-system ${OPENGL_LIBRARY})

# Setup targets output, put exe and required SFML dll in the same folder
target_set_output_directory(sfml-system "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
target_set_output_directory(sfml-window "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
target_set_output_directory(sfml-graphics "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
target_set_output_directory(SysDist-chat "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
