#pragma once
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include <fstream>
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
    Token(const string& str) : Type(Symbol), NumVal(0), SymVal(str) {
        assert(!str.empty());
        if (str[0] >= '0' && str[0] <= '9') {
            Type = Number;
            NumVal = std::stoi(str);
        }
    }
};

struct SExp {
    struct Val {
        enum Type {
            Number,
            Symbol,
            SExp,
        } m_type;
        uint16_t m_numVal;
        string m_symVal;
        class SExp* m_sexpVal = nullptr;

        Val(uint16_t num) : m_type(Number), m_numVal(num), m_symVal(""), m_sexpVal() {}
        Val(const string& sym) : m_type(Symbol), m_numVal(0), m_symVal(sym), m_sexpVal() {}
        Val(class SExp* exp) : m_type(SExp), m_numVal(0), m_symVal(""), m_sexpVal(exp) {}
        string toStr() const {
            switch (m_type) {
            case Number: return std::to_string(m_numVal);
            case Symbol: return m_symVal;
            case SExp: return m_sexpVal->toStr();
            default: assert(false); return nullptr;
            }
        }
    };
    vector<Val> m_values;

    static void Delete(SExp* sexp) {
        for (Val& v : sexp->m_values) {
            if (v.m_type == Val::SExp) {
                Delete(v.m_sexpVal);
            }
        }
        delete sexp;
    }

    static vector<SExp*> ParseSExpressions(const vector<Token>& tokens) {
        vector<SExp*> expressions;
        uint16_t i=0;
        while (i<tokens.size()) {
            expressions.push_back(ParseSExp(tokens, i));
        }
        return expressions;
    }
    
    static SExp* ParseSExp(const vector<Token>& tokens, uint16_t& i) {
        assert(tokens.size()-i >= 2);
        assert(tokens[i].Type == Token::LParen);
        ++i;
        
        SExp* current = new SExp{};
        while (i<tokens.size()) {
            switch (tokens[i].Type) {
            case Token::LParen: {
                Val subExp{ParseSExp(tokens, i)};
                current->m_values.push_back(subExp);
                break;
            }
            case Token::RParen: {
                ++i;
                return current;
            }
            case Token::Number: {
                Val n{tokens[i++].NumVal};
                current->m_values.push_back(n);
                break;
            }
            case Token::Symbol: {
                Val s{tokens[i++].SymVal};
                current->m_values.push_back(s);
                break;
            }
            }
        }
        assert(false);
        return nullptr;
    }

    string toStr() const {
        string str = "(";
        for (int i=0; i<m_values.size(); ++i) {
            str += m_values[i].toStr();
            str += i<m_values.size()-1 ? " " : ")";
        }
        return str;
    }
};

class LispAsmParser {
public:    
    static bool is_seperator(char c) {
        switch (c) {
        case ' ':
        case '\t':
            return true;
        default:
            return false;
        }
    }

    static bool is_newline(char c) {
        return c == '\n';
    }

    static vector<Token> Tokenize(std::ifstream& inputStream) {
        vector<Token> tokens;
        string current;
        char c;
        while (!inputStream.eof()) {
            inputStream.read(&c, 1);
            if (is_newline(c)) {
            }
            else if (c == '(' || c == ')') {
                if (current != "") {
                    tokens.push_back(Token{current});
                    current = "";
                }
                tokens.push_back(Token{c});
            }
            else if (is_seperator(c)) {
                if (current != "") {
                    tokens.push_back(Token{current});
                    current = "";
                }
            } else {
                current += c;
            }
        }        
        return tokens;
    }

    
    static void ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode) {
        assert(val.m_type == SExp::Val::Symbol);
        string upperOp = toUpcase(val.m_symVal);
            for (int i=0; i<OpCode_Count; ++i) {
                if (upperOp == OpCodeToStr(static_cast<OpCode>(i))) {
                    outOpcode = static_cast<OpCode>(i);
                    break;
                }
            }
            assert(outOpcode != OpCode_Count);
    }

    static void ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, uint16_t& outWord) {
        switch (val.m_type) {
        case SExp::Val::Number: {
            // todo, check when isA for embedded number
            out = Value_NextLitteral;
            outWord = val.m_numVal;
            break;
        }
        case SExp::Val::Symbol: {
            string upperSym = toUpcase(val.m_symVal);
            for(int i=0; i<Value_Count; ++i) {
                if (upperSym == ValueToStr(static_cast<Value>(i), isA, 0)) {
                    out = static_cast<Value>(i);
                    outWord = 0;
                    break;
                }
            }
            break;
        }
        case SExp::Val::SExp: {
            assert(val.m_sexpVal->m_values.size() >= 2);
            assert(val.m_sexpVal->m_values[0].m_type == SExp::Val::Symbol);
            assert(toUpcase(val.m_sexpVal->m_values[0].m_symVal) == "REF");

            outWord = 0;
            
            assert(val.m_sexpVal->m_values[1].m_type == SExp::Val::Symbol);
            const Value first = StrToValue(val.m_sexpVal->m_values[1].m_symVal, isA);
            const bool hasOffset = val.m_sexpVal->m_values.size() == 3;
            if (first <= Value_Register_J) {
                if (hasOffset) {
                    assert(val.m_sexpVal->m_values[2].m_type == SExp::Val::Number);
                    
                    out = static_cast<Value>(first | 0x10);
                    outWord = val.m_sexpVal->m_values[2].m_numVal;
                }
                else {
                    out = static_cast<Value>(first | 0x08);
                }
            } else if (first == Value_SP) {
                if (hasOffset) {
                    out = Value_Pick;
                    outWord = val.m_sexpVal->m_values[2].m_numVal;
                } else {
                    out = Value_Peek;
                }
            } else if (hasOffset) {
                out = Value_Next;
                outWord = val.m_sexpVal->m_values[2].m_numVal;
            } else {
                assert(false);
            }
            
            break;
        }
        }
    }


    static vector<Instruction> ParseTokens(const vector<Token>& tokens) {
        vector<Instruction> instructions;
        vector<SExp*> sexpressions = SExp::ParseSExpressions(tokens);
        for (SExp* sexp : sexpressions) {
            assert(sexp->m_values.size() == 3);

            Instruction& inst = instructions.emplace_back();
            ParseOpCodeFromSexp(sexp->m_values[0], inst.m_opcode);
            ParseValueFromSexp(sexp->m_values[1], false, inst.m_b, inst.m_wordB);
            ParseValueFromSexp(sexp->m_values[2], true, inst.m_a, inst.m_wordA);

            SExp::Delete(sexp);
        }
        
        return instructions;
    }
};

int main(int argc, char** args) {
    if (argc != 2) {
        printf("usage: dcpu-asm <source-file>\n");
        return 1;
    }

    std::ifstream inputStream(args[1], std::ios::in);
    
    if (!inputStream.is_open()) {
        printf("unknown file: %s\n", args[1]);
        return 1;
    }
    vector<Token> tokens = LispAsmParser::Tokenize(inputStream);
    printf("tokens count: %d\n", tokens.size());

    vector<SExp*> expressions = SExp::ParseSExpressions(tokens);
    for (const SExp* sexp : expressions) {
        printf("sexp: %s\n", sexp->toStr().c_str());
    }

    for (SExp* sexp : expressions) {
        SExp::Delete(sexp);
    }
    expressions.clear();

    vector<Instruction> instructions = LispAsmParser::ParseTokens(tokens);
    for (const Instruction& i : instructions) {
        printf("parsed: %s\n", i.toStr().c_str());
    }
    return 0;
}
  
