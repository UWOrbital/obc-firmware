mkdir -p build && cd build
rm -rf CMakeCache.txt CMakeFiles/
cmake .. -DCMAKE_BUILD_TYPE=Test
cmake --build . -j16
ctest --verbose
