#include <dcpu-lispasm.h>
#include <dcpu-sexp.h>
#include <algorithm>
#include <cstdio>
#include <fstream>

bool LispAsmParser::ParseOpCodeFromSexp(const SExp::Val& val, OpCode& outOpcode, word_t& outSpecialOp) {
    dcpu_assert_fmt(val.m_type == SExp::Val::Symbol, "Expecting symbol val for opcode, got %d", val.m_type);
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

void LispAsmParser::ParseValueFromSexp(const SExp::Val& val, bool isA, Value& out, word_t& outWord,
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
                        "Expecting first value of val sexp to be symbol, but found type %d", val.m_sexpVal->m_values[0].m_type);
        dcpu_assert_fmt(toUpcase(val.m_sexpVal->m_values[0].m_symVal) == "REF",
                        "Expecting REF first sym value, but found: %s", val.m_sexpVal->m_values[0].m_symVal.c_str());

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

word_t GetAddr(const vector<Instruction>& instructions) {
    word_t addr = 0;
    for (const Instruction& i : instructions)
        addr += i.WordCount();
    return addr;
}

vector<Instruction> LispAsmParser::FromSExpressions(const vector<SExp*>& sexpressions) {
    vector<LabelEnv> labels;
    vector<LabelRef> labelRefs;
    vector<Instruction> instructions;
    instructions.reserve(sexpressions.size()); // way too much but ensures stable memory, for label refs

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
        word_t specialOpCode = 0;
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

vector<Instruction> LispAsmParser::ParseLispAsm(const char* filename){
    std::ifstream inputStream(filename, std::ios::in);
    if (!inputStream.is_open()) {
        printf("unknown file: %s\n", filename);
        return vector<Instruction>();
    }
    vector<Token> tokens = Token::Tokenize(inputStream);
    vector<SExp*> expressions = SExp::FromTokens(tokens);
    vector<Instruction> instructions = LispAsmParser::FromSExpressions(expressions);
    SExp::Delete(expressions);

    return instructions;
}

