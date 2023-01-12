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
        word_t m_numVal;
        string m_symVal;
        class SExp* m_sexpVal = nullptr;

        Val(word_t num) : m_type(Number), m_numVal(num), m_symVal(""), m_sexpVal() {}
        Val(const string& sym) : m_type(Symbol), m_numVal(0), m_symVal(sym), m_sexpVal() {}
        Val(class SExp* exp) : m_type(SExp), m_numVal(0), m_symVal(""), m_sexpVal(exp) {}
        string toStr() const;
    };
    vector<Val> m_values;

    static void Delete(SExp* sexp);
    static void Delete(vector<SExp*>& expressions);
    static vector<SExp*> FromTokens(const vector<Token>& tokens);
    static SExp* FromTokens(const vector<Token>& tokens, word_t& i);

    bool isNil() const;
    bool isFunctionCall() const;
    bool isSpecialForm(const char* formSym) const;
    string toStr() const;
};

inline bool SExp::isNil() const { return m_values.empty(); }
inline bool SExp::isFunctionCall() const { return m_values.size() > 0 && m_values[0].m_type != Val::Number; }
inline bool SExp::isSpecialForm(const char* formSym) const {
    return m_values.size() > 0
        && m_values[0].m_type == Val::Symbol
        && m_values[0].m_symVal == formSym;
}
