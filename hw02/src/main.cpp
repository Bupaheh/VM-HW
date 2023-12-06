#include <iostream>
#include "interpreter.h"

int main(int argc, char* argv[]) {
    interpreter yep = interpreter(argv[1]);
    yep.eval();

    return 0;
}