#pragma once

extern "C" {
    #include "runtime.h"
}

#include <algorithm>

extern int32_t *__gc_stack_top, *__gc_stack_bottom;

const int STACK_CAPACITY = sizeof(int32_t) * (2 << 20);

inline int32_t box(int32_t value) {
    return (value << 1) | 1;
}

inline int32_t unbox(int32_t value) {
    return value >> 1;
}

inline bool is_boxed(int32_t value) {
    return value & 1;
}

namespace stack {
    namespace {
        inline void stack_bounds_failure() {
            failure("Operation is out of stack bounds\n");
        }
    }

    inline int32_t *stack_upper_bound() {
        return __gc_stack_bottom;
    }

    inline int32_t *stack_lower_bound() {
        return __gc_stack_bottom - STACK_CAPACITY;
    }

    inline void init() {
        __gc_stack_top = (new int32_t[STACK_CAPACITY]) + STACK_CAPACITY;
        __gc_stack_bottom = __gc_stack_top;
    }

    inline void clear() {
        delete[] (__gc_stack_bottom - STACK_CAPACITY);
    }

    inline int32_t *get_top() {
        return __gc_stack_top;
    }

    inline void reverse(int32_t n) {
        if (__gc_stack_top + n - 1 >= stack_upper_bound()) {
            stack_bounds_failure();
        }

        int32_t *top = __gc_stack_top;
        int32_t *bot = top + n - 1;

        while (top < bot) {
            std::swap(*(top++), *(bot--));
        }
    }

    inline int32_t peek(int32_t pos = 0) {
        if (__gc_stack_top + pos >= stack_upper_bound()) {
            stack_bounds_failure();
        }

        return __gc_stack_top[pos];
    }

    inline int32_t pop() {
        if (__gc_stack_top + 1 > stack_upper_bound()) {
            stack_bounds_failure();
        }

        return *(__gc_stack_top++);
    }

    inline void push(int32_t value) {
        if (__gc_stack_top - 1 < stack_lower_bound()) {
            stack_bounds_failure();
        }

        *(--__gc_stack_top) = value;
    }

    inline int32_t pop_int() {
        return unbox(pop());
    }

    inline void push_int(int32_t value) {
        push(box(value));
    }

    inline void drop(int32_t n) {
        if (__gc_stack_top + n > stack_upper_bound()) {
            stack_bounds_failure();
        }

        __gc_stack_top += n;
    }

    inline void reserve(int32_t n) {
        if (__gc_stack_top - n < stack_lower_bound()) {
            stack_bounds_failure();
        }

        __gc_stack_top -= n;
    }

    inline void set_top(int32_t *value) {
        __gc_stack_top = value;
    }
}