#pragma once

extern "C" {
    #include "byterun.h"
}

class interpreter {
public:
    explicit interpreter(std::string file_path);
    ~interpreter();

    void eval();

private:
    char get_byte();
    int32_t get_int();
    char *get_str();

    int32_t pop();
    int32_t peek(int32_t pos = 0);
    void push(int32_t value);
    int32_t pop_int();
    void push_int(int32_t value);
    void jmp(int32_t offset);
    void reverse(int32_t n);

    int32_t *global(int32_t i);
    int32_t *local(int32_t i);
    int32_t *arg(int32_t i);
    int32_t *binded(int32_t i);
    int32_t* lookup(char l, int32_t i);

    void eval_binop(char type);
    void eval_const();
    void eval_string();
    void eval_sexp();
    void eval_sta();
    void eval_jmp();
    void eval_drop();
    void eval_dup();
    void eval_swap();
    void eval_elem();
    void eval_ld(char l);
    void eval_lda(char l);
    void eval_st(char l);
    void eval_cjmpz();
    void eval_cjmpnz();
    void eval_begin();
    void eval_end();
    void eval_call();
    void eval_tag();
    void eval_array();
    void eval_fail();
    void eval_line();
    void eval_patt(char l);
    void eval_lread();
    void eval_lwrite();
    void eval_llength();
    void eval_lstring();
    void eval_barray();
    void eval_closure();
    void eval_callc();

    bytefile *bf;
    char *ip;

    int32_t *&stack_top;
    int32_t *&stack_bottom;
    int32_t *fp;
};

void interpret(std::string file_path);
