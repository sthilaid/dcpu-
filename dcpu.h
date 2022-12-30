#pragma once

#include <vector>
#include <queue>
#include <dcpu-types.h>
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
    uint32_t run(Memory& mem, const vector<uint8_t>& codebytes);
    void step(Memory& mem);
    void interrupt(uint16_t message);

    template<typename HardwareType> void addDevice();

    void printRegisters() const;

    uint32_t getCycles() const { return m_cycles; }
    uint16_t getPC() const { return m_pc; }
    uint16_t getSP() const { return m_sp; }
    uint16_t getEX() const { return m_ex; }
    uint16_t getIA() const { return m_ia; }
    uint16_t getRegister(Registers r) const { return m_registers[r]; }

    void setPC(uint16_t v) { m_pc = v; }
    void setSP(uint16_t v) { m_sp = v; }
    void setRegister(Registers r, uint16_t v) { m_registers[r] = v; }

private:
    uint16_t* getAddrPtr(Memory& mem, bool isA, Value v, uint16_t& extraWord, uint8_t& inOutCycles);
    uint8_t eval(Memory& mem, Instruction& nextInstruction);

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

template<typename HardwareType>
void DCPU::addDevice(){
    HardwareType* device = new HardwareType{};
    device->init(m_devices.size());
    m_devices.push_back(device);
}
