#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

extern "C" {
    #include "byterun.h"
}

using instr = std::pair<char *, int>;

struct cmpBytecode {
    bool operator()(const instr &a, const instr &b) const {
        if (a.second != b.second) return a.second > b.second;

        for (int i = 0; i < a.second; i++) {
            if (a.first[i] != b.first[i]) return a.first[i] > b.first[i];
        }

        return false;
    }
};

int main(int argc, char* argv[]) {
    bytefile *bf = read_file(argv[1]);
    char *ip = bf->code_ptr;
    std::map<instr, int, cmpBytecode> counter;

    while (true) {
        char *new_ip = disassemble_instruction(nullptr, bf, ip);

        if (new_ip == nullptr) break;

        int len = new_ip - ip;

        counter[{ ip, len }]++;
        ip = new_ip;
    }

    // stop instruction
    counter[{ ip, 1 }]++;

    std::vector<std::pair<instr, int>> sorted_instr;
    for (auto entry : counter) {
        sorted_instr.emplace_back(entry);
    }

    std::sort(sorted_instr.begin(), sorted_instr.end(),
              [](const std::pair<instr, int> &a, const std::pair<instr, int> &b) {
        return a.second > b.second;
    });

    FILE *f = stdout;

    for (auto entry : sorted_instr) {
        auto [t, len] = entry.first;
        int count = entry.second;

        fprintf(f, "%d: ", count);
        disassemble_instruction(f, bf, t);
    }

    return 0;
}