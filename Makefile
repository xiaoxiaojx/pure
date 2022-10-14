build: 
	g++ src/pure.cc -g -I deps/v8/include/ -o pure -L lib/ -lv8_monolith -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX