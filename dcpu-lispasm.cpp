#include <dcpu-lispasm.h>
#include <algorithm>
#include <cstdio>
#include <fstream>

string SExp::Val::toStr() const {
    switch (m_type) {
    case Number: return std::to_string(m_numVal);
    case Symbol: return m_symVal;
    case SExp: return m_sexpVal->toStr();
    default:
        dcpu_assert(false, "unhandled value type");
        return nullptr;
    }
}

void SExp::Delete(SExp* sexp) {
    for (Val& v : sexp->m_values) {
        if (v.m_type == Val::SExp) {
            Delete(v.m_sexpVal);
        }
    }
    delete sexp;
}

vector<SExp*> SExp::ParseSExpressions(const vector<Token>& tokens) {
    vector<SExp*> expressions;
    uint16_t i=0;
    while (i<tokens.size()) {
        expressions.push_back(ParseSExp(tokens, i));
    }
    return expressions;
}

SExp* SExp::ParseSExp(const vector<Token>& tokens, uint16_t& i) {
    dcpu_assert_fmt(tokens.size()-i >= 2, "Expected more tokens. Left: %d, expecting 2", (tokens.size()-i));
    dcpu_assert_fmt(tokens[i].Type == Token::LParen, "Expecting LParen as first sexp token, got %d", tokens[i].Type);
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
    dcpu_assert(false, "Missing tokens to finish sexp");
    return nullptr;
}

string SExp::toStr() const {
    string str = "(";
    for (int i=0; i<m_values.size(); ++i) {
        str += m_values[i].toStr();
        str += i<m_values.size()-1 ? " " : ")";
    }
    return str;
}

bool LispAsmParser::is_seperator(char c) {
    switch (c) {
    case ' ':
    case '\t':
        return true;
    default:
        return false;
    }
}

vector<Token> LispAsmParser::Tokenize(std::basic_istream<char>& inputStream) {
    vector<Token> tokens;
    string current;
    char c;
    bool is_commenting = false;
    while (true) {
        inputStream.read(&c, 1);
        if (inputStream.eof())
            break; // need to check eof after the read call
        
        if (is_newline(c)) {
            is_commenting = false;
        } else if (is_commenting || c == ';') {
            is_commenting = true;
        } else if (c == '(' || c == ')') {
            if (current != "") {
                tokens.push_back(Token{current});
                current = "";
            }
            tokens.push_back(Token{c});
        } else if (is_seperator(c)) {
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

void LispAsmParser::ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode) {
    dcpu_assert_fmt(val.m_type == SExp::Val::Symbol, "Expecting symbol token for opcode, got %d", val.m_type);
    string upperOp = toUpcase(val.m_symVal);
    for (int i=0; i<OpCode_Count; ++i) {
        if (upperOp == OpCodeToStr(static_cast<OpCode>(i))) {
            outOpcode = static_cast<OpCode>(i);
            break;
        }
    }
    dcpu_assert_fmt(outOpcode != OpCode_Count, "Couldn't match opcode %s to known opcode", upperOp.c_str());
}

void LispAsmParser::ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, uint16_t& outWord,
                                       const vector<LabelEnv>& labels) {
    switch (val.m_type) {
    case SExp::Val::Number: {
        // todo, check when isA for embedded number
        out = Value_NextLitteral;
        outWord = val.m_numVal;
        break;
    }
    case SExp::Val::Symbol: {
        auto findIt = std::find_if(labels.begin(), labels.end(), [&val](const LabelEnv& lab) { return val.m_symVal == lab.m_label; });
        if (findIt != labels.end()) {
            out = Value_NextLitteral;
            outWord = findIt->m_addr;
        } else {
            string upperSym = toUpcase(val.m_symVal);
            for(int i=0; i<Value_Count; ++i) {
                if (upperSym == ValueToStr(static_cast<Value>(i), isA, 0)) {
                    out = static_cast<Value>(i);
                    outWord = 0;
                    break;
                }
            }
        }
        dcpu_assert_fmt(out != Value_Count, "Couldn't match value (%s) to known value type...", val.m_symVal);
        break;
    }
    case SExp::Val::SExp: {
        dcpu_assert_fmt(val.m_sexpVal->m_values.size() >= 2, "Expecting at least 2 values in val sexp, got %d",
                        val.m_sexpVal->m_values.size());
        dcpu_assert_fmt(val.m_sexpVal->m_values[0].m_type == SExp::Val::Symbol,
                        "Expecting first value of val sexp to be token, but found type %d", val.m_sexpVal->m_values[0].m_type);
        dcpu_assert_fmt(toUpcase(val.m_sexpVal->m_values[0].m_symVal) == "REF",
                        "Expecting REF first sym token, but found: %s", val.m_sexpVal->m_values[0].m_symVal);

        outWord = 0;
        
        if (val.m_sexpVal->m_values[1].m_type == SExp::Val::Number) {
            out = Value_Next;
            outWord = val.m_sexpVal->m_values[1].m_numVal;
        } else {
            dcpu_assert_fmt(val.m_sexpVal->m_values[1].m_type == SExp::Val::Symbol,
                            "Expecting symbol as first arg to Ref, found %d (%s)",
                            val.m_sexpVal->m_values[1].m_type, val.m_sexpVal->m_values[2].m_symVal.c_str());
            
            const Value first = StrToValue(val.m_sexpVal->m_values[1].m_symVal, isA);
            const bool hasOffset = val.m_sexpVal->m_values.size() == 3;
            if (first <= Value_Register_J) {
                if (hasOffset) {
                    dcpu_assert_fmt(val.m_sexpVal->m_values[2].m_type == SExp::Val::Number,
                                    "Expecting ref 2nd arg to be number, found type %d (%s)",
                                    val.m_sexpVal->m_values[2].m_type, val.m_sexpVal->m_values[2].m_symVal.c_str());
                    
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
            } else {
                dcpu_assert(false, "Couldn't parse value sexp properly");
            }
        }
            
        break;
    }
    }
}

uint16_t GetAddr(const vector<Instruction>& instructions) {
    uint16_t addr = 0;
    for (const Instruction& i : instructions)
        addr += i.WordCount();
    return addr;
}

vector<Instruction> LispAsmParser::ParseTokens(const vector<Token>& tokens) {
    vector<LabelEnv> labels;
    vector<Instruction> instructions;
    vector<SExp*> sexpressions = SExp::ParseSExpressions(tokens);
    for (SExp* sexp : sexpressions) {
        if (sexp->m_values.size() == 2
            && sexp->m_values[0].m_type == SExp::Val::Symbol
            && toUpcase(sexp->m_values[0].m_symVal) == "LABEL"
            && sexp->m_values[1].m_type == SExp::Val::Symbol) {
            labels.push_back(LabelEnv{sexp->m_values[1].m_symVal, GetAddr(instructions)});
            continue;
        }
        dcpu_assert_fmt(sexp->m_values.size() == 3, "Expecting a 3 value sexp of form (fun b a), but found only %d values",
                        sexp->m_values.size());

        Instruction& inst = instructions.emplace_back();
        ParseOpCodeFromSexp(sexp->m_values[0], inst.m_opcode);
        ParseValueFromSexp(sexp->m_values[1], false, inst.m_b, inst.m_wordB, labels);
        ParseValueFromSexp(sexp->m_values[2], true, inst.m_a, inst.m_wordA, labels);

        SExp::Delete(sexp);
    }
        
    return instructions;
}

vector<Instruction> LispAsmParser::ParseLispAsm(char* filename){
    std::ifstream inputStream(filename, std::ios::in);
    if (!inputStream.is_open()) {
        printf("unknown file: %s\n", filename);
        return vector<Instruction>();
    }
    vector<Token> tokens = LispAsmParser::Tokenize(inputStream);
    vector<Instruction> instructions = LispAsmParser::ParseTokens(tokens);
    return instructions;
}
