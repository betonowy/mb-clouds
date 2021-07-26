#!/bin/sh

mkdir -p build

cd build && \
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && \
cmake --build . -- -j 12 && \
cd test && \
./mb_clouds_gtest --gtest_color=no && \
echo "Build and tests were successful! You can now run the app! build/mb_clouds" && exit 0

echo "Build or tests were not successful!" && exit 1
