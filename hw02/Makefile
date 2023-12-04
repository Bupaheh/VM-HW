CC=gcc
CXX=g++
CFLAGS:=-I include -O2 -m32 -g -fstack-protector-all

all: build/main.o build/gc_runtime.o build/byterun.o build/runtime.o
	$(CXX) -g -m32 build/gc_runtime.o build/runtime.o build/byterun.o build/main.o -o build/main

build/main.o: build src/main.cpp include/runtime.h include/byterun.h
	$(CXX) $(CFLAGS) -c src/main.cpp -o build/main.o

build/byterun.o: build src/byterun.c include/byterun.h
	$(CC) $(CFLAGS) -c src/byterun.c -o build/byterun.o

build/gc_runtime.o: build src/gc_runtime.s
	$(CC) $(CFLAGS) -c src/gc_runtime.s -o build/gc_runtime.o

build/runtime.o: build src/runtime.c include/runtime.h
	$(CC) $(CFLAGS) -c src/runtime.c -o build/runtime.o

build:
	mkdir build

clear:
	rm -r build