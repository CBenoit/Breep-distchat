cmake_minimum_required(VERSION 2.8)

project(SysDist-chat)

set(CMAKE_CXX_STANDARD 17)
set(BOOST_MIN_VERSION "1.55")

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED system)
find_package(OpenAL)
find_package(Threads)

include_directories(breep/include)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(include)

link_directories(${Boost_LIBRARY_DIRS})

add_executable(SysDist-chat src/main.cpp)

target_link_libraries(SysDist-chat ${OPENAL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
