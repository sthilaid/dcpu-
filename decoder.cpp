#include <cassert>
#include <decoder.h>

#define NDEBUG

string Decoder::ValueToStr(Value v, bool isA, uint16_t nextword){
    switch (v){
    case Value_Register_A: return "A";
    case Value_Register_B: return "B";
    case Value_Register_C: return "C";
    case Value_Register_X: return "X";
    case Value_Register_Y: return "Y";
    case Value_Register_Z: return "Z";
    case Value_Register_I: return "I";
    case Value_Register_K: return "L";
    case Value_Register_Ref_A: return "[A]";
    case Value_Register_Ref_B: return "[B]";
    case Value_Register_Ref_C: return "[C]";
    case Value_Register_Ref_X: return "[X]";
    case Value_Register_Ref_Y: return "[Y]";
    case Value_Register_Ref_Z: return "[Z]";
    case Value_Register_Ref_I: return "[I]";
    case Value_Register_Ref_K: return "[K]";
    case Value_Register_RefNext_A: return "[A+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_B: return "[B+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_C: return "[C+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_X: return "[X+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_Y: return "[Y+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_Z: return "[Z+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_I: return "[I+"+std::to_string(nextword)+"]";
    case Value_Register_RefNext_K: return "[K+"+std::to_string(nextword)+"]";
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

string Decoder::OpCodeToStr(OpCode op){
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

string Instruction::toStr() const {
    string msg = Decoder::OpCodeToStr(m_opcode)
        + " "
        + Decoder::ValueToStr(m_b, false, m_wordB)
        + ", "
        + Decoder::ValueToStr(m_a, true, m_wordA);
    return msg;
}

void pushTo8BitLittleEndian(uint16_t val, vector<uint8_t>& bytes){
    uint8_t littleEnd = 0xFF & val;
    uint8_t bigEnd = val >> 8;
    bytes.push_back(littleEnd);
    bytes.push_back(bigEnd);
}

uint16_t from8BitLittleEndian(const vector<uint8_t>& buffer, uint16_t& i) {
    assert(i < buffer.size()-1);
    
    uint8_t littleEnd = buffer[i];
    uint8_t bigEnd = buffer[i+1];
    i += 2;
    // printf("little:: %02X big: %02X, raw: %04X\n", littleEnd, bigEnd, (bigEnd << 8) | littleEnd);
    return (bigEnd << 8) | littleEnd;
}

vector<uint16_t> Decoder::Encode(const vector<Instruction>& instructions){
    vector<uint16_t> codeBuffer;
    for (const Instruction& inst : instructions){
        const uint16_t opcode = inst.m_opcode;
        const uint16_t a = inst.m_a;
        const uint16_t b = inst.m_b;
        const uint16_t binaryInstruction = (a << 0xA) | (b << 0x5) | opcode;
        codeBuffer.push_back(binaryInstruction);

        if (a == Value_Next) {
            codeBuffer.push_back(inst.m_wordA);
        }

        if (b == Value_Next) {
            codeBuffer.push_back(inst.m_wordB);
        }
    }
    return codeBuffer;
}

vector<uint8_t> Decoder::UnpackBytes(const vector<uint16_t>& buffer){
    vector<uint8_t> unpackedBuffer;
    for (uint16_t i=0; i<buffer.size(); ++i) {
        pushTo8BitLittleEndian(buffer[i], unpackedBuffer);
    }
    return unpackedBuffer;
}

vector<uint16_t> Decoder::PackBytes(const vector<uint8_t>& buffer){
    vector<uint16_t> packedBuffer;
    for (uint16_t i=0; i<buffer.size()-1;) {
        packedBuffer.push_back(from8BitLittleEndian(buffer, i));
    }
    return packedBuffer;
}

Instruction Decoder::Decode(const uint16_t* codebytePtr, uint32_t maxlen){
    assert(codebytePtr != nullptr);
    
    uint8_t instructionSize = 1;
    Instruction inst;
    uint16_t codebyte = *codebytePtr;
    inst.m_opcode = static_cast<OpCode>(codebyte & 0x1F);
    inst.m_a = static_cast<Value>(codebyte >> 0xA);
    inst.m_b = static_cast<Value>((codebyte >> 0x5) & 0x1F);

    if (isMultibyteValue(inst.m_a)) {
        assert(++instructionSize < maxlen);
        inst.m_wordA = codebytePtr[1];
    }
    if (isMultibyteValue(inst.m_b)) {
        assert(++instructionSize < maxlen);
        inst.m_wordB = codebytePtr[instructionSize-1];
    }
    // printf("-decoded- op: %02X, b: %02X, a: %02X, wordB: %04X, wordA: %04X, inst: %s\n",
    //        inst.m_opcode, inst.m_b, inst.m_a, inst.m_wordB, inst.m_wordA, inst.toStr().c_str());
    return inst;
}

vector<Instruction> Decoder::Decode(const vector<uint16_t>& buffer){
    vector<Instruction> instructions;
    for (uint16_t i=0; i<buffer.size();) {
        uint16_t raw = buffer[i];
        instructions.push_back(Decode(&buffer[i], buffer.size() - i));
        i += instructions.back().ByteSize();
    }
    return instructions;
}
