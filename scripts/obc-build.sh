rm compile_commands.json >& /dev/null # We will regen this later
mkdir -p build_arm && cd build_arm
cmake .. -DCMAKE_BUILD_TYPE=OBC -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build .
cd ..
ln -s $(pwd)/build_arm/compile_commands.json $(pwd)/compile_commands.json
