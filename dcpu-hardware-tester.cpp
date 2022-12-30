#include <dcpu-hardware-tester.h>
#include <random>
#include <dcpu.h>
#include <dcpu-mem.h>

TesterDevice::TesterDevice() {
    
}

uint8_t TesterDevice::update(DCPU& cpu, Memory& mem) {
    return 0;
}

uint8_t TesterDevice::interrupt(DCPU& cpu, Memory& mem) {
    const uint16_t a = cpu.getRegister(Registers_A);
    if (m_lastkey != 0) {
        const uint16_t res = a == m_lastkey ? 123 : 0xFFFF;
        cpu.setSP(cpu.getSP() - 1);
        mem[cpu.getSP()] = res;
        m_lastkey = 0;
    } else {
        switch(a) {
        case 0: cpu.setRegister(Registers_X, 10); break;
        case 1: {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, 255);
            m_lastkey = static_cast<uint16_t>(distrib(gen));
            cpu.setRegister(Registers_X, m_lastkey);
            cpu.interrupt(m_id);
            break;
        }
        }
    }
    return 0;
}
