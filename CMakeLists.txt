cmake_minimum_required (VERSION 3.1 FATAL_ERROR)
project(anagram)

#flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 REQUIRED)


add_executable(anagram
    anagram.cpp
    )
target_include_directories(anagram PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(anagram ${Boost_LIBRARIES})

add_executable(anagram_c
    anagram.c
    )
target_include_directories(anagram_c PUBLIC ${GLIB_INCLUDE_DIRS})
target_link_libraries(anagram_c ${GLIB_LIBRARIES})
