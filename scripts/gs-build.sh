rm compile_commands.json >& /dev/null # We will regen this later
mkdir -p build_gs && cd build_gs
cmake .. -DCMAKE_BUILD_TYPE=GS -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j16
cd ..
ln -s $(pwd)/build_gs/compile_commands.json $(pwd)/compile_commands.json
