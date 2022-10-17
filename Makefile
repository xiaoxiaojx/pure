build: 
	g++ src/*.cc -g -I deps/v8/include/ -I deps/uv/include/ -o pure -L liba/ -lv8_monolith -luv_a -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

run:
	make build && ./pure ./hello.js