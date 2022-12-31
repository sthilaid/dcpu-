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

bool LispAsmParser::ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode, uint16_t& outSpecialOp) {
    dcpu_assert_fmt(val.m_type == SExp::Val::Symbol, "Expecting symbol token for opcode, got %d", val.m_type);
    string upperOp = toUpcase(val.m_symVal);
    for (int i=0; i<SpecialOpCode_Count; ++i) {
        if (upperOp == SpecialOpCodeToStr(static_cast<SpecialOpCode>(i))) {
        SpecialOp:
            outOpcode = OpCode_Special;
            outSpecialOp = static_cast<SpecialOpCode>(i);
            return true;
        }
    }
    for (int i=0; i<OpCode_Count; ++i) {
        if (upperOp == OpCodeToStr(static_cast<OpCode>(i))) {
            outOpcode = static_cast<OpCode>(i);
            return false;
        }
    }
    dcpu_assert_fmt(outOpcode != OpCode_Count, "Couldn't match opcode %s to known opcode", upperOp.c_str());
    return false;
}

void LispAsmParser::ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, uint16_t& outWord,
                                       vector<LabelRef>& labelRefs) {
    switch (val.m_type) {
    case SExp::Val::Number: {
        if (isA && (val.m_numVal == 0xFFFF || val.m_numVal <= 30)) {
            if (val.m_numVal == 0xFFFF) {
                out = static_cast<Value>(0x3F);
            } else {
                out = static_cast<Value>(0x20 + val.m_numVal);
            }
            outWord = 0;
        } else {
            out = Value_NextLitteral;
            outWord = val.m_numVal;
        }
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
        if (out == Value_Count) {
            out = Value_NextLitteral;
            labelRefs.push_back(LabelRef{upperSym, &outWord});
        }
        break;
    }
    case SExp::Val::SExp: {
        dcpu_assert_fmt(val.m_sexpVal->m_values.size() >= 2, "Expecting at least 2 values in val sexp, got %d",
                        val.m_sexpVal->m_values.size());
        dcpu_assert_fmt(val.m_sexpVal->m_values[0].m_type == SExp::Val::Symbol,
                        "Expecting first value of val sexp to be token, but found type %d", val.m_sexpVal->m_values[0].m_type);
        dcpu_assert_fmt(toUpcase(val.m_sexpVal->m_values[0].m_symVal) == "REF",
                        "Expecting REF first sym token, but found: %s", val.m_sexpVal->m_values[0].m_symVal.c_str());

        refval:
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
                dcpu_assert_fmt(false, "Couldn't parse sexp value properly: %s", val.m_sexpVal->toStr().c_str());
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
    vector<LabelRef> labelRefs;
    vector<Instruction> instructions;
    instructions.reserve(tokens.size()); // way too much but ensures stable memory, for label refs
    vector<SExp*> sexpressions = SExp::ParseSExpressions(tokens);
    for (SExp* sexp : sexpressions) {
        if (sexp->m_values.size() == 2
            && sexp->m_values[0].m_type == SExp::Val::Symbol
            && toUpcase(sexp->m_values[0].m_symVal) == "LABEL"
            && sexp->m_values[1].m_type == SExp::Val::Symbol) {
            string upperLabel = toUpcase(sexp->m_values[1].m_symVal);
            labels.push_back(LabelEnv{upperLabel, GetAddr(instructions)});
            continue;
        }
        dcpu_assert_fmt(sexp->m_values.size() >= 2,
                        "Expecting form (specialop a) / (op b a), but found %d values: %s",
                        sexp->m_values.size(), sexp->toStr().c_str());

        Instruction& inst = instructions.emplace_back();
        uint16_t specialOpCode = 0;
        const bool isSpecialOp = ParseOpCodeFromSexp(sexp->m_values[0], inst.m_opcode, specialOpCode);
        if (isSpecialOp) {
            dcpu_assert_fmt(sexp->m_values.size() == 2,
                            "Expecting a 2 value sexp of form (specialop a), but found only %d values: %s",
                            sexp->m_values.size(), sexp->toStr().c_str());
            inst.m_b = static_cast<Value>(specialOpCode);
            ParseValueFromSexp(sexp->m_values[1], true, inst.m_a, inst.m_wordA, labelRefs);
        } else {
            dcpu_assert_fmt(sexp->m_values.size() == 3,
                            "Expecting a 3 value sexp of form (op b a), but found only %d values: %s",
                            sexp->m_values.size(), sexp->toStr().c_str());
            ParseValueFromSexp(sexp->m_values[1], false, inst.m_b, inst.m_wordB, labelRefs);
            ParseValueFromSexp(sexp->m_values[2], true, inst.m_a, inst.m_wordA, labelRefs);
        }

        SExp::Delete(sexp);
    }

 AssignLabelRefs:
    for (LabelRef& ref : labelRefs) {
        auto it = std::find_if(labels.begin(), labels.end(), [&ref](const LabelEnv& label) { return ref.m_label == label.m_label; });
        if (it != labels.end()) {
            *ref.m_wordPtr = it->m_addr;
        } else {
            dcpu_assert_fmt(false, "Couldn't find label %s in label environment.", ref.m_label.c_str());
        }
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
