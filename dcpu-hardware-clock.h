#pragma once
#include <dcpu-hardware.h>
#include <chrono>

class Clock : public Hardware {
public:
    Clock();
    cycles_t update(DCPU& cpu, Memory& mem) override;
    cycles_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    using time = std::chrono::time_point<std::chrono::system_clock>;
    word_t m_period = 0;
    word_t m_tickCount = 0;
    time m_startTime;
    bool m_interruptsEnabled = false;
};
