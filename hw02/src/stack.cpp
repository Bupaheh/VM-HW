#include <cstdint>
#include <algorithm>
#include "stack.h"

extern int32_t *__gc_stack_top, *__gc_stack_bottom;

const int STACK_CAPACITY = sizeof(int32_t) * (2 << 20);

int32_t box(int32_t value) {
    return (value << 1) | 1;
}

int32_t unbox(int32_t value) {
    return value >> 1;
}

bool is_boxed(int32_t value) {
    return value & 1;
}

stack::stack() : stack_bottom(__gc_stack_bottom), stack_top(__gc_stack_top) {
    stack_top = (new int32_t[STACK_CAPACITY]) + STACK_CAPACITY;
    stack_bottom = stack_top;
}

stack::~stack() {
    delete[] (stack_bottom - STACK_CAPACITY);
}

int32_t *&stack::get_top() {
    return stack_top;
}

void stack::reverse(int32_t n) {
    int32_t *top = stack_top;
    int32_t *bot = top + n - 1;

    while (top < bot) {
        std::swap(*(top++), *(bot--));
    }
}

int32_t stack::peek(int32_t pos) {
    return stack_top[pos];;
}

int32_t stack::pop() {
    return *(stack_top++);
}

void stack::push(int32_t value) {
    *(--stack_top) = value;
}

int32_t stack::pop_int() {
    return unbox(pop());
}

void stack::push_int(int32_t value) {
    push(box(value));
}

void stack::drop(int32_t n) {
    stack_top += n;
}

void stack::reserve(int32_t n) {
    stack_top -= n;
}
