# ImGui

find_package(SDL2)
find_package(Vulkan)

file(GLOB_RECURSE IMGUI_SRCS imgui/*.cpp)

add_library(combined STATIC ${IMGUI_SRCS})

target_compile_options(combined PRIVATE -s -Os -ffast-math -fno-exceptions -pipe -Wall -Wextra -Wpedantic)
target_include_directories(combined PUBLIC imgui)
target_link_libraries(combined SDL2::SDL2 Vulkan::Vulkan imm32)
