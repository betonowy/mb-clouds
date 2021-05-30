cmake_minimum_required(VERSION 3.12)
project(mb_clouds)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -pipe -Wall -Wextra -Wpedantic -DDEBUG_BUILD")
set(CMAKE_CXX_FLAGS_RELEASE "-s -Ofast -ffast-math -fno-exceptions -pipe -Wall -Wextra -Wpedantic")

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

add_executable(mb_clouds main.cpp ${SRCS} ${HDRS})

add_subdirectory(thirdparty)

if (WIN32)
    message("Windows build")

    target_link_libraries(
            mb_clouds
            SDL2::SDL2
            Vulkan::Vulkan
            local_imgui
            imm32
    )
endif ()

target_include_directories(
        mb_clouds PUBLIC
        src
)
