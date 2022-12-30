#include <dcpu-hardware-clock.h>
#include <dcpu-assert.h>
#include <dcpu.h>
#include <cstdio>

Clock::Clock()
    : m_period{0}
    , m_tickCount{0}
    , m_interruptsEnabled{false}
{
    m_id = 0x12d0b402;             // https://github.com/lucaspiller/dcpu-specifications/blob/master/clock.txt
    m_version = 1;
    m_manifacturer = 0x0FE124C0;     // FEIZAC
}

uint8_t Clock::update(DCPU& cpu, Memory& mem) {
    if (m_period == 0)
        return 0;

    using secduration = std::chrono::duration<float>;
    const time now = std::chrono::system_clock::now();
    const secduration duration = (now - m_startTime);
    const float dt = duration.count();
    if (dt > m_period / 60.0f) {
        ++m_tickCount;
        m_startTime = std::chrono::system_clock::now();

        if (m_interruptsEnabled) {
            cpu.interrupt(m_deviceId);
        }
    }

    return 1;
}

uint8_t Clock::interrupt(DCPU& cpu, Memory& mem) {
    const uint16_t a = cpu.getRegister(Registers_A);
    const uint16_t b = cpu.getRegister(Registers_B);
    switch (a) {
    case 0: {
        m_period = b;
        if (m_period != 0) {
            m_startTime = std::chrono::system_clock::now();
        } 
        break;
    }
    case 1: {
        cpu.setRegister(Registers_C, m_tickCount);
        break;
    }
    case 2: {
        m_interruptsEnabled = b != 0;
        break;
    }
    }
    return 1;
}
