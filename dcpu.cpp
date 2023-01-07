#include <dcpu.h>
#include <dcpu-codex.h>
#include <cassert>
#include <dcpu-mem.h>
#include <dcpu-hardware.h>

DCPU::DCPU()
    : m_pc {0}
    , m_sp {Memory::LastValidAddress}
    , m_ex {0}
    , m_ia {0}
{
    memset(&m_registers, 0, Registers_Count*2);
}

word_t* DCPU::getAddrPtr(Memory& mem, bool isA, Value v, word_t& extraWord, cycles_t& inOutCycles) {
    const word_t numV = static_cast<word_t>(v);
    if (isA && numV >= 0x20) {
        if (numV == 0x3F) {
            extraWord = 0xFFFF;
        } else {
            extraWord = numV - 0x20;
        }
        // cheating here by reusing the extra word memory location (located in
        // the Instruction instance) to save the inplace A value;
        return &extraWord;
    }
    const signed_word_t signedOffset = static_cast<signed_word_t>(extraWord);
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
    case Value_Register_RefNext_A: ++inOutCycles; return mem+(m_registers[Registers_A] + signedOffset);
    case Value_Register_RefNext_B: ++inOutCycles; return mem+(m_registers[Registers_B] + signedOffset);
    case Value_Register_RefNext_C: ++inOutCycles; return mem+(m_registers[Registers_C] + signedOffset);
    case Value_Register_RefNext_X: ++inOutCycles; return mem+(m_registers[Registers_X] + signedOffset);
    case Value_Register_RefNext_Y: ++inOutCycles; return mem+(m_registers[Registers_Y] + signedOffset);
    case Value_Register_RefNext_Z: ++inOutCycles; return mem+(m_registers[Registers_Z] + signedOffset);
    case Value_Register_RefNext_I: ++inOutCycles; return mem+(m_registers[Registers_I] + signedOffset);
    case Value_Register_RefNext_J: ++inOutCycles; return mem+(m_registers[Registers_J] + signedOffset);
    case Value_PushPop: return isA ? mem + (m_sp++) : mem + (--m_sp);
    case Value_Peek: return mem + m_sp;
    case Value_Pick: ++inOutCycles; return mem + (m_sp + signedOffset);
    case Value_SP: return &m_sp;
    case Value_PC: return &m_pc;
    case Value_EX: return &m_ex;
    case Value_Next: ++inOutCycles; return mem + signedOffset;
    case Value_NextLitteral: ++inOutCycles; return &extraWord;
    default:
        assert(false);
        return nullptr;
    }
}

word_t GetNextCodeAddress(Memory& mem, word_t pc) {
    Instruction currentInstruction = Codex::Decode(mem+pc, mem.LastValidAddress-pc);
    const word_t currentWordCount = currentInstruction.WordCount();
    return pc + currentWordCount;
}

word_t GetNextCodeAddressSkipIF(Memory& mem, word_t pc, word_t& outSkippedInstructionsCount) {
    word_t nextPC = pc;
    bool foundNext = false;
    while (!foundNext && pc <= Memory::LastValidAddress) {
        Instruction currentInstruction = Codex::Decode(mem+nextPC, mem.LastValidAddress-nextPC);
        const word_t currentWordCount = currentInstruction.WordCount();
        nextPC += currentWordCount;
        ++outSkippedInstructionsCount;

        switch (currentInstruction.m_opcode) {
        case OpCode_IFB:
        case OpCode_IFC:
        case OpCode_IFE:
        case OpCode_IFN:
        case OpCode_IFG:
        case OpCode_IFA:
        case OpCode_IFL:
        case OpCode_IFU:
            foundNext = false;
            break;
        default:
            foundNext = true;
            break;
        }
    }
    return nextPC;
}


cycles_t DCPU::eval(Memory& mem, Instruction& inst) {
    //printf("evaluating mem[0x%04X]: %s\n", m_pc, inst.toStr().c_str());
    cycles_t cycles = 0;
    const bool isSpecialOp = inst.m_opcode == OpCode_Special;
    word_t* a_addr = getAddrPtr(mem, true, inst.m_a, inst.m_wordA, cycles);
    word_t* b_addr= isSpecialOp ? nullptr : getAddrPtr(mem, false, inst.m_b, inst.m_wordB, cycles);
    
    switch (inst.m_opcode) {
    case OpCode_Special:{
        OpCode_Special:
        SpecialOpCode specialOp = static_cast<SpecialOpCode>(inst.m_b);
        switch (specialOp) {
        case SpecialOpCode_JSR: {
            cycles += 3;
            const word_t nextPC = GetNextCodeAddress(mem, m_pc);
            *(mem + (--m_sp)) = nextPC;
            m_pc = *a_addr;
            break;
        }
        case SpecialOpCode_INT: {
            cycles += 4;
            if (m_ia != 0) {
                if (m_isInterruptQueueActive) {
                    m_queuedInterrupts.push(*a_addr);
                } else {
                    m_isInterruptQueueActive = true;
                    const word_t nextPC = GetNextCodeAddress(mem, m_pc);
                    *(mem + (--m_sp)) = nextPC;
                    *(mem + (--m_sp)) = m_registers[Registers_A];;
                    m_pc = m_ia;
                    m_registers[Registers_A] = *a_addr;
                }
            }
            break;
        }
        case SpecialOpCode_IAG: {
            cycles += 1;
            *a_addr = m_ia;
            break;
        }
        case SpecialOpCode_IAS: {
            cycles += 1;
            m_ia = *a_addr;
            break;
        }
        case SpecialOpCode_RFI: {
            SpecialOpCode_RFI:
            cycles += 3;
            m_isInterruptQueueActive = false;
            m_registers[Registers_A] = *(mem + (m_sp++));
            m_pc = *(mem + (m_sp++));
            break;
        }
        case SpecialOpCode_IAQ: {
            cycles += 2;
            m_isInterruptQueueActive = *a_addr != 0;
            break;
        }
        case SpecialOpCode_HWN: {
            cycles += 2;
            m_registers[Registers_A] = static_cast<word_t>(m_devices.size());
            break;
        }
        case SpecialOpCode_HWQ: {
            cycles += 4;
            word_t deviceIndex = *a_addr;
            dcpu_assert_fmt(deviceIndex < m_devices.size(), "device index %d larger then number of devices (%d)",
                            deviceIndex, m_devices.size());
            dcpu_assert_fmt(m_devices[deviceIndex] != nullptr, "device index %d was nullptr", deviceIndex);

            const long_t id = m_devices[deviceIndex]->getId();
            const word_t version = m_devices[deviceIndex]->getVersion();
            const long_t manif = m_devices[deviceIndex]->getManifacturer();
            m_registers[Registers_A] = static_cast<word_t>(0xFFFF & id);
            m_registers[Registers_B] = static_cast<word_t>(0xFFFF & (id >> 16));
            m_registers[Registers_C] = version;
            m_registers[Registers_X] = static_cast<word_t>(0xFFFF & manif);
            m_registers[Registers_Y] = static_cast<word_t>(0xFFFF & (manif >> 16));
            break;
        }
        case SpecialOpCode_HWI: {
            const word_t deviceIndex = *a_addr;
            dcpu_assert_fmt(deviceIndex < m_devices.size(), "device index %d larger then number of devices (%d)",
                            deviceIndex, m_devices.size());
            dcpu_assert_fmt(m_devices[deviceIndex] != nullptr, "device index %d was nullptr", deviceIndex);

            const cycles_t intCycles = m_devices[deviceIndex]->interrupt(*this, mem);
            cycles += 4 + intCycles;
            break;
        }
        }
        break;
    }
    case OpCode_SET:{
        cycles += 1;
        *b_addr = *a_addr;
        break;
    }
    case OpCode_ADD:{
        cycles += 2;
        word_t res =  *b_addr + *a_addr;
        *b_addr = res;
        m_ex = (res < *a_addr || res < *b_addr) ? 1 : 0;
        break;
    }
    case OpCode_SUB:{
        cycles += 2;
        word_t b = *b_addr;
        word_t res = b - *a_addr;
        *b_addr = res;
        m_ex = res > b ? 0xFFFF : 0;
        break;
    }
    case OpCode_MUL:{
        cycles += 2;
        OpCode_MUL:
        long_t res = *b_addr * *a_addr;
        *b_addr = static_cast<word_t>(0xFFFF & res);
        m_ex = static_cast<word_t>((res>>16) & 0xFFFF);
        break;
    }
    case OpCode_MLI:{
        cycles += 2;
        long_t res = *b_addr * *a_addr;
        *b_addr = static_cast<word_t>(0xFFFF & res);
        m_ex = static_cast<signed_word_t>((res>>16) & 0xFFFF);
        break;
    }
    case OpCode_DIV: {
        cycles += 3;
        OpCode_DIV:
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            word_t b = *b_addr;
            long_t res = b / *a_addr;
            *b_addr = static_cast<word_t>(0xFFFF & res);
            m_ex = static_cast<word_t>(((static_cast<long_t>(b) << 16) / *a_addr) & 0xFFFF);
        }
        break;
    }
    case OpCode_DVI: {
        cycles += 3;
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            long_t res = static_cast<long_t>(*b_addr) / static_cast<long_t>(*a_addr);
            *b_addr = static_cast<word_t>(0xFFFF & res);
            m_ex = static_cast<word_t>(((*b_addr << 16) / *a_addr) & 0xFFFF);
        }
        break;
    }
    case OpCode_MOD: {
        cycles += 3;
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            *b_addr = *b_addr % *a_addr;
        }
        break;
    }
    case OpCode_MDI: {
        cycles += 3;
        if (*a_addr == 0) {
            *b_addr = m_ex = 0;
        } else {
            *b_addr = static_cast<word_t>(static_cast<signed_word_t>(*b_addr) % static_cast<signed_word_t>(*a_addr));
        }
        break;
    }
    case OpCode_AND: {
        cycles += 1;
        *b_addr = *b_addr & *a_addr;
        break;
    }
    case OpCode_BOR: {
        cycles += 1;
        OpCode_BOR:
        *b_addr = *b_addr | *a_addr;
        break;
    }
    case OpCode_XOR: {
        cycles += 1;
        *b_addr = *b_addr ^ *a_addr;
        break;
    }
    case OpCode_SHR: {
        cycles += 1;
        const word_t b = *b_addr;
        *b_addr = b >> *a_addr;
        m_ex = ((static_cast<long_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_ASR: {
        cycles += 1;
        OpCode_ASR:
        const word_t b = *b_addr;
        *b_addr = static_cast<word_t>(static_cast<signed_word_t>(b) >> *a_addr);
        m_ex = ((static_cast<long_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_SHL:{
        cycles += 1;
        const word_t b = *b_addr;
        *b_addr = b << *a_addr;
        m_ex = ((static_cast<long_t>(b)<<16) >> *a_addr) & 0xFFFF;
        break;
    }
    case OpCode_IFB: {
        OpCode_IFB:
        if ((*b_addr & *a_addr) == 0) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFC: {
        if ((*b_addr & *a_addr) != 0) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFE: {
        if (*b_addr != *a_addr) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFN: {
        test:
        if (*b_addr == *a_addr) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFG: {
        if (*b_addr <= *a_addr) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFA: {
        if (static_cast<signed_word_t>(*b_addr) <= static_cast<signed_word_t>(*a_addr)) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFL: {
        if (*b_addr >= *a_addr) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_IFU: {
        if (static_cast<signed_word_t>(*b_addr) >= static_cast<signed_word_t>(*a_addr)) {
            word_t skippedCount = 0;
            word_t nextPC = GetNextCodeAddress(mem, m_pc);
            nextPC = GetNextCodeAddressSkipIF(mem, nextPC, skippedCount);
            m_pc = nextPC;
            cycles += 2 + skippedCount;
        } else {
            cycles += 2;
        }
        break;
    }
    case OpCode_ADX: {
        cycles += 3;
        OpCode_ADX:
        long_t res = *b_addr + *a_addr + m_ex;
        *b_addr = res & 0xFFFF;
        m_ex = (res >> 16) & 0xFFFF;
        break;
    }
    case OpCode_SBX: {
        cycles += 3;
        word_t b = *b_addr;
        word_t res = b - *a_addr + m_ex;
        *b_addr = res;
        m_ex = res > b ? 0xFFFF : 0;
        break;
    }
    case OpCode_STI: {
        cycles += 2;
        *b_addr = *a_addr;
        ++m_registers[Registers_I];
        ++m_registers[Registers_J];
        break;
    }
    case OpCode_STD: {
        cycles += 2;
        *b_addr = *a_addr;
        --m_registers[Registers_I];
        --m_registers[Registers_J];
        break;
    }
    default:
        assert(false);
    }
    static_assert(OpCode_Count == 0x20, "Please update this when changing opcodes");
    dcpu_assert_fmt(cycles != 0, "Cycle count was not set for instruction %s", inst.toStr().c_str());

    return cycles;
}

void DCPU::step(Memory& mem) {
    word_t* codebytePtr = mem+m_pc;
    Instruction nextInstruction = Codex::Decode(codebytePtr, mem.LastValidAddress-m_pc);
    const word_t originalPC = m_pc;
    const cycles_t cycles = eval(mem, nextInstruction);
    if (m_pc == originalPC)
        m_pc += nextInstruction.WordCount(); // only increment if it wasn't changed
    m_cycles += cycles;

    for (Hardware* device : m_devices) {
        m_cycles += device->update(*this, mem);
    }

    if (!m_isInterruptQueueActive && !m_queuedInterrupts.empty()) {
        const word_t intMsg = m_queuedInterrupts.front();
        m_queuedInterrupts.pop();
        m_isInterruptQueueActive = true;
        *(mem + (--m_sp)) = m_pc;
        *(mem + (--m_sp)) = m_registers[Registers_A];;
        m_pc = m_ia;
        m_registers[Registers_A] = intMsg;
    }
}

cycles_t DCPU::run(Memory& mem, const vector<byte_t>& codebytes) {
    const word_t lastProgramAddr = mem.LoadProgram(Codex::PackBytes(codebytes));
    cycles_t stepCount = 0;
    while(m_pc < lastProgramAddr) {
        step(mem);
    }
    dcpu_assert_fmt(m_queuedInterrupts.empty(), "Did not process all interrupts, queue: %d, queue active? %d",
                    m_queuedInterrupts.size(), m_isInterruptQueueActive);

    return m_cycles;
}

void DCPU::interrupt(word_t message){
    if (m_ia != 0) {
        m_queuedInterrupts.push(message);
    }
}

void DCPU::printRegisters() const {
    printf("pc: %04X\n", m_pc);
    printf("sp: %04X\n", m_sp);
    printf("ex: %04X\n", m_ex);
    printf("ia: %04X\n", m_ia);
    for (int i=0; i<Registers_Count; ++i) {
        printf("%s: %04X (%d)\n", ValueToStr(static_cast<Value>(i), false, 0).c_str(), m_registers[i], m_registers[i]);
    }
}
