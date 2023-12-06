#include <string>
#include "interpreter.h"
#include "stack.h"

extern "C" {
    #include "runtime.h"

    extern void __init (void);
    extern void* Bstring(void*);
    extern void* Bsexp_arr (int bn, int tag, int *values);
    extern int LtagHash (char *s);
    extern void* Bsta (void *v, int i, void *x);
    extern void* Belem (void *p, int i);
    extern void* Belem_closure (void *p, int i);
    extern int Btag (void *d, int t, int n);
    extern int Barray_patt (void *d, int n);
    extern int Bstring_patt (void *x, void *y);
    extern int Bstring_tag_patt (void *x);
    extern int Barray_tag_patt (void *x);
    extern int Bsexp_tag_patt (void *x);
    extern int Bboxed_patt (void *x);
    extern int Bunboxed_patt (void *x);
    extern int Bclosure_tag_patt (void *x);
    extern int Lread();
    extern int Lwrite(int);
    extern int Llength(void*);
    extern void* Lstring (void *p);
    extern void* Barray_arr (int bn, int *values);
    extern void* Bclosure_arr (int bn, void *entry, int *values);
}

void *__start_custom_data;
void *__stop_custom_data;

void failure(char h, char l) {
    failure("ERROR: invalid opcode %d-%d\n", h, l);
}

interpreter::interpreter(std::string file_path) : bf(read_file(file_path.data())), ip(bf->code_ptr) {
    __init();

    fp = st.get_top();

    st.reserve(2);
    st.push(2);
}

interpreter::~interpreter() {
    free(bf->global_ptr);
    free(bf);
}

int32_t *interpreter::global(int32_t i) {
    return bf->global_ptr + i;
}

int32_t *interpreter::local(int32_t i) {
    return fp - i - 1;
}

int32_t *interpreter::arg(int32_t i) {
    return fp + i + 3;
}

int32_t *interpreter::binded(int32_t i) {
    int32_t nargs = *(fp + 1);
    auto id = reinterpret_cast<int32_t *>(*arg(nargs - 1));
    auto result = reinterpret_cast<int32_t>(Belem_closure(id, box(i + 1)));

    return reinterpret_cast<int32_t *>(result);
}

int32_t *interpreter::lookup(char l, int32_t i) {
    switch (l) {
        case 0:
            return global(i);
        case 1:
            return local(i);
        case 2:
            return arg(i);
        case 3:
            return binded(i);
        default:
            failure("Unexpected location: %d", l);
    }

    return nullptr;
}

void interpreter::jmp(int32_t offset) {
    ip = bf->code_ptr + offset;
}

char interpreter::get_byte() {
    return *ip++;
}

int interpreter::get_int() {
    int32_t res = *reinterpret_cast<int *>(ip);
    ip += sizeof(int32_t);
    return res;
}

char *interpreter::get_str() {
    int32_t string_id = get_int();
    return get_string(bf, string_id);
}


void interpreter::eval_binop(char type) {
    int32_t rhs = st.pop_int(), lhs = st.pop_int(), result;

    switch (type) {
        case 1:
            result = lhs + rhs;
            break;
        case 2:
            result = lhs - rhs;
            break;
        case 3:
            result = lhs * rhs;
            break;
        case 4:
            result = lhs / rhs;
            break;
        case 5:
            result = lhs % rhs;
            break;
        case 6:
            result = lhs < rhs;
            break;
        case 7:
            result = lhs <= rhs;
            break;
        case 8:
            result = lhs > rhs;
            break;
        case 9:
            result = lhs >= rhs;
            break;
        case 10:
            result = lhs == rhs;
            break;
        case 11:
            result = lhs != rhs;
            break;
        case 12:
            result = lhs && rhs;
            break;
        case 13:
            result = lhs || rhs;
            break;
        default:
            failure(0, type);
    }

    st.push_int(result);
}

void interpreter::eval_const() {
    int32_t value = get_int();
    st.push_int(value);
}

void interpreter::eval_string() {
    char *str = get_str();
    st.push(reinterpret_cast<int32_t>(Bstring(str)));
}

void interpreter::eval_sexp() {
    char *s = get_str();
    int32_t len = get_int();
    int32_t tag = LtagHash(s);

    st.reverse(len);
    auto result = reinterpret_cast<int32_t>(Bsexp_arr(box(len + 1), tag, st.get_top()));
    st.drop(len);
    st.push(result);
}

void interpreter::eval_sta() {
    void *v = reinterpret_cast<void *>(st.pop());
    int32_t i = st.pop();

    if (!is_boxed(i)) {
        return st.push(reinterpret_cast<int32_t>(Bsta(v, i, nullptr)));
    }

    void *x = reinterpret_cast<void *>(st.pop());
    st.push(reinterpret_cast<int32_t>(Bsta(v, i, x)));
}

void interpreter::eval_jmp() {
    jmp(get_int());
}

void interpreter::eval_drop() {
    st.pop();
}

void interpreter::eval_dup() {
    st.push(st.peek());
}

void interpreter::eval_swap() {
    int32_t v1 = st.pop();
    int32_t v2 = st.pop();

    st.push(v1);
    st.push(v2);
}

void interpreter::eval_elem() {
    int32_t idx = st.pop();
    void *a = reinterpret_cast<void *>(st.pop());
    st.push(reinterpret_cast<int32_t>(Belem(a, idx)));
}

void interpreter::eval_ld(char l) {
    int32_t value = *lookup(l, get_int());
    st.push(value);
}

void interpreter::eval_lda(char l) {
    int32_t *ptr = lookup(l, get_int());
    st.push(reinterpret_cast<int32_t>(ptr));
}

void interpreter::eval_st(char l) {
    int32_t loc_id = get_int();
    int32_t *ptr = lookup(l, loc_id);
    int32_t value = st.pop();

    *ptr = value;
    st.push(value);
}

void interpreter::eval_cjmpz() {
    int32_t offset = get_int();

    if (st.pop_int() == 0) {
        jmp(offset);
    }
}

void interpreter::eval_cjmpnz() {
    int32_t offset = get_int();

    if (st.pop_int() != 0) {
        jmp(offset);
    }
}

void interpreter::eval_begin() {
    int32_t nargs = get_int();
    int32_t nlocals = get_int();

    st.push(reinterpret_cast<int32_t>(fp));
    fp = st.get_top();
    st.reserve(nlocals);
}

void interpreter::eval_end() {
    int32_t result = st.pop();
    st.get_top() = fp;
    fp = reinterpret_cast<int32_t *>(st.pop());
    int32_t nargs = st.pop();
    ip = reinterpret_cast<char*>(st.pop());
    st.drop(nargs);
    st.push(result);
}

void interpreter::eval_call() {
    int32_t offset = get_int();
    int32_t nargs = get_int();

    st.reverse(nargs);
    st.push(reinterpret_cast<int32_t>(ip));
    st.push(nargs);
    jmp(offset);
}

void interpreter::eval_tag() {
    void *d = reinterpret_cast<void*>(st.pop());
    char *name = get_str();
    int32_t n = get_int();
    int32_t t = LtagHash(name);

    st.push(Btag(d, t, box(n)));
}

void interpreter::eval_array() {
    void *d = reinterpret_cast<void*>(st.pop());
    int len = get_int();
    int32_t res = Barray_patt(d, box(len));

    st.push(res);
}

void interpreter::eval_fail() {
    failure("FAIL %d %d", get_int(), get_int());
}

void interpreter::eval_line() {
    // TODO
    get_int();
}

void interpreter::eval_patt(char l) {
    auto value = reinterpret_cast<int32_t*>(st.pop());
    int32_t result;

    switch (l) {
        case 0:
            result = Bstring_patt(value, reinterpret_cast<int32_t*>(st.pop()));
            break;
        case 1:
            result = Bstring_tag_patt(value);
            break;
        case 2:
            result = Barray_tag_patt(value);
            break;
        case 3:
            result = Bsexp_tag_patt(value);
            break;
        case 4:
            result = Bboxed_patt(value);
            break;
        case 5:
            result = Bunboxed_patt(value);
            break;
        case 6:
            result = Bclosure_tag_patt(value);
            break;
        default:
            failure("Unexpected patt: %d", l);
    }

    return st.push(result);
}

void interpreter::eval_lread() {
    int32_t value = Lread();
    st.push(value);
}

void interpreter::eval_lwrite() {
    int32_t value = st.pop();

    st.push(Lwrite(value));
}

void interpreter::eval_llength() {
    st.push(Llength(reinterpret_cast<void*>(st.pop())));
}

void interpreter::eval_lstring() {
    void *str = Lstring(reinterpret_cast<void*>(st.pop()));
    st.push(reinterpret_cast<int32_t>(str));
}

void interpreter::eval_barray() {
    int32_t len = get_int();
    st.reverse(len);
    auto result = reinterpret_cast<int32_t>(Barray_arr(box(len), st.get_top()));
    st.drop(len);
    st.push(result);
}

void interpreter::eval_closure() {
    int32_t offset = get_int();
    int32_t nargs = get_int();
    int32_t args[nargs];

    for (int i = 0; i < nargs; i++) {
        char l = get_byte();
        int value = get_int();
        args[i] = *lookup(l, value);
    }

    void *result = Bclosure_arr(box(nargs), bf->code_ptr + offset, args);

    st.push(reinterpret_cast<int32_t>(result));
}

void interpreter::eval_callc() {
    int32_t nargs = get_int();
    void *label = Belem(reinterpret_cast<int32_t*>(st.peek(nargs)), box(0));
    st.reverse(nargs);
    st.push(reinterpret_cast<int32_t>(ip));
    st.push(nargs + 1);
    ip = reinterpret_cast<char *>(label);
}


void interpreter::eval() {
    int cnt = 0;

    while (ip != nullptr) {
        char x = get_byte(), h = (x & 0xF0) >> 4, l = x & 0x0F;

//        std::cout << cnt++ << std::endl;

        switch (h) {
            case 15:
                return;

            case 0:
                eval_binop(l);
                break;

            case 1:
                switch (l) {
                    case  0:
                        eval_const();
                        break;

                    case  1:
                        eval_string();
                        break;

                    case  2:
                        eval_sexp();
                        break;

                    case  4:
                        eval_sta();
                        break;

                    case  5:
                        eval_jmp();
                        break;

                    case  6:
                        eval_end();
                        break;

                    case  8:
                        eval_drop();
                        break;

                    case  9:
                        eval_dup();
                        break;

                    case 10:
                        eval_swap();
                        break;

                    case 11:
                        eval_elem();
                        break;

                    default:
                        failure(h, l);
                }
                break;

            case 2:
                eval_ld(l);
                break;

            case 3:
                eval_lda(l);
                break;

            case 4:
                eval_st(l);
                break;

            case 5:
                switch (l) {
                    case  0:
                        eval_cjmpz();
                        break;

                    case  1:
                        eval_cjmpnz();
                        break;

                    case  2:
                    case  3:
                        eval_begin();
                        break;

                    case 4:
                        eval_closure();
                        break;

                    case 5:
                        eval_callc();
                        break;

                    case  6:
                        eval_call();
                        break;

                    case  7:
                        eval_tag();
                        break;

                    case  8:
                        eval_array();
                        break;

                    case  9:
                        eval_fail();
                        break;

                    case 10:
                        eval_line();
                        break;

                    default:
                        failure(h, l);
                }
                break;

            case 6:
                eval_patt(l);
                break;

            case 7:
                switch (l) {
                    case 0:
                        eval_lread();
                        break;

                    case 1:
                        eval_lwrite();
                        break;

                    case 2:
                        eval_llength();
                        break;

                    case 3:
                        eval_lstring();
                        break;

                    case 4:
                        eval_barray();
                        break;

                    default:
                        failure(h, l);
                }
                break;

            default:
                failure(h, l);
        }

    }
}




