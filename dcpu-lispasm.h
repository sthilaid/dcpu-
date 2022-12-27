#pragma once
#include <cassert>
#include <istream>
#include <vector>
#include <string>
#include <types.h>

using std::string;
using std::vector;

struct Token {
    enum TokenType {
        LParen,
        RParen,
        Symbol,
        Number,
    };
    TokenType Type = LParen;
    uint16_t NumVal = 0;
    string SymVal = "";

    Token() : Type(LParen), NumVal(0), SymVal("") {}
    Token(char c) : Type(c == '(' ? LParen : RParen), NumVal(0), SymVal("") {
        assert(c == '(' || c == ')');
    }
    bool isNum(char c) const { return c >= '0' && c <= '9'; }
    Token(const string& str) : Type(Symbol), NumVal(0), SymVal(str) {
        assert(!str.empty());
        int baseStart = 0;
        if ((str[0] == '+' || str[0] == '-') && str.size() > 1)
            baseStart = 1;
        
        if (isNum(str[baseStart])) {
            Type = Number;
            int base = 10;
            if (str.size() >= baseStart+3 && str[baseStart] == '0' && str[baseStart+1] == 'x')
                base = 16;
            else if (str.size() >= baseStart+2 && str[baseStart] == '0')
                base = 8;
            NumVal = std::stoi(str, nullptr, base);
        }
    }
};

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
    static vector<SExp*> ParseSExpressions(const vector<Token>& tokens);
    static SExp* ParseSExp(const vector<Token>& tokens, uint16_t& i);

    string toStr() const;
};

struct LabelEnv {
    string m_label="";
    uint16_t m_addr=0;
    LabelEnv(const string& label, uint16_t addr) : m_label(label), m_addr(addr) {}
};

class LispAsmParser {
public:    
    static bool is_seperator(char c);
    static bool is_newline(char c) { return c == '\n'; }

    static vector<Token> Tokenize(std::basic_istream<char>& inputStream);
    static void ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode);
    static void ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, uint16_t& outWord, const vector<LabelEnv>& labels);
    static vector<Instruction> ParseTokens(const vector<Token>& tokens);
    static vector<Instruction> ParseLispAsm(char* filename);
};
