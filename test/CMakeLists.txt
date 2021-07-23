
find_package(GTest REQUIRED)

add_executable(mb_clouds_gtest test.cpp)
target_link_libraries(mb_clouds_gtest mb_clouds_lib GTest::GTest)
