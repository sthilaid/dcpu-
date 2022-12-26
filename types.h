#pragma once

#include <algorithm>
#include <cassert>
#include <string>
using std::string;

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

    OpCode_Count,
};

struct Instruction
{
    OpCode m_opcode = OpCode_Count;
    Value m_a = Value_Count;
    Value m_b = Value_Count;
    uint16_t m_wordA = 0;
    uint16_t m_wordB = 0;

    Instruction();
    Instruction(OpCode op, Value b, Value a, uint16_t wordB=0, uint16_t wordA=0);
    uint8_t WordCount() const;
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
    : Instruction{OpCode_Special, Value_Register_A, Value_Register_A, 0, 0}
{    
}

inline Instruction::Instruction(OpCode op, Value b, Value a, uint16_t wordB, uint16_t wordA)
    : m_opcode { op }
    , m_a { a }
    , m_b { b }
    , m_wordA { wordA }
    , m_wordB { wordB }
{
}

inline uint8_t Instruction::WordCount() const {
    return 1 + (isMultibyteValue(m_a) ? 1 : 0) + (isMultibyteValue(m_b) ? 1 : 0) ;
}


inline string ValueToStr(Value v, bool isA, uint16_t nextword){
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
    case Value_Next: return "["+std::to_string(nextword)+"]";
    case Value_NextLitteral: return std::to_string(nextword);
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

inline string OpCodeToStr(OpCode op){
    switch (op) {
    case OpCode_Special: return "[todo-special]";
    case OpCode_SET: return "SET";
    case OpCode_ADD: return "ADD";
    case OpCode_SUB: return "SUB";
    case OpCode_MUL: return "MUL";
    case OpCode_MLI: return "MLI";
    case OpCode_DIV: return "DIV";
    default: return "[unknown]";
    }
}

inline string Instruction::toStr() const {
    string msg = OpCodeToStr(m_opcode)
        + " "
        + ValueToStr(m_b, false, m_wordB)
        + ", "
        + ValueToStr(m_a, true, m_wordA);
    return msg;
}
