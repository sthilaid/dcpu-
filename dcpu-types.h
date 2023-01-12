#pragma once

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>
using std::string;

namespace dcpu {
    using byte_t = uint8_t;
    using word_t = uint16_t;
    using signed_word_t = int16_t;
    using cycles_t = uint32_t;
    using long_t = uint32_t;
    using deviceIdx_t = uint16_t;
}

using namespace dcpu;

enum Value {
    Value_Register_A = 0x00,
    Value_Register_B = 0x01,
    Value_Register_C = 0x02,
    Value_Register_X = 0x03,
    Value_Register_Y = 0x04,
    Value_Register_Z = 0x05,
    Value_Register_I = 0x06,
    Value_Register_J = 0x07,
    Value_Register_Ref_A = 0x8,
    Value_Register_Ref_B = 0x9,
    Value_Register_Ref_C = 0x0A,
    Value_Register_Ref_X = 0x0B,
    Value_Register_Ref_Y = 0x0C,
    Value_Register_Ref_Z = 0x0D,
    Value_Register_Ref_I = 0x0E,
    Value_Register_Ref_J = 0x0F,
    Value_Register_RefNext_A = 0x10,
    Value_Register_RefNext_B = 0x11,
    Value_Register_RefNext_C = 0x12,
    Value_Register_RefNext_X = 0x13,
    Value_Register_RefNext_Y = 0x14,
    Value_Register_RefNext_Z = 0x15,
    Value_Register_RefNext_I = 0x16,
    Value_Register_RefNext_J = 0x17,
    Value_PushPop = 0x18,
    Value_Peek = 0x19,
    Value_Pick = 0x1A,
    Value_SP = 0x1B,
    Value_PC = 0x1C,
    Value_EX = 0x1D,
    Value_Next = 0x1E,
    Value_NextLitteral = 0x1F,

    Value_Count,
};

enum OpCode {
    OpCode_Special = 0x00,
    OpCode_SET = 0x01,
    OpCode_ADD = 0x02,
    OpCode_SUB = 0x03,
    OpCode_MUL = 0x04,
    OpCode_MLI = 0x05,
    OpCode_DIV = 0x06,
    OpCode_DVI = 0x07,
    OpCode_MOD = 0x08,
    OpCode_MDI = 0x09,
    OpCode_AND = 0x0A,
    OpCode_BOR = 0x0B,
    OpCode_XOR = 0x0C,
    OpCode_SHR = 0x0D,
    OpCode_ASR = 0x0E,
    OpCode_SHL = 0x0F,
    OpCode_IFB = 0x10,
    OpCode_IFC = 0x11,
    OpCode_IFE = 0x12,
    OpCode_IFN = 0x13,
    OpCode_IFG = 0x14,
    OpCode_IFA = 0x15,
    OpCode_IFL = 0x16,
    OpCode_IFU = 0x17,
    OpCode_ADX = 0x1A,
    OpCode_SBX = 0x1B,
    OpCode_STI = 0x1E,
    OpCode_STD = 0x1F,

    OpCode_Count,
};

enum SpecialOpCode {
    SpecialOpCode_JSR = 0x01,
    SpecialOpCode_INT = 0x08,
    SpecialOpCode_IAG = 0x09,
    SpecialOpCode_IAS = 0x0A,
    SpecialOpCode_RFI = 0x0B,
    SpecialOpCode_IAQ = 0x0C,
    SpecialOpCode_HWN = 0x10,
    SpecialOpCode_HWQ = 0x11,
    SpecialOpCode_HWI = 0x12,

    SpecialOpCode_Count,
};

struct Instruction
{
    OpCode m_opcode = OpCode_Count;
    Value m_a = Value_Count;
    Value m_b = Value_Count;
    word_t m_wordA = 0;
    word_t m_wordB = 0;

    Instruction();
    Instruction(OpCode op, Value b, Value a, word_t wordB=0, word_t wordA=0);
    byte_t WordCount() const;
    string toStr() const;
};


inline bool isMultibyteValue(Value v) {
    switch (v) {
    case Value_Register_RefNext_A:
    case Value_Register_RefNext_B:
    case Value_Register_RefNext_C:
    case Value_Register_RefNext_X:
    case Value_Register_RefNext_Y:
    case Value_Register_RefNext_Z:
    case Value_Register_RefNext_I:
    case Value_Register_RefNext_J:
    case Value_Pick:
    case Value_Next:
    case Value_NextLitteral:
        return true;
    default:
        return false;
    }
}

inline Instruction::Instruction()
    : Instruction{OpCode_Count, Value_Count, Value_Count, 0, 0}
{    
}

inline Instruction::Instruction(OpCode op, Value b, Value a, word_t wordB, word_t wordA)
    : m_opcode { op }
    , m_a { a }
    , m_b { b }
    , m_wordA { wordA }
    , m_wordB { wordB }
{
}

inline byte_t Instruction::WordCount() const {
    const bool isSpecial = m_opcode == OpCode_Special;
    if (isSpecial) {
        return 1 + (isMultibyteValue(m_a) ? 1 : 0);
    } else {
        return 1 + (isMultibyteValue(m_a) ? 1 : 0) + (isMultibyteValue(m_b) ? 1 : 0) ;
    }
}

inline string NumToHexStr(word_t w) {
    std::stringstream stream;
    stream << "0x" << std::hex << w;
    return std::string( stream.str() );
}

inline string ValueToStr(Value v, bool isA, word_t nextword){
    if (isA) {
        const word_t numV = static_cast<word_t>(v);
        if (numV >= 0x20) {
            if (numV == 0x3F)
                return std::to_string(-1);
            else
                return std::to_string(numV-0x20);
        }
    }
    switch (v){
    case Value_Register_A: return "A";
    case Value_Register_B: return "B";
    case Value_Register_C: return "C";
    case Value_Register_X: return "X";
    case Value_Register_Y: return "Y";
    case Value_Register_Z: return "Z";
    case Value_Register_I: return "I";
    case Value_Register_J: return "J";
    case Value_Register_Ref_A: return "[A]";
    case Value_Register_Ref_B: return "[B]";
    case Value_Register_Ref_C: return "[C]";
    case Value_Register_Ref_X: return "[X]";
    case Value_Register_Ref_Y: return "[Y]";
    case Value_Register_Ref_Z: return "[Z]";
    case Value_Register_Ref_I: return "[I]";
    case Value_Register_Ref_J: return "[J]";
    case Value_Register_RefNext_A: return "[A+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_B: return "[B+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_C: return "[C+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_X: return "[X+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_Y: return "[Y+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_Z: return "[Z+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_I: return "[I+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_J: return "[J+"+std::to_string(nextword)+"]";
    case Value_PushPop: return isA ? "POP" : "PUSH";
    case Value_Peek: return "PEEK";
    case Value_Pick: return "PICK "+ std::to_string(nextword);
    case Value_SP: return "SP";
    case Value_PC: return "PC";
    case Value_EX: return "EX";
    case Value_Next: return "["+NumToHexStr(nextword)+"]";
    case Value_NextLitteral: return NumToHexStr(nextword);
    default: return "[unknown]";
    }
}

inline string toUpcase(const string& str) {
    string upStr = str;
    std::transform(upStr.begin(), upStr.end(), upStr.begin(), [](unsigned char c){ return std::toupper(c); });
    assert(str.size() == upStr.size());
    return upStr;
}

inline Value StrToValue(const string& str, bool isA) {
    string upperSym = toUpcase(str);
    for(int i=0; i<Value_Count; ++i) {
        if (upperSym == ValueToStr(static_cast<Value>(i), isA, 0)) {
            return static_cast<Value>(i);
        }
    }
    return Value_Count;
}

inline string SpecialOpCodeToStr(SpecialOpCode op) {
    switch (op) {
    case SpecialOpCode_JSR: return "JSR";
    case SpecialOpCode_INT: return "INT";
    case SpecialOpCode_IAG: return "IAG";
    case SpecialOpCode_IAS: return "IAS";
    case SpecialOpCode_RFI: return "RFI";
    case SpecialOpCode_IAQ: return "IAQ";
    case SpecialOpCode_HWN: return "HWN";
    case SpecialOpCode_HWQ: return "HWQ";
    case SpecialOpCode_HWI: return "HWI";

    default:
        return "<Unknown SpecialOpCode>";
    }
}

inline string OpCodeToStr(OpCode op, word_t b=0xFFFF){
    switch (op) {
    case OpCode_Special:
        return SpecialOpCodeToStr(static_cast<SpecialOpCode>(b));
    case OpCode_SET: return "SET";
    case OpCode_ADD: return "ADD";
    case OpCode_SUB: return "SUB";
    case OpCode_MUL: return "MUL";
    case OpCode_MLI: return "MLI";
    case OpCode_DIV: return "DIV";
    case OpCode_DVI: return "DVI";
    case OpCode_MOD: return "MOD";
    case OpCode_MDI: return "MDI";
    case OpCode_AND: return "AND";
    case OpCode_BOR: return "BOR";
    case OpCode_XOR: return "XOR";
    case OpCode_SHR: return "SHR";
    case OpCode_ASR: return "ASR";
    case OpCode_SHL: return "SHL";
    case OpCode_IFB: return "IFB";
    case OpCode_IFC: return "IFC";
    case OpCode_IFE: return "IFE";
    case OpCode_IFN: return "IFN";
    case OpCode_IFG: return "IFG";
    case OpCode_IFA: return "IFA";
    case OpCode_IFL: return "IFL";
    case OpCode_IFU: return "IFU";
    case OpCode_ADX: return "ADX";
    case OpCode_SBX: return "SBX";
    case OpCode_STI: return "STI";
    case OpCode_STD: return "STD";
    default: return "[unknown]";
    }
    static_assert(OpCode_Count == 0x20, "Please update this when changing opcodes");
}

inline string Instruction::toStr() const {
    string msg;
    if (m_opcode == OpCode_Special) {
        msg = OpCodeToStr(m_opcode, m_b)
            + " "
            + ValueToStr(m_a, true, m_wordA);
    }
    else {
        msg = OpCodeToStr(m_opcode, m_b)
            + " "
            + ValueToStr(m_b, false, m_wordB)
            + ", "
            + ValueToStr(m_a, true, m_wordA);
    }
    return msg;
}
