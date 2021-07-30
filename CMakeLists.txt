cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(mb_clouds)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -pipe -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-s -O3 -ffast-math -pipe -Wall -Wextra -Wpedantic -DNDEBUG")

    add_subdirectory(thirdparty)
    add_subdirectory(src)

    add_executable(mb_clouds main.cpp)
    target_link_libraries(mb_clouds mb_clouds_lib)

    add_subdirectory(test)
