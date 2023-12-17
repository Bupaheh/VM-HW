CC=gcc
CXX=g++
CFLAGS:=-I include -O3 -m32 -g2 -fstack-protector-all

all: build/main.o build/byterun.o
	$(CXX) $(CFLAGS) build/byterun.o build/main.o -o build/main

build/main.o: build src/main.cpp include/byterun.h
	$(CXX) $(CFLAGS) -c src/main.cpp -o build/main.o

build/byterun.o: build src/byterun.c include/byterun.h
	$(CC) $(CFLAGS) -c src/byterun.c -o build/byterun.o

build:
	mkdir build

clean:
	rm -r build
