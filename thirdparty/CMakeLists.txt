#ImGui

find_package(SDL2)

file(GLOB_RECURSE IMGUI_SRCS imgui/*.cpp)
add_library(local_imgui ${IMGUI_SRCS})
target_compile_options(local_imgui PRIVATE -s -Ofast -ffast-math -fno-exceptions -pipe -Wall -Wextra -Wpedantic)
target_include_directories(local_imgui PUBLIC imgui)
target_link_libraries(local_imgui SDL2::SDL2)
