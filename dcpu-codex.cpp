#include <cassert>
#include <dcpu-codex.h>

#define NDEBUG

void pushTo8BitLittleEndian(word_t val, vector<byte_t>& bytes){
    byte_t littleEnd = 0xFF & val;
    byte_t bigEnd = val >> 8;
    bytes.push_back(littleEnd);
    bytes.push_back(bigEnd);
}

word_t from8BitLittleEndian(const vector<byte_t>& buffer, word_t& i) {
    assert(i < buffer.size()-1);
    
    byte_t littleEnd = buffer[i];
    byte_t bigEnd = buffer[i+1];
    i += 2;
    // printf("little:: %02X big: %02X, raw: %04X\n", littleEnd, bigEnd, (bigEnd << 8) | littleEnd);
    return (bigEnd << 8) | littleEnd;
}

vector<word_t> Codex::Encode(const vector<Instruction>& instructions){
    vector<word_t> codeBuffer;
    for (const Instruction& inst : instructions){
        const word_t opcode = inst.m_opcode;
        const word_t a = inst.m_a;
        const word_t b = inst.m_b;
        const word_t binaryInstruction = (a << 0xA) | (b << 0x5) | opcode;
        codeBuffer.push_back(binaryInstruction);

        if (isMultibyteValue(inst.m_a)) {
            codeBuffer.push_back(inst.m_wordA);
        }

        if (opcode != OpCode_Special && isMultibyteValue(inst.m_b)) {
            codeBuffer.push_back(inst.m_wordB);
        }
    }
    return codeBuffer;
}

vector<byte_t> Codex::UnpackBytes(const vector<word_t>& buffer){
    vector<byte_t> unpackedBuffer;
    for (word_t i=0; i<buffer.size(); ++i) {
        pushTo8BitLittleEndian(buffer[i], unpackedBuffer);
    }
    return unpackedBuffer;
}

vector<word_t> Codex::PackBytes(const vector<byte_t>& buffer){
    vector<word_t> packedBuffer;
    for (word_t i=0; i<buffer.size()-1;) {
        packedBuffer.push_back(from8BitLittleEndian(buffer, i));
    }
    return packedBuffer;
}

Instruction Codex::Decode(const word_t* codebytePtr, word_t maxlen){
    assert(codebytePtr != nullptr);
    
    byte_t instructionSize = 1;
    Instruction inst;
    word_t codebyte = *codebytePtr;
    inst.m_opcode = static_cast<OpCode>(codebyte & 0x1F);
    inst.m_a = static_cast<Value>(codebyte >> 0xA);
    inst.m_b = static_cast<Value>((codebyte >> 0x5) & 0x1F);

    if (isMultibyteValue(inst.m_a)) {
        inst.m_wordA = codebytePtr[instructionSize];
        ++instructionSize;
        assert(instructionSize <= maxlen);
    }
    if (isMultibyteValue(inst.m_b)) {
        inst.m_wordB = codebytePtr[instructionSize];
        ++instructionSize;
        assert(instructionSize <= maxlen);
    }
    // printf("-decoded- op: %02X, b: %02X, a: %02X, wordB: %04X, wordA: %04X, inst: %s\n",
    //        inst.m_opcode, inst.m_b, inst.m_a, inst.m_wordB, inst.m_wordA, inst.toStr().c_str());
    return inst;
}

vector<Instruction> Codex::Decode(const vector<word_t>& buffer){
    vector<Instruction> instructions;
    for (word_t i=0; i<buffer.size();) {
        word_t raw = buffer[i];
        instructions.push_back(Decode(&buffer[i], buffer.size() - i));
        i += instructions.back().WordCount();
    }
    return instructions;
}
