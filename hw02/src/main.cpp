#include <iostream>
#include "interpreter.h"

int main(int argc, char* argv[]) {
    interpreter evaluator = interpreter(argv[1]);
    evaluator.eval();

    return 0;
}