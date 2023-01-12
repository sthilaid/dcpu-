#pragma once
#include <dcpu-sexp.h>
#include <dcpu-types.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

class LispParser {
public:    
    static vector<Instruction> FromSExpressions(const vector<SExp*>& sexpressions);
    // static vector<Instruction> ParseLispAsm(const char* filename);
private:
    struct EnvBinding {
        string m_val;
    };
};
