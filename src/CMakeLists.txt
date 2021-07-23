file(GLOB_RECURSE mb_clouds_lib_srcs ./*.cpp ./*.h)

set(CMAKE_MODULE_PATH "C:/msys64/mingw64/lib/cmake/OpenVDB")

find_package(OpenVDB REQUIRED)
find_package(tbb)

add_library(mb_clouds_lib STATIC ${mb_clouds_lib_srcs})

target_link_libraries(
        mb_clouds_lib
        combined
        OpenVDB
        tbb
        blosc
        half
)

target_include_directories(mb_clouds_lib PUBLIC .)
