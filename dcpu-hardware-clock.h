#pragma once
#include <dcpu-hardware.h>
#include <chrono>

class Clock : public Hardware {
public:
    Clock();
    uint32_t update(DCPU& cpu, Memory& mem) override;
    uint32_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    using time = std::chrono::time_point<std::chrono::system_clock>;
    uint16_t m_period = 0;
    uint16_t m_tickCount = 0;
    time m_startTime;
    bool m_interruptsEnabled = false;
};
