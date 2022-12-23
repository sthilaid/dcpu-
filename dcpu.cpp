#include <dcpu.h>
#include <decoder.h>

DCPU::DCPU()
    : m_pc {0}
    , m_sp {Memory::LastValidAddress}
    , m_ex {0}
    , m_ia {0}
{
    memset(&m_registers, 0, RegistersCount*2);
}

uint8_t DCPU::Eval(const Instruction& nextInstruction) {
    return 1;
}

uint32_t DCPU::Step(Memory& mem) {
    uint16_t* codebytePtr = mem+m_pc;
    Instruction nextInstruction = Decoder::Decode(codebytePtr, mem.LastValidAddress-m_pc);
    return Eval(nextInstruction);
}
