#include <iostream>

extern "C" {
    #include "byterun.h"
}

extern "C" {
    extern void __init (void);
}

void *__start_custom_data;
void *__stop_custom_data;

int main() {
    __init();
    char *name = "Sort.bc";
    bytefile *t = read_file(name);
    std::cout << t->string_ptr;
}