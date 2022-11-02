# in the deps/uv directory

mkdir -p build

(cd build && cmake .. -DBUILD_TESTING=ON)

cmake --build build