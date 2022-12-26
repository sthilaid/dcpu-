#pragma once

#include <types.h>
#include <cstdint>
#include <mem.h>

class Instruction;

enum Registers : uint16_t {
    Registers_A,
    Registers_B,
    Registers_C,
    Registers_X,
    Registers_Y,
    Registers_Z,
    Registers_I,
    Registers_J,
    Registers_Count,
};

class DCPU {

public:
    DCPU();
    uint32_t Step(Memory& mem);
    void PrintRegisters() const;

    uint16_t GetPC() const { return m_pc; }

private:
    uint16_t* GetAddrPtr(Memory& mem, bool isA, Value v, uint16_t& extraWord);
    uint8_t Eval(Memory& mem, Instruction& nextInstruction);
    
    uint16_t m_pc;
    uint16_t m_sp;
    uint16_t m_ex;
    uint16_t m_ia;
    uint16_t m_registers[Registers_Count];
};

