#include <dcpu-lispasm.h>
#include <algorithm>

string SExp::Val::toStr() const {
    switch (m_type) {
    case Number: return std::to_string(m_numVal);
    case Symbol: return m_symVal;
    case SExp: return m_sexpVal->toStr();
    default: assert(false); return nullptr;
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

vector<Token> LispAsmParser::Tokenize(std::ifstream& inputStream) {
    vector<Token> tokens;
    string current;
    char c;
    bool is_commenting = false;
    while (!inputStream.eof()) {
        inputStream.read(&c, 1);
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
        assert(out != Value_Count);
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
        assert(sexp->m_values.size() == 3);

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
