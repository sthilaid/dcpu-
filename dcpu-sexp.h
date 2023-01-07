#pragma once

#include <vector>
#include <string>
#include <dcpu-types.h>

using std::string;
using std::vector;

struct Token;

struct SExp {
    struct Val {
        enum Type { Number, Symbol, SExp } m_type;
        uint16_t m_numVal;
        string m_symVal;
        class SExp* m_sexpVal = nullptr;

        Val(uint16_t num) : m_type(Number), m_numVal(num), m_symVal(""), m_sexpVal() {}
        Val(const string& sym) : m_type(Symbol), m_numVal(0), m_symVal(sym), m_sexpVal() {}
        Val(class SExp* exp) : m_type(SExp), m_numVal(0), m_symVal(""), m_sexpVal(exp) {}
        string toStr() const;
    };
    vector<Val> m_values;

    static void Delete(SExp* sexp);
    static vector<SExp*> FromTokens(const vector<Token>& tokens);
    static SExp* FromTokens(const vector<Token>& tokens, uint16_t& i);

    string toStr() const;
};
