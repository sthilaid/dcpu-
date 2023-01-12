#pragma once
#include <dcpu-sexp.h>
#include <dcpu-types.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

struct LabelRef {
    string m_label="";
    word_t* m_wordPtr;
    LabelRef(const string& label, word_t* wordPtr) : m_label(label), m_wordPtr(wordPtr) {}
};

struct LabelEnv {
    string m_label="";
    word_t m_addr=0;
    LabelEnv(const string& label, word_t addr) : m_label(label), m_addr(addr) {}
};

class LispAsmParser {
public:    
    static bool ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode, word_t& outSpecialOp);
    static void ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, word_t& outWord, vector<LabelRef>& foundLabels);
    static vector<Instruction> FromSExpressions(const vector<SExp*>& sexpressions);
    static vector<Instruction> ParseLispAsm(const char* filename);
};
