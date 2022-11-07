start:
	make build && ./pure ./hello.js

build:
	make build_lib && make build_src

build_src: 
	g++ src/*.cc -g -I deps/v8/include/ -I deps/uv/include/ -o pure -L liba/ -lv8_monolith -luv_a -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

build_lib:
	node ./tools/js2c.js

build_i: 
	g++ src/env.cc -g -I deps/v8/include/ -I deps/uv/include/ -E -o env.i -L liba/ -lv8_monolith -luv_a -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

start_demo:
	g++ test-demo/v8-api-test.cc -g -I deps/v8/include/ -I deps/uv/include/ -o pure_test -L liba/ -lv8_monolith -luv_a -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX && ./pure_test