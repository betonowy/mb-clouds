# ImGui

find_package(SDL2)
find_package(OpenGL)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE IMGUI_SRCS imgui/*.cpp)

add_library(stb_image_local INTERFACE)

    target_include_directories(stb_image_local INTERFACE stb)

add_library(combined STATIC ${IMGUI_SRCS})

    target_compile_options(combined PRIVATE -s -Os -ffast-math -fno-exceptions -pipe -Wall -Wextra -Wpedantic)
    target_include_directories(combined PUBLIC imgui ${OPENGL_INCLUDE_DIR})
    target_link_libraries(combined SDL2::SDL2 ${OPENGL_LIBRARIES} GLEW::GLEW Imm32 stb_image_local)
