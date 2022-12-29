#pragma once

#include <vector>
#include <queue>
#include <types.h>
#include <cstdint>
using std::vector;
using std::queue;

class Instruction;
class Hardware;
class Memory;

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
    uint32_t Run(Memory& mem, const vector<uint8_t>& codebytes);
    void Step(Memory& mem);
    void AddDevice(Hardware* device);

    void PrintRegisters() const;

    uint32_t GetCycles() const { return m_cycles; }
    uint16_t GetPC() const { return m_pc; }
    uint16_t GetSP() const { return m_sp; }
    uint16_t GetEX() const { return m_ex; }
    uint16_t GetIA() const { return m_ia; }
    uint16_t GetRegister(Registers r) const { return m_registers[r]; }

private:
    uint16_t* GetAddrPtr(Memory& mem, bool isA, Value v, uint16_t& extraWord, uint8_t& inOutCycles);
    uint8_t Eval(Memory& mem, Instruction& nextInstruction);

    uint32_t m_cycles = 0;
    uint16_t m_pc = 0;
    uint16_t m_sp = 0;
    uint16_t m_ex = 0;
    uint16_t m_ia = 0;
    uint16_t m_registers[Registers_Count];
    vector<Hardware*> m_devices;
    queue<uint16_t> m_queuedInterrupts;
    bool m_isInterruptQueueActive = false;
};

