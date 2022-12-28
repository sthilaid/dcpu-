#include <dcpu.h>
#include <decoder.h>
#include <cassert>

DCPU::DCPU()
    : m_pc {0}
    , m_sp {Memory::LastValidAddress}
    , m_ex {0}
    , m_ia {0}
{
    memset(&m_registers, 0, Registers_Count*2);
}

uint16_t* DCPU::GetAddrPtr(Memory& mem, bool isA, Value v, uint16_t& extraWord) {
    switch (v) {
    case Value_Register_A: return &m_registers[Registers_A];
    case Value_Register_B: return &m_registers[Registers_B];
    case Value_Register_C: return &m_registers[Registers_C];
    case Value_Register_X: return &m_registers[Registers_X];
    case Value_Register_Y: return &m_registers[Registers_Y];
    case Value_Register_Z: return &m_registers[Registers_Z];
    case Value_Register_I: return &m_registers[Registers_I];
    case Value_Register_J: return &m_registers[Registers_J];
    case Value_Register_Ref_A: return mem+m_registers[Registers_A];
    case Value_Register_Ref_B: return mem+m_registers[Registers_B];
    case Value_Register_Ref_C: return mem+m_registers[Registers_C];
    case Value_Register_Ref_X: return mem+m_registers[Registers_X];
    case Value_Register_Ref_Y: return mem+m_registers[Registers_Y];
    case Value_Register_Ref_Z: return mem+m_registers[Registers_Z];
    case Value_Register_Ref_I: return mem+m_registers[Registers_I];
    case Value_Register_Ref_J: return mem+m_registers[Registers_J];
    case Value_Register_RefNext_A: return mem+(m_registers[Registers_A] + extraWord);
    case Value_Register_RefNext_B: return mem+(m_registers[Registers_B] + extraWord);
    case Value_Register_RefNext_C: return mem+(m_registers[Registers_C] + extraWord);
    case Value_Register_RefNext_X: return mem+(m_registers[Registers_X] + extraWord);
    case Value_Register_RefNext_Y: return mem+(m_registers[Registers_Y] + extraWord);
    case Value_Register_RefNext_Z: return mem+(m_registers[Registers_Z] + extraWord);
    case Value_Register_RefNext_I: return mem+(m_registers[Registers_I] + extraWord);
    case Value_Register_RefNext_J: return mem+(m_registers[Registers_J] + extraWord);
    case Value_PushPop: return isA ? mem + (m_sp++) : mem + (--m_sp);
    case Value_Peek: return mem + m_sp;
    case Value_Pick: return mem + (m_sp + extraWord);
    case Value_SP: return &m_sp;
    case Value_PC: return &m_pc;
    case Value_EX: return &m_ex;
    case Value_Next: return mem + extraWord;
    case Value_NextLitteral: return &extraWord;
    default:
        assert(false);
        return nullptr;
    }
}

uint8_t DCPU::Eval(Memory& mem, Instruction& inst) {
    //printf("evaluating mem[0x%04X]: %s\n", m_pc, inst.toStr().c_str());
    uint16_t* a_addr = GetAddrPtr(mem, true, inst.m_a, inst.m_wordA);
    uint16_t* b_addr= GetAddrPtr(mem, false, inst.m_b, inst.m_wordB);
    
    switch (inst.m_opcode) {
    case OpCode_Special:{
        break;
    }
    case OpCode_SET:{
        *b_addr = *a_addr;
        break;
    }
    case OpCode_ADD:{
        uint16_t res =  *b_addr + *a_addr;
        *b_addr = res;
        m_ex = (res < *a_addr || res < *b_addr) ? 1 : 0;
        break;
    }
    case OpCode_SUB:{
        uint16_t b = *b_addr;
        uint16_t res = b - *a_addr;
        *b_addr = res;
        m_ex = res > b ? 0xFFFF : 0;
        break;
    }
    case OpCode_MUL:{
        OpCode_MUL:
        uint32_t res = *b_addr * *a_addr;
        *b_addr = static_cast<uint16_t>(0xFFFF & res);
        m_ex = static_cast<uint16_t>((res>>16) & 0xFFFF);
        break;
    }
    case OpCode_MLI:{
        int32_t res = *b_addr * *a_addr;
        *b_addr = static_cast<uint16_t>(0xFFFF & res);
        m_ex = static_cast<int16_t>((res>>16) & 0xFFFF);
        break;
    }
    case OpCode_DIV: {
        OpCode_DIV:
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            uint16_t b = *b_addr;
            uint32_t res = b / *a_addr;
            *b_addr = static_cast<uint16_t>(0xFFFF & res);
            m_ex = static_cast<uint16_t>(((static_cast<uint32_t>(b) << 16) / *a_addr) & 0xFFFF);
        }
        break;
    }
    case OpCode_DVI: {
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            int32_t res = static_cast<int32_t>(*b_addr) / static_cast<int32_t>(*a_addr);
            *b_addr = static_cast<uint16_t>(0xFFFF & res);
            m_ex = static_cast<uint16_t>(((*b_addr << 16) / *a_addr) & 0xFFFF);
        }
        break;
    }
    case OpCode_MOD: {
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            *b_addr = *b_addr % *a_addr;
        }
        break;
    }
    case OpCode_MDI: {
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            *b_addr = static_cast<uint16_t>(static_cast<int16_t>(*b_addr) % static_cast<int16_t>(*a_addr));
        }
        break;
    }
    case OpCode_AND: {
        *b_addr = *b_addr & *a_addr;
        break;
    }
    case OpCode_BOR: {
        OpCode_BOR:
        *b_addr = *b_addr | *a_addr;
        break;
    }
    case OpCode_XOR: {
        *b_addr = *b_addr ^ *a_addr;
        break;
    }
    case OpCode_SHR: {
        const uint16_t b = *b_addr;
        *b_addr = b >> *a_addr;
        m_ex = ((static_cast<uint32_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_ASR: {
        OpCode_ASR:
        const uint16_t b = *b_addr;
        *b_addr = static_cast<uint16_t>(static_cast<int16_t>(b) >> *a_addr);
        m_ex = ((static_cast<uint32_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_SHL:{
        const uint16_t b = *b_addr;
        *b_addr = b << *a_addr;
        m_ex = ((static_cast<uint32_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_IFB: {
        if ((*b_addr & *a_addr) == 0) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFC: {
        if ((*b_addr & *a_addr) != 0) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFE: {
        if (*b_addr != *a_addr) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFN: {
        test:
        if (*b_addr == *a_addr) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFG: {
        if (*b_addr <= *a_addr) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFA: {
        if (static_cast<int16_t>(*b_addr) <= static_cast<int16_t>(*a_addr)) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFL: {
        if (*b_addr >= *a_addr) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_IFU: {
        if (static_cast<int16_t>(*b_addr) >= static_cast<int16_t>(*a_addr)) {
            Instruction currentInstruction = Decoder::Decode(mem+m_pc, mem.LastValidAddress-m_pc);
            const uint8_t currentWordCount = currentInstruction.WordCount();
            Instruction nextInstruction = Decoder::Decode(mem+m_pc+currentWordCount, mem.LastValidAddress-m_pc-currentWordCount);
            m_pc += currentWordCount + nextInstruction.WordCount();
        }
        break;
    }
    case OpCode_ADX: {
        OpCode_ADX:
        uint32_t res = *b_addr + *a_addr + m_ex;
        *b_addr = res & 0xFFFF;
        m_ex = (res >> 16) & 0xFFFF;
        break;
    }
    case OpCode_SBX: {
        uint16_t b = *b_addr;
        uint16_t res = b - *a_addr + m_ex;
        *b_addr = res;
        m_ex = res > b ? 0xFFFF : 0;
        break;
    }
    case OpCode_STI: {
        *b_addr = *a_addr;
        ++m_registers[Registers_I];
        ++m_registers[Registers_J];
        break;
    }
    case OpCode_STD: {
        *b_addr = *a_addr;
        --m_registers[Registers_I];
        --m_registers[Registers_J];
        break;
    }
    default:
        assert(false);
    }
    static_assert(OpCode_Count == 0x20, "Please update this when changing opcodes");
    return 1;
}

uint32_t DCPU::Step(Memory& mem) {
    uint16_t* codebytePtr = mem+m_pc;
    Instruction nextInstruction = Decoder::Decode(codebytePtr, mem.LastValidAddress-m_pc);
    const uint8_t instWordCount = nextInstruction.WordCount();
    const uint16_t originalPC = m_pc;
    const uint8_t framecount = Eval(mem, nextInstruction);
    if (m_pc == originalPC)
        m_pc += instWordCount; // only increment if it wasn't changed
    return framecount;
}

uint32_t DCPU::Run(Memory& mem, const vector<uint8_t>& codebytes) {
    const uint16_t lastProgramAddr = mem.LoadProgram(Decoder::PackBytes(codebytes));
    uint32_t stepCount = 0;
    while(m_pc < lastProgramAddr) {
        Step(mem);
        ++stepCount;
    }
    return stepCount; // fixme, use proper cpu cycles
}

void DCPU::PrintRegisters() const {
    printf("pc: %04X\n", m_pc);
    printf("sp: %04X\n", m_sp);
    printf("ex: %04X\n", m_ex);
    printf("ia: %04X\n", m_ia);
    for (int i=0; i<Registers_Count; ++i) {
        printf("%s: %04X (%d)\n", ValueToStr(static_cast<Value>(i), false, 0).c_str(), m_registers[i], m_registers[i]);
    }
}
