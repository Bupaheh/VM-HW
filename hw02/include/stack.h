#pragma once

int32_t box(int32_t value);
int32_t unbox(int32_t value);
bool is_boxed(int32_t value);

class stack {
public:
    stack();
    ~stack();

    int32_t *&get_top();
    void reverse(int32_t n);
    int32_t peek(int32_t pos = 0);
    int32_t pop();
    void push(int32_t value);
    int32_t pop_int();
    void push_int(int32_t value);
    void drop(int32_t n);
    void reserve(int32_t n);

private:
    int32_t *&stack_bottom;
    int32_t *&stack_top;
};
