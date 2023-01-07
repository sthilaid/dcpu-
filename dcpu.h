#pragma once

#include <dcpu-assert.h>
#include <vector>
#include <queue>
#include <dcpu-types.h>
using std::vector;
using std::queue;

class Instruction;
class Hardware;
class Memory;

enum Registers : word_t {
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
    cycles_t run(Memory& mem, const vector<byte_t>& codebytes);
    void step(Memory& mem);
    void interrupt(word_t message);

    template<typename HardwareType> void addDevice();

    void printRegisters() const;

    cycles_t getCycles() const { return m_cycles; }
    word_t getPC() const { return m_pc; }
    word_t getSP() const { return m_sp; }
    word_t getEX() const { return m_ex; }
    word_t getIA() const { return m_ia; }
    word_t getRegister(Registers r) const { return m_registers[r]; }

    void setPC(word_t v) { m_pc = v; }
    void setSP(word_t v) { m_sp = v; }
    void setRegister(Registers r, word_t v) { m_registers[r] = v; }

private:
    word_t* getAddrPtr(Memory& mem, bool isA, Value v, word_t& extraWord, cycles_t& inOutCycles);
    cycles_t eval(Memory& mem, Instruction& nextInstruction);

    cycles_t m_cycles = 0;
    word_t m_pc = 0;
    word_t m_sp = 0;
    word_t m_ex = 0;
    word_t m_ia = 0;
    word_t m_registers[Registers_Count];
    vector<Hardware*> m_devices;
    queue<word_t> m_queuedInterrupts;
    bool m_isInterruptQueueActive = false;
};

template<typename HardwareType>
void DCPU::addDevice(){
    dcpu_assert_fmt(m_devices.size() < 0x10000, "Trying to add to many devices: %d", m_devices.size());
    
    HardwareType* device = new HardwareType{};
    device->init(m_devices.size());
    m_devices.push_back(device);
}
