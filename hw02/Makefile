CC=gcc
CXX=g++
CFLAGS:=-I include -O3 -m32 -g2 -fstack-protector-all

all: build/main.o build/gc_runtime.o build/byterun.o build/runtime.o build/interpreter.o
	$(CXX) $(CFLAGS) build/gc_runtime.o build/runtime.o build/byterun.o build/interpreter.o build/main.o -o build/main

build/main.o: build src/main.cpp include/interpreter.h
	$(CXX) $(CFLAGS) -c src/main.cpp -o build/main.o

build/interpreter.o: build include/stack.h src/interpretator.cpp include/runtime.h include/byterun.h
	$(CXX) $(CFLAGS) -c src/interpreter.cpp -o build/interpreter.o

build/byterun.o: build src/byterun.c include/byterun.h
	$(CC) $(CFLAGS) -c src/byterun.c -o build/byterun.o

build/runtime.o: build src/runtime.c include/runtime.h
	$(CC) $(CFLAGS) -c src/runtime.c -o build/runtime.o

build/gc_runtime.o: build src/gc_runtime.s
	$(CC) $(CFLAGS) -c src/gc_runtime.s -o build/gc_runtime.o

build:
	mkdir build

clean:
	rm -r build
