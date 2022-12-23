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
    Value_Register_K = 0x07,
    Value_Register_Ref_A = 0x8,
    Value_Register_Ref_B = 0x9,
    Value_Register_Ref_C = 0x0A,
    Value_Register_Ref_X = 0x0B,
    Value_Register_Ref_Y = 0x0C,
    Value_Register_Ref_Z = 0x0D,
    Value_Register_Ref_I = 0x0E,
    Value_Register_Ref_K = 0x0F,
    Value_Register_RefNext_A = 0x10,
    Value_Register_RefNext_B = 0x11,
    Value_Register_RefNext_C = 0x12,
    Value_Register_RefNext_X = 0x13,
    Value_Register_RefNext_Y = 0x14,
    Value_Register_RefNext_Z = 0x15,
    Value_Register_RefNext_I = 0x16,
    Value_Register_RefNext_K = 0x17,
    Value_PushPop = 0x18,
    Value_Peek = 0x19,
    Value_Pick = 0x1A,
    Value_SP = 0x1B,
    Value_PC = 0x1C,
    Value_EX = 0x1D,
    Value_Next = 0x1E,
    Value_NextLitteral = 0x1F,
};

enum OpCode {
    OpCode_Special = 0x00,
    OpCode_SET = 0x01,
    OpCode_ADD = 0x02,
    OpCode_SUB = 0x03,
    OpCode_MUL = 0x04,
    OpCode_MLI = 0x05,
    OpCode_DIV = 0x06,
};

struct Instruction
{
    OpCode m_opcode = OpCode_Special;
    Value m_a = Value_Register_A;
    Value m_b = Value_Register_B;
    uint16_t m_wordA = 0;
    uint16_t m_wordB = 0;

    Instruction();
    Instruction(OpCode op, Value a, Value b, uint16_t wordA=0, uint16_t wordB=0);
    uint8_t ByteSize() const;
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
    case Value_Register_RefNext_K:
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

inline Instruction::Instruction(OpCode op, Value a, Value b, uint16_t wordA, uint16_t wordB)
    : m_opcode { op }
    , m_a { a }
    , m_b { b }
    , m_wordA { wordA }
    , m_wordB { wordB }
{
}

inline uint8_t Instruction::ByteSize() const {
    return 1 + (isMultibyteValue(m_a) ? 1 : 0) + (isMultibyteValue(m_b) ? 1 : 0) ;
}

