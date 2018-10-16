cmake_minimum_required(VERSION 3.10)

project(SysDist-chat)

set(CMAKE_CXX_STANDARD 17)
set(BOOST_MIN_VERSION "1.55")

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED system)
find_package(OpenAL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(external/)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdisabled-optimization -Wvector-operation-performance -Winvalid-pch \
                            -Wunused -Wconversion -Wlogical-op -Wuninitialized -Wsuggest-final-types -Wdouble-promotion \
                            -Wmissing-include-dirs -Wall -pedantic  -Wcast-qual -Wswitch-default -Wunreachable-code \
                            -Wfloat-equal -Wundef -Wcast-align -Wcast-align -Wredundant-decls -Winit-self -Wshadow \
                            -Wnon-virtual-dtor -Weffc++ -Wswitch-enum -Wmissing-declarations -fmessage-length=150 -W \
                            -Wpointer-arith -Wmain -Wwrite-strings -Wmissing-noreturn")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined -g")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O2")
endif()

include_directories(
    include
    breep/include
    SYSTEM ${Boost_INCLUDE_DIR}
    SYSTEM ${OPENAL_INCLUDE_DIR}
    SYSTEM external/
)

link_directories(${Boost_LIBRARY_DIRS})

file(
    GLOB SysDist-chat_SRC
    "include/*.hpp"
    "src/*.cpp"
)

add_executable(SysDist-chat ${SysDist-chat_SRC})

target_link_libraries(SysDist-chat imgui gl3w glfw ${OPENAL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})
