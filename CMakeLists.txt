cmake_minimum_required (VERSION 3.1 FATAL_ERROR)
project(anagram)

option(ANAGRAM_BUILD_CPP "Build C++ implementation" ON)
option(ANAGRAM_BUILD_C "Build C implementation" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(ANAGRAM_BUILD_CPP)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

    find_package(Boost COMPONENTS program_options REQUIRED)

    add_executable(anagram anagram.cpp)
    target_include_directories(anagram PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(anagram ${Boost_LIBRARIES})
endif()

if(ANAGRAM_BUILD_C)
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB glib-2.0 REQUIRED)

    add_executable(anagram_c anagram.c)
    target_include_directories(anagram_c PUBLIC ${GLIB_INCLUDE_DIRS})
    target_link_libraries(anagram_c ${GLIB_LIBRARIES})
endif()
