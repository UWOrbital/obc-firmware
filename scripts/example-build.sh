if [ $# -ne 1 ]; then
  echo "Usage: $0 [EXAMPLE]"
  exit 1
fi
mkdir -p build_examples && cd build_examples
cmake .. -DCMAKE_BUILD_TYPE=Examples -DEXAMPLE_TYPE=$1
cmake --build . -j16
